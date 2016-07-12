#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Room struct to hold all room information during intialization
struct Room {
  char name[50];
  char type[15];
  int num_of_connections;
  char connections[10][50];
};

// Basic swap function for randomization of rooms
void swap(char a[][50], int i, int j) {
  char t[50];
  strcpy(t,  a[i]);
  strcpy(a[i], a[j]);
  strcpy(a[j], t);  
}

// Function for shuffling the array of room names for game initialization
void shuffle_rooms(char rooms[][50], int size) {
  int i, tmp;
  srand(time(NULL));
  printf("SHUFFLING");
  for (i = (size-1); i > 0; i--) {
    tmp = rand() % (i + 1);
    swap(rooms, i, tmp);
  }

}

int already_connected(char room_name[], char connections[][50], int size){
  int i;
  for (i = 0; i < size; i++) {
    if (connections[i] == NULL) strcpy(connections[i], "");
    if (strcmp(room_name, connections[i]) == 0) return 0;
  }
  return 1;
}

// function to create the rooms directory
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

void build_connections(struct Room rooms[], int index) {
  printf("BUILDING CONNECTIONS");
  // choose number of connections to make (3 to 6)
  int num_connections_to_make = rand() % 4 + 3;
  while (rooms[index].num_of_connections < num_connections_to_make) {
    // randomly choose a room to connect to
    int connection_index = rand() % 7;
    char tmp_name[50];
    strcpy(tmp_name, rooms[connection_index].name);
    // check if chosen room is current room or if rooms are already connected
    if(connection_index != index && already_connected(tmp_name, rooms[index].connections, 7) == 1) {
      // connect the rooms
      strcpy(rooms[index].connections[rooms[index].num_of_connections], tmp_name);
      rooms[index].num_of_connections++;
      strcpy(rooms[connection_index].connections[rooms[connection_index].num_of_connections],\
             rooms[index].name);
      rooms[connection_index].num_of_connections++;
    }
  }
}

struct Room create_rooms(char* directory) {
  printf("Creating rooms\n");
  // Basic variables
  int i,j;
  const int BUFFER_SIZE = 100;
  const int NUMBER_OF_ROOMS = 7;

  // Room generation variables
  struct Room rooms[10]; // array of structs to hold the room information on init
  char room_names[][50] = {
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

  shuffle_rooms(room_names, 10); // random shuffle of room names

  // initialize an array of empty Room structs
  for (i = 0; i < NUMBER_OF_ROOMS; i++) {
    struct Room tmp;
    strcpy(tmp.name, room_names[i]);
    strcpy(tmp.type, "MID_ROOM");
    tmp.num_of_connections = 0;
    rooms[i] = tmp;
  }

  // Loop through the array of Room structs and build connections
  for (i = 0; i < NUMBER_OF_ROOMS; i++) {
    build_connections(rooms, i);
  }
  printf("CONNECTIONS BUILT\n");

  // choose a start and an end room
  int start_index = rand() % 7;
  int end_index = start_index;

  while (start_index == end_index) {
    end_index = rand() % 7;
  }

  strcpy(rooms[start_index].type, "START_ROOM");
  strcpy(rooms[end_index].type, "END_ROOM"); 

  // save room structs to disc
  for (i = 0; i < NUMBER_OF_ROOMS; i++) {
    char *file_location = malloc(BUFFER_SIZE);
    char room_name[50];
    strcpy(room_name, rooms[i].name);
    snprintf(file_location, BUFFER_SIZE, "%s/%s.txt", directory, room_name);
    FILE *f = fopen(file_location, "w");
    fprintf(f, "ROOM NAME: %s\n", room_name);
    for (j = 0; j < rooms[i].num_of_connections; j++) {
      fprintf(f, "CONNECTION %i: %s\n", (j + 1), rooms[i].connections[j]);
    }
    fprintf(f, "ROOM TYPE: %s\n", rooms[i].type);
    fclose(f);
  }

  printf("SAVED TO DISC\n");
  return rooms[start_index];
}

int main() {
  int i;
  struct Room start_room;
  char *directory = create_directory();
  char current_room_type[20];
  printf("DIR: %s\n", directory);
  start_room = create_rooms(directory);
  strcpy(current_room_type, start_room.type);

  while(strcmp(current_room_type, "END_ROOM") != 0) {
    
  }
  return 0;
}


