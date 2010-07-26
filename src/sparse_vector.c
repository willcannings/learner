#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "sparse_vector.h"

SparseVector *sparse_vector_new() {
  SparseVector *vector = (SparseVector *) malloc(sizeof(SparseVector));
  vector->count = 0;
  vector->klass  = 0;
  vector->_frozen = 0;
  vector->_magnitude = 0.0;
  vector->_min_index = -1;
  vector->_max_index = -1;
  
  // postconditions
  assert(vector);
  return vector;
}


void sparse_vector_free(SparseVector *vector) {
  // preconditions
  assert(vector);
  
  // todo - test vector->values true & false
  if(vector->values)
    free(vector->values);
  free(vector);
}


void sparse_vector_freeze(SparseVector *vector) {
  // no-op if the vector has already been frozen
  assert(vector);
  if(vector->_frozen)
    return;
  
  vector->_magnitude = sparse_vector_magnitude(vector);
  vector->_frozen = 1;
}


char sparse_vector_frozen(SparseVector *vector) {
  // preconditions
  assert(vector);
  return vector->_frozen == 1;
}


float sparse_vector_dot_product(SparseVector *v1, SparseVector *v2) {
  // if either vector is length 0, the dot product will always be 0
  assert(v1);
  assert(v2);
  if(v1->count == 0 || v2->count == 0)
    return 0.0;

  // preconditions
  assert(v1->count == v2->count);
  assert(v1->values);
  assert(v2->values);

  float accumulator = 0.0;
  for(int i = 0, count = v1->count; i < count; i++)
    accumulator += v1->values[i].value * v2->values[i].value;
  return accumulator;
}


float sparse_vector_magnitude(SparseVector *vector) {
  // if we've cached the magnitude of this vector, return it immediately
  assert(vector);
  if(vector->_frozen)
    return vector->_magnitude;

  // preconditions  
  if(vector->count > 0)
    assert(vector->values);
  else
    return 0.0;

  float accumulator = 0.0;
  for(int i = 0, count = vector->count; i < count; i++)
    accumulator += powf(vector->values[i].value, 2);
  return sqrtf(accumulator);
}


// TODO: add optimisation for euclidean normal 1 vectors (just return dot product)
float sparse_vector_cosine_similarity(SparseVector *v1, SparseVector *v2) {
  return sparse_vector_dot_product(v1, v2) / (sparse_vector_magnitude(v1) * sparse_vector_magnitude(v2));
}


float sparse_vector_euclidean_distance(SparseVector *v1, SparseVector *v2) {
  // if one vector is length 0, the euclidean distance is equivalent to the magnitude of the other vector
  assert(v1);
  assert(v2);
  if(v1->count == 0)
    return sparse_vector_magnitude(v2);
  if(v2->count == 0)
    return sparse_vector_magnitude(v1);
  
  // preconditions
  assert(v1->values);  
  assert(v2->values);

  float accumulator = 0.0;
  for(int i = 0, count = v1->count; i < count; i++)
    accumulator += powf(v1->values[i].value - v2->values[i].value, 2);
  return sqrtf(accumulator);
}
