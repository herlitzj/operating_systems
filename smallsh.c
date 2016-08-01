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

// function for printing the status of the most recently
// completed process or background process
void print_status(int status) {
  if(WIFSIGNALED(status)) printf("terminated by signal %i\n", WTERMSIG(status));
  else if(WIFEXITED(status)) printf("exit value %i\n", WEXITSTATUS(status));
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
// used for easily finding where in the tokenized list args to stop/end
int get_arg_index(char keyChar, char **args) {
  int i;
  int numberOfArgs = count_args(args);
  for(i = 0; i < numberOfArgs; i++) {
    if(args[i][0] == keyChar) return i; // return the index of the key character if found
  }
  return -1; // return -1 if not found
}

// a helper function for executing child processes
void execute_child(char *file_args, char *exec_args, int is_output) {
  int flags = is_output ? O_WRONLY|O_CREAT|O_TRUNC : O_RDONLY;
  int fd, fd2;
  fd = open(file_args, flags, 0644);
  if(fd == -1) {
    perror("open");
    exit(1);
  }
  fd2 = dup2(fd, is_output);
  if(fd2 == -1) {
    perror("dup2");
    exit(1);
  }
  if(!is_output) fflush(stdin);
  execlp(exec_args, exec_args, NULL);
}

// execute non built-in commands
int execute_command(char **args) {
  pid_t pid, wpid;
  int status, fd, fd2;
  int outputRedirectIndex = get_arg_index('>', args);
  int inputRedirectIndex = get_arg_index('<', args);
  int backgroundIndex = get_arg_index('&', args);

  // fork the process
  if((pid = fork()) < 0) {
    perror("Error forking child process");
    exit(1);
  } else if (pid == 0) { //handle the child fork
    if(backgroundIndex < 0) signal(SIGINT, SIG_DFL);// catch sigint for foreground children
    if(outputRedirectIndex > 0) { // deal with output redirection
      execute_child(args[outputRedirectIndex + 1], args[outputRedirectIndex - 1], 1);
    }
    if(inputRedirectIndex > 0) { // deal with input redirection
      execute_child(args[inputRedirectIndex + 1], args[inputRedirectIndex - 1], 0);
    }
    if(backgroundIndex > 0) { // replace the & with a null char
      args[backgroundIndex] = '\0';
    }
    pid = execvp(args[0], args); // exec the child
    if(pid < 0) { // deal with any errors
      perror(args[0]);
      exit(1);
    }
  } else { // handle the parent fork
    signal(SIGINT, SIG_IGN);
    if(backgroundIndex > 0) { // handle waiting/message for background child
      wpid = waitpid(pid, &status, WNOHANG);
      printf("background pid is %i\n", pid);
    } else { // deal with foreground processes
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      if(WIFSIGNALED(status)) print_status(status); // print status if process was killed
    }
  }
  return status; // return the status to main
}

// execute the input
// check for any built-in commands and execute
// or call the function for handling non built-in commands
int execute(char **args, int status) {
  int i;

  if (args[0] == NULL) {
   return 0;
  }
 
  // deal with any built-ins
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

  // run non-built in commands    
  return execute_command(args);
}

// main function that loops until exit is input
// checks for background pids and prints status on complete
int main() {
  char *userInput;
  char **args;
  int status, lastChild, pid;

  do { // loop unil killed by exit command
    do { // check for background processes that have completed and print info
      pid = waitpid(-1, &status, WNOHANG);
      if(pid > 0) {
        printf("background pid %i is done: ", pid);
        print_status(status);
      }
    } while (pid > 0);
    printf(": ");
    userInput = get_input(); // get user input
    args = parse_input(userInput); // parse user input
    status = execute(args, status); // execute user input
    fflush(stdout);
  } while(1);

  return 0;
}
