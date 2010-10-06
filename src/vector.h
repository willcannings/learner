#include <sys/types.h>
#include "errors.h"

#ifndef __learner_vector__
#define __learner_vector__

#pragma pack(push)
#pragma pack(1)
typedef struct {
  u_int32_t length;
  u_int8_t  _frozen;
  float     _magnitude;  
} vector_header;

typedef struct {
  vector_header header;
  float *values;
} Vector;
#pragma pack(pop)

// core functions
learner_error vector_new(int length, Vector **vector);
learner_error vector_free(Vector *vector);
learner_error vector_freeze(Vector *vector);
learner_error vector_frozen(Vector *vector, int *frozen);
learner_error vector_unfreeze(Vector *vector);

// getter & setter to match the sparse vector functions
learner_error vector_set(Vector *vector, int index, float value);
learner_error vector_get(Vector *vector, int index, float *value);

// calculations
learner_error vector_dot_product(Vector *v1, Vector *v2, float *result);
learner_error vector_magnitude(Vector *vector, float *result);
learner_error vector_cosine_similarity(Vector *v1, Vector *v2, float *result);
learner_error vector_euclidean_distance(Vector *v1, Vector *v2, float *result);

#endif
