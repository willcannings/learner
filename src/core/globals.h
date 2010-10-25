// global constants and variables that should only be defined once
// for example, learner_error_key is defined in errors.h as extern
// all objects using these globals should link against the object
// which includes this file (typically learner.o) to have the
// extern references resolved

#include "logging.h"
#ifndef __learner_globals__
#define __learner_globals__

// ------------------------------------------
// errors
// ------------------------------------------
char *learner_error_codes[] = {
  "no error",
  "invalid length",
  "missing vector",
  "index out of range",
  "vectors are not of equal length",
  "values of the supplied vector are missing",
  "there is no value at index in this sparse vector",
  "communication error - check the log file for more information",
  "unknown distributed operation requested",
  "an operation requiring a name was requested, but no name was provided",
  "database error",
  "unknown key"
};

// ------------------------------------------
// logging
// ------------------------------------------
learner_logging_level current_learner_logging_level;
FILE                  *learner_logging_file;
pthread_mutex_t       logging_lock;
char *learner_logging_level_names[] = {
  " DEBUG ",
  " NOTE ",
  " WARN ",
  " FATAL "
};

// ------------------------------------------
// distributed api
// ------------------------------------------
char *learner_operation_names[] = {
  "get",
  "set",
  "delete"
};

char *learner_item_names[] = {
  "key/value",
  "matrix",
  "row",
  "column",
  "cell"
};

char *learner_attribute_names[] = {
  "value",
  "name",
  "index"
};

#endif
