#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

char* create_directory() {
  int buffer_size = 25; 
  int pid = getpid(); 
  char *directory = malloc(buffer_size);
  char *prefix = "./herlitzj.rooms.";
  struct stat st = {0};

  snprintf(directory, buffer_size, "%s%d", prefix, pid);

  if (stat(directory, &st) == -1) {
    mkdir(directory, 0700);
  }

  return directory;
}

void create_rooms(char* directory) {
  printf("Creating rooms\n");
  int i;
  const BUFFER_SIZE = 100;
  const NUMBER_OF_ROOMS = 7;
  char *room_names[10] = {
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten"
  };

  for(i=0; i<NUMBER_OF_ROOMS; i++) {
    char *file_location = malloc(BUFFER_SIZE);
    char *file_name = room_names[i];
    snprintf(file_location, BUFFER_SIZE, "%s/%s.txt", directory, file_name);
    FILE *f = fopen(file_location, "w");
    fprintf(f, "ROOM NAME:: %s\n", file_name);
    fclose(f);
  }

}

int main() {

  char *directory = create_directory();
  printf("DIR: %s\n", directory);
  create_rooms(directory);

  return 0;
}


