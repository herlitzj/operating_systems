#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

// function for getting the input string from the user
char *get_input() {
  char *input = NULL;
  ssize_t bufferSize = 0;
  getline(&input, &bufferSize, stdin);
  return input;
}

// function to count the number of args for parsing, etc.
int count_args(char **args) {
  int i;
  for(i = 0; args[i] != '\0'; i++);
  return i--;
}

// function for printing the status
void print_status(int status) {
  //printf("STATUS: %i\n", status);
  //printf("WIFEXITED: %i\n", WIFEXITED(status));
  //printf("WEXITSTATUS: %i\n", WEXITSTATUS(status));
  //printf("WIFSIGNALED: %i\n", WIFSIGNALED(status));
  //printf("WTERMSIG: %i\n", WTERMSIG(status));
  if(WIFSIGNALED(status)) printf("terminated by signal %i\n", WTERMSIG(status));
  else if(WIFEXITED(status)) printf("exit value %i\n", WEXITSTATUS(status));
  fflush(stdout);
}

// function for parsing the input to tokenize args, etc.
char **parse_input(char *input) {
  int bufferSize = 100;
  int pos = 0;
  char **parsedInput = malloc(bufferSize * sizeof(char));
  char *token;

  token = strtok(input, " \t\r\n\a");
  while (token != NULL) {
    parsedInput[pos] = token;
    pos++;
    token = strtok(NULL, " \t\r\n\a");
  }

  parsedInput[pos] = NULL;
  return parsedInput;
}

// function for changing directory when cd command is used
int change_directory(char **args) {
  if (args[1] == NULL) {
    if(chdir(getenv("HOME")) != 0) perror("smallish");
  } else {
    if (chdir(args[1]) != 0) perror("smallish");
  }
  return 0;
}

// function for getting the index of any key characters in the args
int get_arg_index(char keyChar, char **args) {
  int i;
  int numberOfArgs = count_args(args);
  for(i = 0; i < numberOfArgs; i++) {
    if(args[i][0] == keyChar) return i; // return the index of the key character if found
  }
  return -1;
}

// a helper function for executing child commands
void execute_child(char *file_args, char *exec_args, int is_output) {
  int flags = is_output ? O_WRONLY|O_CREAT|O_TRUNC : O_RDONLY;
  int fd, fd2;
  fd = open(file_args, flags, 0644);
  if(fd == -1) {
    perror("open");
    fflush(stdout);
    exit(1);
  }
  fd2 = dup2(fd, is_output);
  if(fd2 == -1) {
    perror("dup2");
    exit(1);
  }
  fflush(stdout);
  fflush(stdin);
  execlp(exec_args, exec_args, NULL);
}

// execute non built-in commands
int execute_command(char **args) {
  pid_t pid, wpid;
  int status, fd, fd2;
  int outputRedirectIndex = get_arg_index('>', args);
  int inputRedirectIndex = get_arg_index('<', args);
  int backgroundIndex = get_arg_index('&', args);

  if((pid = fork()) < 0) {
    perror("Error forking child process");
    exit(1);
  } else if (pid == 0) {
    if(backgroundIndex < 0) signal(SIGINT, SIG_DFL);
    if(outputRedirectIndex > 0) {
      execute_child(args[outputRedirectIndex + 1], args[outputRedirectIndex - 1], 1);
    }
    if(inputRedirectIndex > 0) {
      execute_child(args[inputRedirectIndex + 1], args[inputRedirectIndex - 1], 0);
    }

    if(backgroundIndex > 0) {
      args[backgroundIndex] = '\0';
    }

    pid = execvp(args[0], args);
    if(pid < 0) {
      perror(args[0]);
      exit(1);
    }

  } else {
    signal(SIGINT, SIG_IGN);
    if(backgroundIndex > 0) {
      wpid = waitpid(pid, &status, WNOHANG);
      printf("background pid is %i\n", pid);
      fflush(stdout);
    } else {
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      if(WIFSIGNALED(status)) print_status(status);
    }
  }

  return status;
}

// execute the input
// check for any built-in commands and execute
// or call the function for handling non built-in commands
int execute(char **args, int status) {
  int i;

  if (args[0] == NULL) {
   return 0;
  }
  
  if (strcmp(args[0], "cd") == 0) {
    return change_directory(args);
  } else if (strcmp(args[0], "exit") == 0) {
    kill(0, SIGTERM);
  } else if (args[0][0] == '#') {
    return 0;
  } else if(strcmp(args[0], "status") == 0) {
    print_status(status);
    return 0;
  }
    
  return execute_command(args);
}

// main function that loops until exit is input
// checks for background pids and prints status on complete
int main() {
  char *userInput;
  char **args;
  int status, lastChild, pid;

  do {
    do {
      pid = waitpid(-1, &status, WNOHANG);
      if(pid > 0) {
        printf("background pid %i is done: ", pid);
        fflush(stdout);
        print_status(status);
      }
    } while (pid > 0);

    printf(": ");
    fflush(stdout);
    userInput = get_input();
    args = parse_input(userInput);
    status = execute(args, status);
  } while(1);

  return 0;
}
