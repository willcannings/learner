#include <pthread.h>
#include <stdio.h>
#include "errors.h"

#ifndef __learner_logging__
#define __learner_logging__

// logging levels
typedef enum {
  DEBUG,
  NOTE,
  WARN,
  FATAL
} learner_logging_level;
extern char *learner_logging_level_names[];

// globals related to the log file and level
extern learner_logging_level current_learner_logging_level;
extern FILE                  *learner_logging_file;
extern pthread_mutex_t       logging_lock;

// logging functions
learner_error set_learner_logging_level(learner_logging_level new_level);
learner_error set_learner_logging_file(char *path);
void log_msg(learner_logging_level level, char *description);

// utility macros for the various log levels
#define debug(description)  log_msg(DEBUG, description);
#define note(description)   log_msg(NOTE, description);
#define warn(description)   log_msg(WARN, description);
#define fatal(description)  log_msg(FATAL, description);

#endif
