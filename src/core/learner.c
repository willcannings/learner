#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "learner.h"
#include "core/globals.h"

void learner_initialize() {
  // initialise errors
  int error = 0;
  error = pthread_key_create(&learner_error_key, NULL);
  if(error) {
    fprintf(stderr, "Failed to create pthread key learner_error, error: %i\n", error);
    exit(1);
  }
  
  // initialise logging
  error = pthread_mutex_init(&logging_lock, NULL);
  if(error) {
    fprintf(stderr, "Failed to create logging_lock, error: %i\n", error);
    exit(1);
  }
  set_learner_logging_level(WARN);
  learner_logging_file = stderr;
}
