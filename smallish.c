#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

int execute(char **args) {
  pid_t pid, wpid;
  int status;

  if((pid = fork()) < 0) {
    perror("Error forking child process");
  } else if (pid == 0) {
    if(execvp(args[0], args) < 0) {
      perror("Error executing child process");
      exit(EXIT_FAILURE);
    }
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

void smallish() {
  char *userInput;
  char **args;
  int status, i=0;

  do {
    printf(": ");
    userInput = get_input();
    printf("Input Before: %s\n", userInput);
    args = parse_input(userInput);

    printf("Input After: %s\n", userInput);
    for(i = 0; i<3; i++) printf("ARG %i: %s ", i, args[i]);
    printf("\n");
    printf("EXECUTING");
    execute(args);
    status = 1;

    free(userInput);
    free(args);

  } while(status);

}

int main(int argc, char **argv) {
  smallish();
  return 0;
}
