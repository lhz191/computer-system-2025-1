#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "Hello World!\n", 13);
  int j=1;
  while (1) {
    printf("Hello World for the %dth time\n", j ++);
  }
  return 0;
}
