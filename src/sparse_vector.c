#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "sparse_vector.h"

SparseVector *sparse_vector_new() {
  SparseVector *vector  = (SparseVector *) calloc(1, sizeof(SparseVector));
  vector->_min_index    = -1;
  vector->_max_index    = -1;
  assert(vector);
  return vector;
}


void sparse_vector_free(SparseVector *vector) {
  assert(vector);
  
  // todo - test vector->values true & false
  if(vector->values)
    free(vector->values);
  free(vector);
}


void sparse_vector_freeze(SparseVector *vector) {
  assert(vector);
  if(vector->_frozen)
    return;
  
  vector->_magnitude = sparse_vector_magnitude(vector);
  vector->_frozen = 1;
}


char sparse_vector_frozen(SparseVector *vector) {
  assert(vector);
  return vector->_frozen == 1;
}


// TODO: test unfreeze
void sparse_vector_unfreeze(SparseVector *vector) {
  assert(vector);
  vector->_frozen = 0;
}


void sparse_vector_set(SparseVector *vector, int index, float value) {
  assert(vector);
  assert(index >= 0);
  
  // if we're inserting a value in between existing values, determine
  // how many values will exist below the value we're inserting
  int lower_values_count = 0;
  for(int i = 0, count = vector->count; i < count; i++) {
    if(vector->values[i].index == index) {
      vector->values[i].value = value;
      return;
    }
    
    if(vector->values[i].index < index)
      lower_values_count++;
    else
      break;
  }
  
  // create a new values array, copy the values below the new value,
  // then the values following the new value, before setting the value
  sparse_vector_value *new_values = (sparse_vector_value *) malloc(sizeof(sparse_vector_value) * (vector->count + 1));
  memcpy((void *) new_values, (void *) vector->values, lower_values_count * sizeof(sparse_vector_value));
  memcpy((void *) new_values + ((lower_values_count + 1) * sizeof(sparse_vector_value)), (void *) vector->values + (lower_values_count * sizeof(sparse_vector_value)), (vector->count - lower_values_count) * sizeof(sparse_vector_value));
  new_values[lower_values_count].index = index;
  new_values[lower_values_count].value = value;
  
  if(vector->values)
    free(vector->values);
  vector->values = new_values;

  if(index < vector->_min_index || vector->_min_index == -1)
    vector->_min_index = index;  
  if(index > vector->_max_index || vector->_max_index == -1)
    vector->_max_index = index;
  vector->count++;
}


float sparse_vector_get(SparseVector *vector, int index) {
  assert(vector);
  assert(index >= vector->_min_index && index <= vector->_max_index);
  
  // TODO: binary search instead of linear
  for(int i = 0, count = vector->count; i < count; i++)
    if(vector->values[i].index == index)
      return vector->values[i].value;
  
  // TODO: set error code
  return 0;
}


float sparse_vector_dot_product(SparseVector *v1, SparseVector *v2) {
  assert(v1);
  assert(v2);
  if(v1->count == 0 || v2->count == 0)
    return 0.0;

  assert(v1->count == v2->count);
  assert(v1->values);
  assert(v2->values);

  float accumulator = 0.0;
  for(int i = 0, count = v1->count; i < count; i++)
    accumulator += v1->values[i].value * v2->values[i].value;
  return accumulator;
}


float sparse_vector_magnitude(SparseVector *vector) {
  assert(vector);
  if(vector->_frozen)
    return vector->_magnitude;

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
  // TODO: are these opts valid for sparse vectors?
  if(v1->count == 0)
    return sparse_vector_magnitude(v2);
  if(v2->count == 0)
    return sparse_vector_magnitude(v1);
  
  assert(v1->values);  
  assert(v2->values);

  float accumulator = 0.0;
  for(int i = 0, count = v1->count; i < count; i++)
    accumulator += powf(v1->values[i].value - v2->values[i].value, 2);
  return sqrtf(accumulator);
}
