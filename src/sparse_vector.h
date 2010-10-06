#include "errors.h"
#include <sys/types.h>

#ifndef __learner_sparse_vector__
#define __learner_sparse_vector__

#pragma pack(push)
#pragma pack(1)
typedef struct {
  u_int32_t index;
  float     value;
} sparse_vector_value;

typedef struct {
  u_int32_t count;
  u_int8_t  _frozen;
  float     _magnitude;
  u_int32_t _min_index;
  u_int32_t _max_index;  
} sparse_vector_header;

typedef struct {
  sparse_vector_header  header;
  sparse_vector_value   *values;
} SparseVector;
#pragma pack(pop)


// core functions
SparseVector *sparse_vector_new(void);
void    sparse_vector_free(SparseVector *vector);
void    sparse_vector_freeze(SparseVector *vector);
char    sparse_vector_frozen(SparseVector *vector);
void    sparse_vector_unfreeze(SparseVector *vector);

// getter & setter required because we don't have contiguous data
void    sparse_vector_set(SparseVector *vector, int index, float value);
float   sparse_vector_get(SparseVector *vector, int index);

// calculations
float   sparse_vector_dot_product(SparseVector *v1, SparseVector *v2);
float   sparse_vector_magnitude(SparseVector *vector);
float   sparse_vector_cosine_similarity(SparseVector *v1, SparseVector *v2);
float   sparse_vector_euclidean_distance(SparseVector *v1, SparseVector *v2);

#endif
