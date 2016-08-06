#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

char decrypt_char(char cipher, char key) {
  char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int position = 0;
  int cipher_index = 0, key_index = 0;
  if(cipher == '\n') return cipher;
  if(cipher == EOF) return '\0';
  for(position; position < 28; position++) {
    if (cipher == dictionary[position]) cipher_index = position;
    if (key == dictionary[position]) key_index = position;
  }

  //deal with C's crappy modulo
  cipher_index = cipher_index  - key_index;
  if(cipher_index < 0) cipher_index = 27 + cipher_index;

  return dictionary[cipher_index];
}

void get_plaintext(char *buffer, int buffer_size, char *cipher_location, char *key_location) {
  int i=0;
  char cipher_ch = '0', key_ch = '0';
  FILE *c = fopen(cipher_location, "r");
  FILE *k = fopen(key_location, "r");
  bzero(buffer, buffer_size);
  while(cipher_ch != EOF) {
    cipher_ch = fgetc(c);
    key_ch = fgetc(k);
    if(key_ch < 0) {
      perror("Key too short");
      fclose(c);
      fclose(k);
      exit(1);
    }
    buffer[i] = decrypt_char(cipher_ch, key_ch);
    i++;
  }

  // replace the last line break with a null char
  buffer[i-2] = '\0';

  // close the files
  fclose(c);
  fclose(k);
}

int main(int argc, char *argv[]) {
  
  char plaintext[512];
  get_plaintext(plaintext, 512, argv[1], argv[2]);
  printf("%s\n", plaintext);
  return 0;
}
