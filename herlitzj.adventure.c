#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void create_directory() {
  int bufferSize = 25; 
  int pid = getpid(); 
  char *directory = malloc(bufferSize);
  char *prefix = "./herlitzj.rooms.";
  struct stat st = {0};

  snprintf(directory, bufferSize, "%s%d", prefix, pid);

  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }
}

int main() {

  create_directory();
  //create_rooms();

  return 0;
}
