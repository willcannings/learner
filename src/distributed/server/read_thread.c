#include "distributed/server/server.h"
#include "core/logging.h"
#include <pthread.h>
#include <sys/select.h>

void read_thread_cleanup(void *fdset) {
  if(fdset) {
    for(int socket = 0; socket < FD_SETSIZE; socket++) {
      if(FD_ISSET(socket, fdset)) {
        close(socket);
      }
    }
    free(fdset);
  }
}

void *read_thread(void *param) {
  fd_set *sockets = (fd_set *) malloc(sizeof(fd_set));
  fd_set *watching = (fd_set *) malloc(sizeof(fd_set));
  pthread_cleanup_push(read_thread_cleanup, (void *)sockets);
  FD_ZERO(sockets);
  FD_ZERO(watching);
  
  FD_SET(read_queue_reader, sockets);
  int highest_socket = read_queue_reader;
  FD_COPY(sockets, watching);
  
  while(1) {
    
    
    // get the next client to process
    bytes = read(read_queue_reader, current_client, sizeof(int));
    if(bytes != sizeof(int)) {
      if(bytes == -1) {
        fatal_with_format("Error reading from process queue: %s", strerror(errno));
      } else {
        fatal("Unexpected termination of process queue");
      }
    }
  }
  
  pthread_cleanup_pop(1);
  return NULL;
}
