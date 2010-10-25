#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "learner.h"
#include "core/globals.h"

learner_error learner_initialize() {
  // initialise logging
  int error = pthread_mutex_init(&logging_lock, NULL);
  if(error) {
    fprintf(stderr, "Failed to create logging_lock, error: %i\n", error);
    exit(1);
  }
  set_learner_logging_level(DEBUG);
  learner_logging_file = stderr;
  return NO_ERROR;
}
