#include <stdio.h>

int main() {
  int arg1 = 1;
  int arg2 = 23;
  int mod = -22 % 27;
  //if(mod < 0) mod *= -1;
  printf("%i mod %i = %i\n", arg1, arg2, mod);
  return 0;
}
