#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

char *get_input() {
  char *input = NULL;
  ssize_t bufferSize = 0;
  getline(&input, &bufferSize, stdin);
  return input;
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
    perror("smallish: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("smallish");
    }
  }
  return 1;
}

int execute_command(char **args) {
  pid_t pid, wpid;
  int status;

  if((pid = fork()) < 0) {
    perror("Error forking child process");
  } else if (pid == 0) {
    if(execvp(args[0], args) < 0) {
      perror("Error executing child process");
      exit(EXIT_FAILURE);
    }

    int signal = WIFSIGNALED(status);
    printf("CSignal: %i\n", signal);
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      int signal = WIFSIGNALED(status);
      printf("WPID: %i\n", wpid);
      printf("WPID Status: %i\n", status);
      printf("WSignal: %i\n", signal);
    } while(status);//while (!WIFEXITED(status) && !WIFSIGNALED(status));
    
  }

  return 1;
}

int execute(char **args) {
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
  }
    
  return execute_command(args);
}

void catchInt(int signo) {
  printf("Caught an interrupt: %d\n", signo);
}

int main(int argc, char **argv) {
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
    printf("Input Before: %s\n", userInput);
    args = parse_input(userInput);

    printf("Input After: %s\n", userInput);
    for(i = 0; i<3; i++) printf("ARG %i: %s ", i, args[i]);
    printf("\n");
    printf("EXECUTING\n");
    status = execute(args);

    free(userInput);
    free(args);

  } while(status);

  return 0;
}
