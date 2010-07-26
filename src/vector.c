#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vector.h"

Vector *vector_new(int length) {
  // preconditions
  assert(length > 0);
  
  Vector *vector = (Vector *) malloc(sizeof(Vector));
  vector->length = length;
  vector->klass  = 0;
  vector->_frozen = 0;
  vector->_magnitude = 0.0;
  vector->values = (float *) calloc(sizeof(float), length);
  
  // postconditions
  assert(vector);
  assert(vector->values);
  return vector;
}


void vector_free(Vector *vector) {
  // preconditions
  assert(vector);
  assert(vector->values);
  
  free(vector->values);
  free(vector);
}


void vector_freeze(Vector *vector) {
  // no-op if the vector has already been frozen
  assert(vector);
  if(vector->_frozen)
    return;
  assert(vector->values);

  vector->_magnitude = vector_magnitude(vector);
  vector->_frozen = 1;
}


char vector_frozen(Vector *vector) {
  // preconditions
  assert(vector);
  return vector->_frozen == 1;
}


float vector_dot_product(Vector *v1, Vector *v2) {
  // preconditions
  assert(v1);
  assert(v2);
  assert(v1->length == v2->length);
  assert(v1->values);
  assert(v2->values);
  
  float accumulator = 0.0;
  for(int i = 0, length = v1->length; i < length; i++)
    accumulator += v1->values[i] * v2->values[i];
  return accumulator;
}


float vector_magnitude(Vector *vector) {
  // if we've cached the magnitude of this vector, return it immediately
  assert(vector);
  if(vector->_frozen)
    return vector->_magnitude;
  else
    assert(vector->values);
  
  float accumulator = 0.0;  
  for(int i = 0, length = vector->length; i < length; i++)
    accumulator += powf(vector->values[i], 2);
  return sqrtf(accumulator);
}


// TODO: add optimisation for euclidean normal 1 vectors (just return dot product)
float vector_cosine_similarity(Vector *v1, Vector *v2) {
  return vector_dot_product(v1, v2) / (vector_magnitude(v1) * vector_magnitude(v2));
}


float vector_euclidean_distance(Vector *v1, Vector *v2) {
  // preconditions
  assert(v1);
  assert(v2);
  assert(v1->length == v2->length);
  assert(v1->values);
  assert(v2->values);
  
  float accumulator = 0.0;
  for(int i = 0, length = v1->length; i < length; i++)
    accumulator += powf(v1->values[i] - v2->values[i], 2);
  return sqrtf(accumulator);
}
