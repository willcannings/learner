#include "distributed/protocol/protocol.h"
#include "distributed/server/server.h"
#include "core/logging.h"
#include <pthread.h>
#include <stdlib.h>

void process_thread_cleanup(void *param) {
  if (param) {
    if (*((int *)param))
      close(*((int *)param));
    free(param);
  }
}

void *process_thread(void *param) {
  learner_response *res = NULL;
  learner_request *req = NULL;
  int bytes = 0, error = 0;
  
  // we store a reference to the current client, allowing cleanup to close the connection
  int *current_client = (int *) malloc(sizeof(int));
  pthread_cleanup_push(process_thread_cleanup, (void *)current_client);
  *current_client = 0;
  note("Process thread started");
  
  while(1) {
    // get the next client to process
    bytes = read(process_queue_reader, current_client, sizeof(int));
    if (bytes != sizeof(int)) {
      if (shutting_down) {
        pthread_exit(NULL);
      } else if (bytes == 0) {
        note("Process queue has closed. Process thread shutting down.");
        pthread_exit(NULL);
      } else if (bytes == -1) {
        fatal_with_errno("Error reading from process queue");
      } else {
        fatal("Unexpected termination of process queue");
      }
    }
    
    read_learner_request(req, *current_client, error);
    if (error) {
      if(shutting_down) {
        pthread_exit(NULL);
      } else {
        warn_with_errno("Error reading request from client");
        close(*current_client);
        continue;
      }
    }
    
    init_learner_response(res);
    switch (get_learner_request_item(req)) {
      case KEY_VALUE:
        debug("Key value request");
        switch (get_learner_request_operation(req)) {
          case SET:
            error = handle_set_key_value(req, res);
            break;
          case GET:
            error = handle_get_key_value(req, res);
            break;
          case DELETE:
            error = handle_delete_key_value(req, res);
            break;
        }
        break;
        
      case MATRIX:
        debug("Matrix request");
        break;
        
      case ROW:
        debug("Row request");
        break;
      case COLUMN:
        debug("Column request");
        break;
        
      case CELL:
        debug("Cell request");
        break;
    }
    
    // write the response
    debug("Writing response");
    write_learner_response(res, *current_client, error);
    if (error) {
      if (shutting_down) {
        pthread_exit(NULL);
      } else {
        warn_with_errno("Unable to write complete response to client");
        close(*current_client);
        continue;
      }
    }
    
    // free the response and finish
    free_learner_response(res);
    res = NULL;
    debug("Completed request");
    
    // move the client back to the read queue
    bytes = write(read_queue_writer, current_client, sizeof(int));
    if (bytes != sizeof(int)) {
      if (shutting_down) {
        pthread_exit(NULL);
      } else if (bytes == 0) {
        note("Read queue has closed. Process thread shutting down.");
        pthread_exit(NULL);
      } else if (bytes == -1) {
        fatal_with_errno("Unable to write client socket file descriptor back to read queue");
      } else {
        fatal_with_format("Unable to write complete socket file descriptor back to read queue. Wrote: %i", bytes);
      }
    }
  }
  
  pthread_cleanup_pop(1);
  return NULL;
}
