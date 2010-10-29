#include <poll.h>

int main(void) {
  struct pollfd p;
  p.fd = 0;
  p.events = 0;
  p.revents = 0;
  poll(&p, 1, 0);
}
