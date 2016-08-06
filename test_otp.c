#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

char encrypt_char(char plain, char key) {
  char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int position = 0;
  int plain_index = 0, key_index = 0;
  if(plain == '\n') return plain;
  if(plain == EOF) return '\0';
  for(position; position < 28; position++) {
    if (plain == dictionary[position]) plain_index = position;
    if (key == dictionary[position]) key_index = position;
  }

  plain_index = (plain_index + key_index) % 27;
  return dictionary[plain_index];
}

void get_ciphertext(char *buffer, int buffer_size, char *plain_location, char *key_location) {
  int i=0;
  char plain_ch = '0', key_ch = '0';
  FILE *p = fopen(plain_location, "r");
  FILE *k = fopen(key_location, "r");
  bzero(buffer, buffer_size);
  while(plain_ch != EOF) {
    plain_ch = fgetc(p);
    key_ch = fgetc(k);
    if(key_ch < 0) {
      perror("Key too short");
      fclose(p);
      fclose(k);
      exit(1);
    }

    buffer[i] = encrypt_char(plain_ch, key_ch);
    i++;
  }

  // replace the last line break with a null char
  buffer[i-2] = '\0';

  // close the files
  fclose(p);
  fclose(k);
}

int main(int argc, char *argv[]) {
  
  char ciphertext[512];
  get_ciphertext(ciphertext, 512, argv[1], argv[2]);
  printf("%s\n", ciphertext);
  return 0;
}
