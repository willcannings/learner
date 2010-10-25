#include "distributed/server/server.h"
#include "core/logging.h"
#include "learner.h"

static TCHDB *db;
int server = 0;
int client = 0;

void cleanup() {
  note("Preparing to quit... cleaning up");
  
  // tokyo db
  if(!tchdbclose(db))
    fatal_with_format("Error closing the database: %s", tchdberrmsg(tchdbecode(db)));
  tchdbdel(db);
  
  // sockets
  if(client) close(client);
  if(server) close(server);
  
  note("Exiting");
  exit(0);
}


int main(void) {
  learner_request  *req = NULL;
  learner_response *res = NULL;
  int error = 0;
  
  // initialise learner logging
  learner_initialize();
  
  // before starting, install the cleanup signal handler
  signal(SIGQUIT, cleanup);
  signal(SIGTERM, cleanup);
  signal(SIGINT,  cleanup);
  
  // open the backing database
  db = tchdbnew();
  if(!tchdbopen(db, "learner.db", HDBOWRITER | HDBOCREAT))
    fatal_with_format("Unable to open the backing database: %s", tchdberrmsg(tchdbecode(db)));
  
  // create the server socket
  create_server_socket(3579, 1, server, error);
  if(error)
    fatal_with_format("Unable to open a server socket: %i", error);
  note("Server started");
  
  // in testing there's only a single connection per server
  accept_client(server, client, error);
  if(error)
    fatal_with_format("Unable to accept client: %i", error);
  
  while(1) {
    debug("Waiting for request");
    read_learner_request(req, client, error);
    if(error) {
      warn_with_format("Error reading request from client: %i", error);
      continue;
    }
    
    init_learner_response(res);
    switch(get_learner_request_item(req)) {
      case KEY_VALUE:
        debug("Key value request");
        switch(get_learner_request_operation(req)) {
          case SET:
            error = set_keyed_value(db, req, res);
            break;
          case GET:
            error = get_keyed_value(db, req, res);
            break;
          case DELETE:
            error = delete_keyed_value(db, req, res);
            break;
        }
        break;
      case MATRIX:
        break;
      case ROW:
        break;
      case COLUMN:
        break;
      case CELL:
        break;
    }
    
    // free the request object
    free(req->name);
    free_learner_request_structure(req);
    req = NULL;
    
    // write the response
    debug("Writing response");
    write_learner_response(res, client, error);
    
    // free the response and finish
    free_learner_response(res);
    res = NULL;
    debug("Completed request");
  }
}