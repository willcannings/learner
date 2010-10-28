#include <pthread.h>
#include <stdio.h>
#include <errno.h>
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
void learner_log_msg(learner_logging_level level, char *description);


// helpers to create a formatted log message
#define learner_log_message(level, description) {\
  if(current_learner_logging_level <= level) {\
    learner_log_msg(level, description);\
  }\
}

#define learner_log_with_format(level, format, ...) {\
  if(current_learner_logging_level <= level) {\
    char *_log_message = NULL;\
    int _log_err = asprintf(&_log_message, format, __VA_ARGS__);\
    if(_log_err < 0) {\
      learner_log_msg(FATAL, "FATAL: Unable to create memory after call to learner_log_with_format");\
    }\
    learner_log_msg(level, _log_message);\
    free(_log_message);\
  }\
}

#define learner_log_with_error(level, description, err)  {\
  if(current_learner_logging_level <= level) {\
    char *_log_message = NULL;\
    int _log_err = asprintf(&_log_message, "%s: %s", description, learner_error_codes[err]);\
    if(_log_err < 0) {\
      learner_log_msg(FATAL, "FATAL: Unable to create memory after call to learner_log_with_error");\
    }\
    learner_log_msg(level, _log_message);\
    free(_log_message);\
  }\
}

#define learner_log_with_errno(level, description)  {\
  if(current_learner_logging_level <= level) {\
    char *_log_message = NULL;\
    int _log_err = asprintf(&_log_message, "%s: %s", description, strerror(errno));\
    if(_log_err < 0) {\
      learner_log_msg(FATAL, "FATAL: Unable to create memory after call to learner_log_with_errno");\
    }\
    learner_log_msg(level, _log_message);\
    free(_log_message);\
  }\
}


// utility macros for the various log levels
#define debug(description)  learner_log_message(DEBUG, description);
#define note(description)   learner_log_message(NOTE,  description);
#define warn(description)   learner_log_message(WARN,  description);
#define fatal(description)  learner_log_message(FATAL, description);

#define debug_with_format(description, ...)  learner_log_with_format(DEBUG, description, __VA_ARGS__);
#define note_with_format(description,  ...)  learner_log_with_format(NOTE,  description, __VA_ARGS__);
#define warn_with_format(description,  ...)  learner_log_with_format(WARN,  description, __VA_ARGS__);
#define fatal_with_format(description, ...)  learner_log_with_format(FATAL, description, __VA_ARGS__);

#define debug_with_error(description, err)  learner_log_with_error(DEBUG, description, err);
#define note_with_error(description,  err)  learner_log_with_error(NOTE,  description, err);
#define warn_with_error(description,  err)  learner_log_with_error(WARN,  description, err);
#define fatal_with_error(description, err)  learner_log_with_error(FATAL, description, err);

#define debug_with_errno(description) learner_log_with_errno(DEBUG, description);
#define note_with_errno(description)  learner_log_with_errno(NOTE,  description);
#define warn_with_errno(description)  learner_log_with_errno(WARN,  description);
#define fatal_with_errno(description) learner_log_with_errno(FATAL, description);

#endif
