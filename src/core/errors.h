#include <pthread.h>

#ifndef __learner_errors__
#define __learner_errors__

extern char *learner_error_codes[];
typedef enum {
  NO_ERROR,
  INVALID_LENGTH,
  MISSING_VECTOR,
  INDEX_OUT_OF_RANGE,
  VECTORS_NOT_OF_EQUAL_LENGTH,
  MISSING_VALUES,
  INDEX_NOT_FOUND
} learner_error;

#endif
