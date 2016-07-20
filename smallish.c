#include <stdio.h>
#include <string.h>

int main() {
  char action[1024];  

  while(0==0) {
    printf(": ");
    scanf("%s", &action);

    if(strncmp(action, "exit", strlen("exit")) == 0) return 0;
  }

}

