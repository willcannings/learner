#include "distributed/server/server.h"
#include "core/logging.h"

static TCHDB *db;
int server = 0;
int client = 0;

void cleanup() {
  note("Preparing to quit... cleaning up");

  if(!tchdbclose(db))
    fatal_with_format("Error closing the database: %s", tchdberrmsg(tchdbecode(db)));
  tchdbdel(db);

  note("Exiting");
  exit(0);
}


int main(void) {
  learner_request  *req = NULL;
  learner_response *res = NULL;
  int error = 0;
  
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
  note("Server started, waiting for requests");
  
  // in testing there's only a single connection per server
  accept_client(server, client, error);
  if(error)
    fatal_with_format("Unable to accept client: %i", error);
  
  while(1) {
    read_learner_request(req, client, error);
    if(error) {
      warn_with_format("Error reading request from client: %i", error);
      continue;
    }
    
    
  }
}