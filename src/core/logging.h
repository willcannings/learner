#include <pthread.h>
#include <stdio.h>
#include "core/errors.h"

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

// helpers to create a formatted log message
#define log_with_int(level, description, i)  {\
  char *_log_message = NULL;\
  int _log_err = asprintf(&_log_message, description, i);\
  if(_log_err < 0) {\
    log_msg(FATAL, "FATAL: Unable to create memory after call to log_with_int");\
  }\
  log_msg(level, _log_message);\
  free(_log_message);\
}

#define log_with_string(level, description, s)  {\
  char *_log_message = NULL;\
  int _log_err = asprintf(&_log_message, description, s);\
  if(_log_err < 0) {\
    log_msg(FATAL, "FATAL: Unable to create memory after call to log_with_string");\
  }\
  log_msg(level, _log_message);\
  free(_log_message);\
}

#define log_with_string_and_int(level, description, s, i)  {\
  char *_log_message = NULL;\
  int _log_err = asprintf(&_log_message, description, s, i);\
  if(_log_err < 0) {\
    log_msg(FATAL, "FATAL: Unable to create memory after call to log_with_string_and_int");\
  }\
  log_msg(level, _log_message);\
  free(_log_message);\
}

#define log_with_error(level, description, err)  {\
  char *_log_message = NULL;\
  int _log_err = asprintf(&_log_message, "%s (%s)", description, learner_error_codes[err]);\
  if(_log_err < 0) {\
    log_msg(FATAL, "FATAL: Unable to create memory after call to log_with_int");\
  }\
  log_msg(level, _log_message);\
  free(_log_message);\
}



// utility macros for the various log levels
#define debug(description)  log_msg(DEBUG, desc);
#define note(description)   log_msg(NOTE,  desc);
#define warn(description)   log_msg(WARN,  desc);
#define fatal(description)  log_msg(FATAL, desc);

#define debug_with_int(desc, i)  log_with_int(DEBUG, desc, i);
#define note_with_int(desc,  i)  log_with_int(NOTE,  desc, i);
#define warn_with_int(desc,  i)  log_with_int(WARN,  desc, i);
#define fatal_with_int(desc, i)  log_with_int(FATAL, desc, i);

#define debug_with_string(desc, s)  log_with_string(DEBUG, desc, s);
#define note_with_string(desc,  s)  log_with_string(NOTE,  desc, s);
#define warn_with_string(desc,  s)  log_with_string(WARN,  desc, s);
#define fatal_with_string(desc, s)  log_with_string(FATAL, desc, s);

#define debug_with_string_and_int(desc, s, i) log_with_string_and_int(DEBUG, desc, s, i);
#define note_with_string_and_int(desc,  s, i) log_with_string_and_int(NOTE,  desc, s, i);
#define warn_with_string_and_int(desc,  s, i) log_with_string_and_int(WARN,  desc, s, i);
#define fatal_with_string_and_int(desc, s, i) log_with_string_and_int(FATAL, desc, s, i);

#define debug_with_error(desc, err)  log_with_error(DEBUG, desc, err);
#define note_with_error(desc,  err)  log_with_error(NOTE,  desc, err);
#define warn_with_error(desc,  err)  log_with_error(WARN,  desc, err);
#define fatal_with_error(desc, err)  log_with_error(FATAL, desc, err);

#endif
