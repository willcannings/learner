#include "distributed/server/server.h"
#include "core/logging.h"
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define LEARNER_KQUEUE

#ifdef LEARNER_EPOLL
  #include <sys/epoll.h>
#endif

#ifdef LEARNER_KQUEUE
  #include <sys/types.h>
  #include <sys/event.h>
  #include <sys/time.h>
#endif

#ifdef LEARNER_POLL
  #include <poll.h>
#endif


void read_thread_cleanup(void *fdset) {
  #ifdef LEARNER_EPOLL
  #endif
  
  #ifdef LEARNER_KQUEUE
  #endif
  
  #ifdef LEARNER_POLL
  // if(fdset) {
  //   for(int socket = 0; socket < FD_SETSIZE; socket++) {
  //     if(FD_ISSET(socket, fdset)) {
  //       close(socket);
  //     }
  //   }
  //   free(fdset);
  // }
  #endif
  
}


void *read_thread(void *param) {
  int error = 0;
  
  #ifdef LEARNER_KQUEUE
    int events = 0, client_socket = 0, bytes = 0;
    int queue = kqueue();
    struct kevent change;
    struct kevent event;
    
    // initialise the kqueue
    if (queue == -1) {
      fatal_with_errno("Unable to create a new kqueue object");
    }
    
    // add the queue socket to the queue
    memset(&change, 0, sizeof(struct kevent));
    EV_SET(&change, read_queue_reader, EVFILT_READ, EV_ADD, 0, 0, NULL);
    error = kevent(queue, &change, 1, NULL, 0, NULL);
    if (error == -1) {
      fatal_with_errno("Unable to add the read queue reader to a kqueue");
    }
    note("Read thread started");
    
    while(1) {
      // for now, we only monitor one event change at a time
      memset(&event, 0, sizeof(struct kevent));
      
      // block in kevent until a socket is available for reading
      events = kevent(queue, NULL, 0, &event, 1, NULL);
      if (events == -1 || event.flags == EV_ERROR) {
        fatal_with_errno("kevent failed in read thread");
      }
      
      // the socket used for the queue is available to read
      if (event.ident == read_queue_reader) {
        if (event.flags == EV_EOF) {
          pthread_exit(NULL);
        }
        
        // ensure we can read a full client socket file descriptor
        if (event.data == sizeof(int)) {
          bytes = read(read_queue_reader, &client_socket, sizeof(int));
          
          if (bytes == 0) {
            pthread_exit(NULL);
          } else if (bytes == -1) {
            fatal_with_errno("Read thread unable to read from read queue");
          } else if (bytes != sizeof(int)) {
            fatal("Unable to read full client socket file descriptor");
          }
          
          // add the client socket to the kqueue
          memset(&change, 0, sizeof(struct kevent));
          EV_SET(&change, client_socket, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
          error = kevent(queue, &change, 1, NULL, 0, NULL);
          if (error == -1) {
            fatal_with_errno("Unable to add a client socket to the read queue");
          }
        }
      } else {
        if (event.flags == EV_EOF) {
          // close the connection to the client, because one-shot
          // events are used, the event is removed automatically
          close(event.ident);
        } else {
          // add the client to the process queue
          bytes = write(process_queue_writer, &event.ident, sizeof(int));
          if (bytes == 0) {
            warn("Process queue has closed. Reader thread shutting down.");
            pthread_exit(NULL);
          } else if (bytes == -1) {
            fatal_with_errno("Unable to write client socket file descriptor to process queue");
          } else if (bytes != sizeof(int)) {
            fatal_with_format("Unable to write complete client socket file descriptor. Wrote: %i", bytes);
          }
        }
      }
    }
  #endif
  
  #ifdef LEARNER_EPOLL
  #endif
  
  #ifdef LEARNER_POLL
  #endif  
}
