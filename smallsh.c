#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

char *get_input() {
  char *input = NULL;
  ssize_t bufferSize = 0;
  getline(&input, &bufferSize, stdin);
  return input;
}

int count_args(char **args) {
  int i;
  for(i = 0; args[i] != '\0'; i++);
  return i--;
}

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

int change_directory(char **args) {
  if (args[1] == NULL) {
    if(chdir(getenv("HOME")) != 0) perror("smallish");
  } else {
    if (chdir(args[1]) != 0) perror("smallish");
  }
  return 1;
}

int get_arg_index(char keyChar, int numberOfArgs, char **args) {
  int i;
  for(i = 0; i < numberOfArgs; i++) {
    if(args[i][0] == keyChar) return i; // return the index of the key character is found
  }
  return -1;
}

int execute_command(char **args) {
  pid_t pid, wpid;
  int status, fd, fd2;
  int numberOfArgs = count_args(args);
  int outputRedirectIndex = get_arg_index('>', numberOfArgs, args);
  int inputRedirectIndex = get_arg_index('>', numberOfArgs, args);
  int backgroundIndex = get_arg_index('&', numberOfArgs, args);

  if((pid = fork()) < 0) {
    perror("Error forking child process");
  } else if (pid == 0) {
    if(outputRedirectIndex > 0) {
      fd = open(args[outputRedirectIndex + 1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
      if(fd == -1) {
        perror("open");
        exit(1);
      }
      fd2 = dup2(fd, 1);
      if(fd2 == -1) {
        perror("dup2");
        exit(2);
      }

      execlp(args[outputRedirectIndex - 1], args[outputRedirectIndex - 1], NULL);
    }
    if(inputRedirectIndex > 0) {
      fd = open(args[inputRedirectIndex + 1], O_RDONLY, 0644);
      if(fd == -1) {
        perror("open");
        exit(1);
      }
      fd2 = dup2(fd, 1);
      if(fd2 == -1) {
        perror("dup2");
        exit(2);
      }
      execlp(args[inputRedirectIndex - 1], args[inputRedirectIndex - 1], NULL);
    }

    if(backgroundIndex > 0) {
      args[backgroundIndex] = '\0';
    }

    if(execvp(args[0], args) < 0) {
      perror(args[0]);
      exit(EXIT_FAILURE);
    }

    int signal = WIFSIGNALED(status);
    printf("CSignal: %i\n", signal);
  } else {
    if(backgroundIndex > 0) {
      wpid = waitpid(pid &status, WNOHANG);
      printf("background pid is %i\n", pid);
    } else {
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
        int signal = WIFSIGNALED(status);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }

  return 1;
}

int execute(char **args, int prevStatus) {
  int i;

  if (args[0] == NULL) {
   return 1;
  }
  
  if (strcmp(args[0], "cd") == 0) {
    return change_directory(args);
  } else if (strcmp(args[0], "exit") == 0) {
    return 0;
  } else if (args[0][0] == '#') {
    return 1;
  } else if (strcmp(args[0], "status") == 0) {
    printf("HERE IS THE STATUS %i\n", prevStatus);
    return 1;
  }
    
  return execute_command(args);
}

void catchInt(int signo) {
  printf("Caught an interrupt: %d\n", signo);
}

int main() {
  char *userInput;
  char **args;
  int status, i=0;

  struct sigaction act;
  act.sa_handler = catchInt;
  act.sa_flags = 0;
  sigfillset(&(act.sa_mask));

  sigaction(SIGINT, &act, NULL);

  do {
    printf(": ");
    userInput = get_input();
    args = parse_input(userInput);
    status = execute(args, status);
    free(userInput);
    free(args);
  } while(1);

  return 0;
}
