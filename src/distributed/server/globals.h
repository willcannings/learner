#include <tchdb.h>

#ifndef __learner_server_globals__
#define __learner_server_globals__

// tokyo db reference
TCHDB *db = NULL;

// unnamed sockets used for queue syncro
int read_queue_reader = 0;
int read_queue_writer = 0;
int process_queue_reader = 0;
int process_queue_writer = 0;

// state
int shutting_down = 0;

#endif
