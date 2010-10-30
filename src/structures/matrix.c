#include <stdlib.h>
#include "core/errors.h"
#include "core/logging.h"
#include "matrix.h"

learner_error matrix_new(Matrix **matrix) {
  *matrix = (Matrix *) calloc(1, sizeof(Matrix));
  (*matrix)->buffer_delta = LEARNER_DEFAULT_BUFFER_DELTA;
  return NO_ERROR;
}

learner_error matrix_free(Matrix *matrix) {
  if(!matrix) return MISSING_MATRIX;
  free(matrix);
  return NO_ERROR;
}
