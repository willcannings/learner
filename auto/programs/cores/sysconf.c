/* http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine */
#include <unistd.h>
#include <stdio.h>

int main(void) {
  printf("%i\n", sysconf(_SC_NPROCESSORS_ONLN));
}
