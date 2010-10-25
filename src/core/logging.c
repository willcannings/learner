#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "core/logging.h"
#include "core/errors.h"

learner_error set_learner_logging_level(learner_logging_level new_level) {
  int error = pthread_mutex_lock(&logging_lock);
  if(error) {
    fprintf(stderr, "Unable to acquire logging lock, error: %i\n", error);
    exit(1);
  }
  
  current_learner_logging_level = new_level;
  error = pthread_mutex_unlock(&logging_lock);
  if(error) {
    fprintf(stderr, "Unable to release logging lock, error: %i\n", error);
    exit(1);
  }

  return NO_ERROR;
}


learner_error set_learner_logging_file(char *path) {
  int error = pthread_mutex_lock(&logging_lock);
  if(error) {
    fprintf(stderr, "Unable to acquire logging lock, error: %i\n", error);
    exit(1);
  }
  
  // attempt to create/open the log file
  learner_logging_file = fopen(path, "a");
  
  // fatal condition if we can't open the file
  if(!learner_logging_file) {
    fprintf(stderr, "Unable to open or create logging file, error: %i", errno);
    exit(1);
  }
  
  error = pthread_mutex_unlock(&logging_lock);
  if(error) {
    fprintf(stderr, "Unable to release logging lock, error: %i\n", error);
    exit(1);
  }
  
  return NO_ERROR;
}


void learner_log_msg(learner_logging_level level, char *description) {
  // ignore the notice if it's below what's being logged
  if(level < current_learner_logging_level)
    return;
  
  // make sure we have a description to write
  if(!description) {
    fprintf(learner_logging_file, "WARN: Call to log without a description\n");
    return;
  }
  
  // attempt to write the log entry
  time_t current_time = time(NULL);
  struct tm *t = localtime(&current_time);
  int bytes = fprintf(learner_logging_file, "%d/%.2d/%.2d %.2d:%.2d:%.2d%s: %s\n", t->tm_year, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, learner_logging_level_names[level], description);
  
  // if writing failed, we have a fatal condition
  if(bytes < 0) {
    fprintf(stderr, "Fatal error writing to log file. Error: %i\n", bytes);
    exit(1);
  }
  
  // if the log entry was a fatal entry, we need to die
  if(level == FATAL)
    exit(1);
}
