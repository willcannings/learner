#include <sys/types.h>
#include "core/errors.h"
#include "matrix.h"

#ifndef __learner_sparse_vector__
#define __learner_sparse_vector__

#pragma pack(push)
#pragma pack(1)
typedef struct {
  u_int32_t index;
  float     value;
} sparse_vector_value;

// attempt to align fields on 32/64 bit boundaries
typedef struct {
  u_int64_t count;
  u_int64_t min_index;
  u_int64_t max_index;
  u_int64_t matrix_index;
  u_int64_t buffer_remaining;
  float     magnitude;
  u_int8_t  frozen;
} sparse_vector_header;

typedef struct {
  sparse_vector_header  header;
  sparse_vector_value   *values;
  Matrix                *matrix;
} SparseVector;
#pragma pack(pop)


// core functions
learner_error sparse_vector_new(SparseVector **vector, Matrix *matrix);
learner_error sparse_vector_free(SparseVector *vector);
learner_error sparse_vector_freeze(SparseVector *vector);
learner_error sparse_vector_frozen(SparseVector *vector, int *frozen);
learner_error sparse_vector_unfreeze(SparseVector *vector);

// getter & setter required because we don't have contiguous data
learner_error sparse_vector_set(SparseVector *vector, u_int32_t index, float value);
learner_error sparse_vector_get(SparseVector *vector, u_int32_t index, float *value);

// calculations
learner_error sparse_vector_dot_product(SparseVector *v1, SparseVector *v2, float *result);
learner_error sparse_vector_magnitude(SparseVector *vector, float *result);
learner_error sparse_vector_cosine_similarity(SparseVector *v1, SparseVector *v2, float *result);
learner_error sparse_vector_euclidean_distance(SparseVector *v1, SparseVector *v2, float *result);

#endif
