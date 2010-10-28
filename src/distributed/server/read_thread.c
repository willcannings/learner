#include "distributed/server/server.h"
#include "core/logging.h"
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define LEARNER_KQUEUE

// ------------------------------------------
// includes
// ------------------------------------------
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


// ------------------------------------------
// event system list additions
// ------------------------------------------
#ifdef LEARNER_KQUEUE
  void add_socket_to_queue(int queue, int socket, uint16_t extra_flags) {
    struct kevent change;
    EV_SET(&change, socket, EVFILT_READ, EV_ADD | extra_flags, 0, 0, NULL);
    if(kevent(queue, &change, 1, NULL, 0, NULL) == -1) {
      fatal_with_errno("Unable to add a socket to the kqueue");
    }
  }
#endif

#ifdef LEARNER_EPOLL
  void add_socket_to_queue(int queue, int socket, uint16_t extra_flags) {
    struct epoll_event change;
    change.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | extra_flags;
    change.data.fd = socket;
    if (epoll_ctl(queue, EPOLL_CTL_ADD, socket, &change) == -1) {
      fatal_with_errno("Unable to add a socket to the epoll queue");
    }
  }
#endif

#ifdef LEARNER_POLL
#endif


// ------------------------------------------
// cleanup
// ------------------------------------------
// FIXME: we need to cleanup our open file descriptors here
void read_thread_cleanup(void *fdset) {
  #ifdef LEARNER_EPOLL
  #endif
  
  #ifdef LEARNER_KQUEUE
  #endif
  
  #ifdef LEARNER_POLL
  #endif  
}


// ------------------------------------------
// helpers
// ------------------------------------------
// add a client to the process queue
void add_client_to_process_queue(int socket) {
  int bytes = write(process_queue_writer, &socket, sizeof(int));
  if (bytes != sizeof(int) && shutting_down) {
    pthread_exit(NULL);
  } else if (bytes == 0) {
    note("Process queue has closed. Reader thread shutting down.");
    pthread_exit(NULL);
  } else if (bytes == -1) {
    fatal_with_errno("Unable to write client socket file descriptor to process queue");
  } else if (bytes != sizeof(int)) {
    fatal_with_format("Unable to write complete client socket file descriptor. Wrote: %i", bytes);
  }
}

// read & return a client from the read queue socket
int read_client_from_queue() {
  int bytes = 0, client_socket = 0;
  bytes = read(read_queue_reader, &client_socket, sizeof(int));
  
  if (bytes != sizeof(int) && shutting_down) {
    pthread_exit(NULL);
  } else if (bytes == 0) {
    note("Read queue has closed. Reader thread shutting down.");
    pthread_exit(NULL);
  } else if (bytes == -1) {
    fatal_with_errno("Read thread unable to read from read queue");
  } else if (bytes != sizeof(int)) {
    fatal("Unable to read full client socket file descriptor");
  }
  
  return client_socket;
}

// remove a client
void close_client_connection(int socket) {
  if (close(socket) == -1) {
    warn_with_errno("Unable to close client connection");
  }
}


// ------------------------------------------
// event loop
// ------------------------------------------
void *read_thread(void *param) {
  #ifdef LEARNER_KQUEUE
    int queue = kqueue();
    struct kevent event;
  #endif
  #ifdef LEARNER_EPOLL
    int queue = epoll_create(EPOLL_SIZE);
    struct epoll_event event;
  #endif
  int events = 0, fd = 0, eof = 0;
    
  if (queue == -1) {
    fatal_with_errno("Unable to create a new kqueue object");
  }
    
  // add the read queue socket to the queue
  add_socket_to_queue(queue, read_queue_reader, 0);
  note("Read thread started");
    
  while(1) {
    // FIXME: for now, we only monitor one event change at a time
    // block until a socket is available for reading
    #ifdef LEARNER_KQUEUE
      events = kevent(queue, NULL, 0, &event, 1, NULL);
    #endif
    #ifdef LEARNER_EPOLL
      events = epoll_wait(queue, &event, 1, -1);
    #endif
    
    // handle errors
    if (events == -1) {
      fatal_with_errno("Error waiting for the read thread queue");
    }
    
    // the socket used for the queue is available to read
    #ifdef LEARNER_KQUEUE
      fd = event.ident;
      eof = (event.flags & EV_EOF) || (event.flags & EV_ERROR);
    #endif
    #ifdef LEARNER_EPOLL
      fd = event.data.fd;
      eof = (event.events & EPOLLRDHUP) || (event.events & EPOLLERR) || (event.events & EPOLLHUP);
    #endif
    
    if (fd == read_queue_reader) {
      if (eof) {
        pthread_exit(NULL);
      } else {
        #ifdef LEARNER_KQUEUE
           if (event.data != sizeof(int)) {
             continue;
            }
        #endif
        
        fd = read_client_from_queue();
        
        #ifdef LEARNER_KQUEUE
          add_socket_to_queue(queue, fd, EV_ONESHOT);
        #endif
        #ifdef LEARNER_EPOLL
          add_socket_to_queue(queue, fd, 0);
        #endif
      }
    } else {
      if (eof) {
        close_client_connection(fd);
      } else {
        add_client_to_process_queue(fd);
        #ifdef LEARNER_EPOLL
          if (epoll_ctl(queue, EPOLL_CTL_DEL, fd, NULL) == -1) {
            fatal_with_errno("Unable to remove client socket from the epoll queue");
          }
        #endif
      }
    }
  }
  
}
