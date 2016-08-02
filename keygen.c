#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *args[]) {
  if(args[1] == NULL) { printf("Usage: keygen [length]\n"); return 1; }
  srand(time(NULL));
  int i, random, key_length = atoi(args[1]);
  const int ALPHA_LENGTH = 27;
  const char ALPHA[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ "};
  for(i = 0; i < key_length; i++) {
    random = rand() % 27;
    printf("%c", ALPHA[random]);
  }
  printf("\n");
  return 0;
}
