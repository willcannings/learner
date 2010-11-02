#ifndef __learner_matrix__
#define __learner_matrix__

#define LEARNER_DEFAULT_BUFFER_DELTA  256

typedef struct {
  u_int64_t index;
  u_int64_t rows;
  u_int64_t columns;
  u_int32_t buffer_delta;
  char      *name;
} Matrix;

learner_error matrix_new(Matrix **matrix);
learner_error matrix_free(Matrix *matrix);

#endif
