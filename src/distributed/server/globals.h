#include <tchdb.h>

#ifndef __learner_server_globals__
#define __learner_server_globals__

// tokyo db reference
TCHDB *db;

// unnamed sockets used for queue syncro
int read_queue_reader;
int read_queue_writer;
int process_queue_reader;
int process_queue_writer;

#endif
