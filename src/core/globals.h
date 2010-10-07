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
pthread_key_t learner_error_key;
char *learner_error_codes[] = {
  "No error",
  "Invalid length",
  "Missing vector",
  "Index out of range",
  "Vectors are not of equal length",
  "Values of the supplied vector are missing",
  "There is no value at index in this sparse vector"
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

#endif
