#include <unistd.h>
#include <stdio.h>

int main(void) {
  printf("%i\n", sysconf(_SC_PAGE_SIZE));
}
