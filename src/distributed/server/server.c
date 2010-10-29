#include "distributed/server/globals.h"
#include "distributed/server/server.h"
#include "core/logging.h"
#include "learner.h"
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <tcutil.h>
#include <tchdb.h>

// tokyo cabinet
#define MMAPED_MEMORY_SIZE    (512 * 1024 * 1024)
#define NUM_CACHED_RECORDS    50000
#define DELETES_BEFORE_DEFRAG 100000

// thread lists and server socket
static int server_socket = 0;
static pthread_t *process_threads;
static pthread_t *read_threads;


void initialize_server() {
  // initialise learner & server configuration
  learner_initialize();
  find_and_read_configuration_file(NULL, &config);
  
  // install the cleanup signal handler
  signal(SIGQUIT, cleanup_server);
  signal(SIGTERM, cleanup_server);
  signal(SIGINT,  cleanup_server);
  
  // open the backing database
  db = tchdbnew();
  if (!tchdbsetxmsiz(db, MMAPED_MEMORY_SIZE)) {
    fatal_with_format("Unable to set mmaped memory size of backing database: %s", tchdberrmsg(tchdbecode(db)));
  }
  if (!tchdbsetcache(db, NUM_CACHED_RECORDS)) {
    fatal_with_format("Unable to set cache size of backing database: %s", tchdberrmsg(tchdbecode(db)));
  }
  if (!tchdbsetdfunit(db, DELETES_BEFORE_DEFRAG)) {
    fatal_with_format("Unable to set defrag options on backing database: %s", tchdberrmsg(tchdbecode(db)));
  }
  if (!tchdbtune(db, -1, -1, -1, HDBTLARGE | HDBTDEFLATE)) {
    fatal_with_format("Unable to set backing database options: %s", tchdberrmsg(tchdbecode(db)));
  }
  if (!tchdbopen(db, "learner.db", HDBOWRITER | HDBOCREAT | HDBOTSYNC)) {
    fatal_with_format("Unable to open the backing database: %s", tchdberrmsg(tchdbecode(db)));
  }
  
  // create the server socket
  int error = 0;
  create_server_socket(config.port, config.accept_backlog, server_socket, error);
  if (error) {
    fatal_with_errno("Unable to open a server socket");
  } else {
    debug_with_format("Server listening on port %i", config.port);
  }
  
  // create the unnamed sockets used as queues between threads
  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
    fatal_with_errno("Unable to open read queue sockets");
  }
  read_queue_reader = sockets[0];
  read_queue_writer = sockets[1];
  
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
    fatal_with_errno("Unable to open process queue sockets");
  }
  process_queue_reader = sockets[0];
  process_queue_writer = sockets[1];
  
  // reader and processing threads
  read_threads = (pthread_t *) malloc(sizeof(pthread_t *) * config.read_threads);
  process_threads = (pthread_t *) malloc(sizeof(pthread_t *) * config.process_threads);
  for (int i = 0; i < config.read_threads; i++) {
    pthread_create(&read_threads[i], NULL, read_thread, NULL);
  }
  for (int i = 0; i < config.process_threads; i++) {
    pthread_create(&process_threads[i], NULL, process_thread, NULL);
  }
}


void cleanup_server() {
  note("Preparing to quit... cleaning up");
  shutting_down = 1;
  int error = 0;
  
  // tokyo db
  if (!tchdbclose(db)) {
    fatal_with_format("Error closing the database: %s", tchdberrmsg(tchdbecode(db)));
  }
  tchdbdel(db);
  
  // sockets
  if (server_socket) close(server_socket);
  if (read_queue_reader) close(read_queue_reader);
  if (read_queue_writer) close(read_queue_writer);
  if (process_queue_reader) close(process_queue_reader);
  if (process_queue_writer) close(process_queue_writer);
  
  // threads; these will close their own sockets with their cleanup handlers
  for (int i = 0; i < config.read_threads; i++) {
    if (error = pthread_cancel(read_threads[i])) {
      fatal_with_format("Unable to close read thread during cleanup: %s", strerror(error));
    }
  }
  
  for (int i = 0; i < config.process_threads; i++) {
    if (error = pthread_cancel(process_threads[i])) {
      fatal_with_format("Unable to close process thread during cleanup: %s", strerror(error));
    }
  }

  // finished
  note("Cleanup complete, exiting. Goodbye.");
  exit(0);
}




int main(void) {
  int error = 0, client_socket = 0, bytes = 0;
  initialize_server();
  note("Server started");
  
  // the main thread accepts connections on the server socket and sends
  // the client socket fildes along the read_queue socket pair, for a
  // reader thread to pick up and watch for reads. the reader thread
  // doesn't do any reading - it uses kqueue/epoll to watch for sockets
  // that can be processed, and passes these along to process threads.
  // after a first connection, a new client is passed immediately along
  // to a process thread. after processing the client returns to the 
  // reader pool until they disconnect or request another operation.
  
  while(1) {
    // accept a connection
    accept_client(server_socket, client_socket, error);
    if (error) {
      // TODO: handle error conditions here and attempt to reconnect
      // could use kqueue/epoll to detect when we can connect again
      fatal_with_errno("Unable to accept client");
    }
    
    // pass it to a read thread
    bytes = write(read_queue_writer, &client_socket, sizeof(client_socket));
    if (bytes == 0) {
      warn("Read queue has closed. Shutting down.");
      break;
    } else if (bytes == -1) {
      fatal_with_errno("Unable to write new connection socket filedes");
    } else if (bytes != sizeof(int)) {
      fatal_with_format("Unable to write complete socket file descriptor after connect. Wrote: %i", bytes);
    }
    
    client_socket = 0;
    bytes = 0;
  }
  
  cleanup_server();
}
