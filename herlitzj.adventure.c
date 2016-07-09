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
  int buffer_size = 100;
  char *file_location = malloc(buffer_size);
  char *file_name = "/test.txt";

  snprintf(file_location, buffer_size, "%s%s", directory, file_name);


  FILE *f = fopen(file_location, "w");
  /*if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }*/ 

  /* print some text */
  const char *text = "Write this to the file";
  fprintf(f, "Some text: %s\n", text);

  /* print integers and floats */
  int i = 1;
  float py = 3.1415927;
  fprintf(f, "Integer: %d, float: %f\n", i, py);

  /* printing single chatacters */
  char c = 'A';
  fprintf(f, "A character: %c\n", c);

  fclose(f);
}

int main() {

  char *directory = create_directory();
  printf("DIR: %s\n", directory);
  create_rooms(directory);

  return 0;
}


