#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Room struct to hold all room information during intialization
struct Room {
  char *name;
  char *type;
  int num_of_connections;
  char *connections[6];
};

// Basic swap function for randomization of rooms
void swap(char *a[], int i, int j) {
  char *t = a[i];
  a[i] = a[j];
  a[j] = t;
}

// Function for shuffling the array of room names for game initialization
void shuffle_rooms(char *rooms[], int size) {
  int i, tmp;
  srand(time(NULL));

  for (i = (size-1); i > 0; i--) {
    tmp = rand() % (i + 1);
    swap(rooms, i, tmp);
  }

}

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

  for (i = 0; i < NUMBER_OF_ROOMS; i++) {
    char *file_location = malloc(BUFFER_SIZE);
    char *file_name = room_names[i];
    snprintf(file_location, BUFFER_SIZE, "%s/%s.txt", directory, file_name);
    FILE *f = fopen(file_location, "w");
    fprintf(f, "ROOM NAME: %s\n", file_name);
    fprintf(f, "CONNECTION:");
    fprintf(f,"ROOM TYPE: ");
    fclose(f);
  }

}

int main() {
  int i;
  //char *directory = create_directory();
  //printf("DIR: %s\n", directory);
  //create_rooms(directory);

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

  shuffle_rooms(room_names, 10);

  for (i = 0; i < 7; i++) {
    printf("Room: %s\n", room_names[i]);
  }

  return 0;
}


