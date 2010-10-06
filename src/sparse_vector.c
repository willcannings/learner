#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "sparse_vector.h"

learner_error sparse_vector_new(SparseVector **vector) {
  *vector = (SparseVector *) calloc(1, sizeof(SparseVector));
  (*vector)->header.min_index  = -1;
  (*vector)->header.max_index  = -1;
  return NO_ERROR;
}


learner_error sparse_vector_free(SparseVector *vector) {
  if(!vector) return MISSING_VECTOR;
  if(vector->values)
    free(vector->values);
  free(vector);
  return NO_ERROR;
}


learner_error sparse_vector_freeze(SparseVector *vector) {
  if(!vector) return MISSING_VECTOR;
  if(!vector->header.frozen) {
    learner_error error = sparse_vector_magnitude(vector, &vector->header.magnitude);
    if(error) return error;
    vector->header.frozen = 1;
  }
  return NO_ERROR;
}


learner_error sparse_vector_frozen(SparseVector *vector, int *frozen) {
  if(!vector) return MISSING_VECTOR;
  *frozen = (vector->header.frozen == 1);
  return NO_ERROR;
}


learner_error sparse_vector_unfreeze(SparseVector *vector) {
  if(!vector) return MISSING_VECTOR;
  vector->header.frozen = 0;
  return NO_ERROR;
}


learner_error sparse_vector_set(SparseVector *vector, int index, float value) {
  if(!vector) return MISSING_VECTOR;
  if(index < 0) return INDEX_OUT_OF_RANGE;
  
  // if we're inserting a value in between existing values, determine
  // how many values will exist below the value we're inserting
  int lower_values_count = 0;
  for(int i = 0, count = vector->header.count; i < count; i++) {
    if(vector->values[i].index == index) {
      vector->values[i].value = value;
      return NO_ERROR;
    }
    
    if(vector->values[i].index < index)
      lower_values_count++;
    else
      break;
  }
  
  // create a new values array, copy the values below the new value,
  // then the values following the new value, before setting the value
  sparse_vector_value *new_values = (sparse_vector_value *) malloc(sizeof(sparse_vector_value) * (vector->header.count + 1));
  memcpy((void *) new_values, (void *) vector->values, lower_values_count * sizeof(sparse_vector_value));
  memcpy((void *) new_values + ((lower_values_count + 1) * sizeof(sparse_vector_value)), (void *) vector->values + (lower_values_count * sizeof(sparse_vector_value)), (vector->header.count - lower_values_count) * sizeof(sparse_vector_value));
  new_values[lower_values_count].index = index;
  new_values[lower_values_count].value = value;
  
  if(vector->values)
    free(vector->values);
  vector->values = new_values;

  if(index < vector->header.min_index || vector->header.min_index == -1)
    vector->header.min_index = index;
  if(index > vector->header.max_index || vector->header.max_index == -1)
    vector->header.max_index = index;
  vector->header.count++;
  
  return NO_ERROR;
}


learner_error sparse_vector_get(SparseVector *vector, int index, float *value) {
  if(!vector) return MISSING_VECTOR;
  if(index < vector->header.min_index || index > vector->header.max_index) return INDEX_OUT_OF_RANGE;
  
  // TODO: binary search instead of linear
  for(int i = 0, count = vector->header.count; i < count; i++) {
    if(vector->values[i].index == index) {
      *value = vector->values[i].value;
      return NO_ERROR;
    }
  }

  return INDEX_NOT_FOUND;
}


learner_error sparse_vector_dot_product(SparseVector *v1, SparseVector *v2, float *result) {
  if(!v1 || !v2) return MISSING_VECTOR;
  if(v1->header.count == 0 || v2->header.count == 0) {
    *result = 0.0;
    return NO_ERROR;
  }

  *result = 0.0;
  for(int i = 0, count = v1->header.count; i < count; i++)
    *result += v1->values[i].value * v2->values[i].value;
  return NO_ERROR;
}


learner_error sparse_vector_magnitude(SparseVector *vector, float *result) {
  if(!vector) return MISSING_VECTOR;
  if(vector->header.frozen) {
    *result = vector->header.magnitude;
    return NO_ERROR;
  }

  if(vector->header.count == 0) {
    *result = 0.0;
    return NO_ERROR;
  }

  *result = 0.0;
  for(int i = 0, count = vector->header.count; i < count; i++)
    *result += powf(vector->values[i].value, 2);
  *result = sqrtf(*result);
  return NO_ERROR;
}


// TODO: add optimisation for euclidean normal 1 vectors (just return dot product)
learner_error sparse_vector_cosine_similarity(SparseVector *v1, SparseVector *v2, float *result) {
  float dot_product, magnitude_v1, magnitude_v2;
  learner_error error;
  
  if(error = sparse_vector_dot_product(v1, v2, &dot_product))
    return error;
  
  if(error = sparse_vector_magnitude(v1, &magnitude_v1))
    return error;
  
  if(error = sparse_vector_magnitude(v2, &magnitude_v2))
    return error;
  
  *result = dot_product / (magnitude_v1 * magnitude_v2);
  return NO_ERROR;
}


learner_error sparse_vector_euclidean_distance(SparseVector *v1, SparseVector *v2, float *result) {
  // if one vector is length 0, the euclidean distance is equivalent to the magnitude of the other vector
  if(!v1 || !v2) return MISSING_VECTOR;
  // TODO: are these opts valid for sparse vectors?
  if(v1->header.count == 0)
    return sparse_vector_magnitude(v2, result);
  else if(v2->header.count == 0)
    return sparse_vector_magnitude(v1, result);
  
  *result = 0.0;
  for(int i = 0, count = v1->header.count; i < count; i++)
    *result += powf(v1->values[i].value - v2->values[i].value, 2);
  *result = sqrtf(*result);
  return NO_ERROR;
}
