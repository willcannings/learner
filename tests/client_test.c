#include "distributed/protocol/protocol.h"
#include "learner.h"
#include <string.h>

int main(void) {
  learner_initialize();
  add_server("localhost", 1);
  
  char *name = "test_value";
  char *data = "I AM A WALRUS. hear me speak.";
  void *buffer;
  long long blen;
  
  learner_error error = set_key_value(name, strlen(name), data, strlen(data));
  note_with_error("response to set", error);
  
  error = get_key_value(name, strlen(name), &buffer, &blen);
  note_with_error("response to get", error);
  ((char *)buffer)[blen-1] = 0;
  note_with_format("value: %s", buffer);
  
  return 0;
}
