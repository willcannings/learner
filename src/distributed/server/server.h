#include <tcutil.h>
#include <tchdb.h>
#include "distributed/protocol/protocol.h"
#include "config.h"

#ifndef __learner_server__
#define __learner_server__

// from globals.h
extern TCHDB *db;
extern int read_queue_reader;
extern int read_queue_writer;
extern int process_queue_reader;
extern int process_queue_writer;
extern int shutting_down;
extern learner_config config;

// core server functions
void initialize_server();
void cleanup_server();
void *read_thread(void *param);
void *process_thread(void *param);

// operation processers
learner_error handle_delete_key_value(learner_request *req, learner_response *res);
learner_error handle_get_key_value(learner_request *req, learner_response *res);
learner_error handle_set_key_value(learner_request *req, learner_response *res);

#endif
