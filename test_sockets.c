#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  printf("SEC: %d\n", tv.tv_sec); // seconds
  printf("MIC: %d\n", tv.tv_usec); // microseconds
  return 0;
}
