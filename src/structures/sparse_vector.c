#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "core/logging.h"
#include "structures/sparse_vector.h"

learner_error sparse_vector_new(SparseVector **vector, Matrix *matrix) {
  if(!matrix) return MISSING_MATRIX;
  *vector = (SparseVector *) calloc(1, sizeof(SparseVector));
  (*vector)->header.min_index  = -1;
  (*vector)->header.max_index  = -1;
  (*vector)->matrix = matrix;
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


// internal binary search through a sparse vector to find element indexes
int sparse_vector_value_index(SparseVector *vector, u_int32_t index, u_int32_t *hint) {
  int low = 0;
  int high = vector->header.count - 1;
  int mid = high / 2;
  
  // branch prediction makes this triple clause if statement faster
  // than a double clause "single comparison" search. precondition
  // loops also seem to be faster than post condition loops in GCC,
  // really don't know why... this implementation ends up being
  // around 30% faster than well known single comparison versions.
  while(low <= high) {
    if (vector->values[mid].index < index) {
      low = mid + 1;
    } else if(vector->values[mid].index > index) {
      high = mid - 1;
    } else {
      return mid;
    }
    mid = (high + low) / 2;
  }
  
  if(hint)
    *hint = high + 1;
  return -1;
}


learner_error sparse_vector_set(SparseVector *vector, u_int32_t index, float value) {
  if(!vector) return MISSING_VECTOR;
  if(index < 0) return INDEX_OUT_OF_RANGE;
  
  int i = -1, hint = -1;
  i = sparse_vector_value_index(vector, index, &hint);
  
  if(i == -1) {
    if(vector->header.buffer_remaining == 0) {
      vector->values = realloc(vector->values, (vector->header.count + vector->matrix->buffer_delta) * sizeof(sparse_vector_value));
      vector->header.buffer_remaining = vector->matrix->buffer_delta;
    }
    // when we set a new value we need to shift all the values with indexes greater
    // than the new value up by one. if we were to copy from left to right values
    // would obliterate each other (e.g a, b, c shifted by one would overwrite b
    // with a before overwriting c with a again). copy from right to left is very
    // slow because the CPU caches from left to right, so we create a temporary
    // buffer, copy the necessary values to it, then copy it back.
    int bytes = (vector->header.count - hint) * sizeof(sparse_vector_value);
    sparse_vector_value *block = (sparse_vector_value *) malloc(bytes);
    memcpy(block, vector->values + hint, bytes);
    memcpy(vector->values + hint + 1, block, bytes);
    free(block);
    
    vector->values[hint].value = value;
    vector->values[hint].index = index;
    
    if(index < vector->header.min_index || vector->header.min_index == -1)
      vector->header.min_index = index;
    if(index > vector->header.max_index || vector->header.max_index == -1)
      vector->header.max_index = index;
    
    vector->header.count++;
    vector->header.buffer_remaining--;  
    return NO_ERROR;
    
  } else {
    vector->values[i].value = value;
    return NO_ERROR;
  }
}


learner_error sparse_vector_get(SparseVector *vector, u_int32_t index, float *value) {
  if(!vector) return MISSING_VECTOR;
  if(index < vector->header.min_index || index > vector->header.max_index) return INDEX_OUT_OF_RANGE;
  
  u_int32_t i = sparse_vector_value_index(vector, index, NULL);
  
  if(i != -1) {
    *value = vector->values[i].value;
    return NO_ERROR;
  } else {
    *value = 0.0;
    return INDEX_NOT_FOUND;
  }
}


learner_error sparse_vector_dot_product(SparseVector *v1, SparseVector *v2, float *result) {
  if(!v1 || !v2) return MISSING_VECTOR;
  if(v1->header.count == 0 || v2->header.count == 0) {*result = 0.0; return NO_ERROR;}
  
  // TODO: binary search or fixed length jumping. rather than just iterating through
  // each entry to find the next entry we're interested in, we can use a binary search
  // to help us 'jump' through the list, like skip lists. We could aso use fixed length
  // jumps (such as every 10 elements, or one quarter of the list).
  int v1_count = v1->header.count, v2_count = v2->header.count;
  int v1_pos = 0, v2_pos = 0;
  *result = 0.0;
  
  while(v1_pos < v1_count && v2_pos < v2_count) {
    if(v1->values[v1_pos].index == v2->values[v2_pos].index) {
      *result += v1->values[v1_pos].value * v2->values[v2_pos].value;
      v1_pos++;
      v2_pos++;
    } else if(v1->values[v1_pos].index < v2->values[v2_pos].index) {
      v1_pos++;
    } else {
      v2_pos++;
    }
  }
  
  return NO_ERROR;
}


learner_error sparse_vector_magnitude(SparseVector *vector, float *result) {
  if(!vector) return MISSING_VECTOR;
  if(vector->header.frozen) {*result = vector->header.magnitude; return NO_ERROR;}
  if(vector->header.count == 0) {*result = 0.0; return NO_ERROR;}

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
  if(v1->header.count == 0)
    return sparse_vector_magnitude(v2, result);
  else if(v2->header.count == 0)
    return sparse_vector_magnitude(v1, result);
  
  int v1_count = v1->header.count, v2_count = v2->header.count;
  int v1_pos = 0, v2_pos = 0;
  *result = 0.0;
  
  // TODO: change loop condition so we don't need the 'catch up' loop below
  while(v1_pos < v1_count && v2_pos < v2_count) {
    if(v1->values[v1_pos].index == v2->values[v2_pos].index) {
      *result += powf(v1->values[v1_pos].value - v2->values[v2_pos].value, 2);
      v1_pos++;
      v2_pos++;
    } else if(v1->values[v1_pos].index < v2->values[v2_pos].index) {
      *result += powf(v1->values[v1_pos].value, 2);
      v1_pos++;
    } else {
      *result += powf(v2->values[v2_pos].value, 2);
      v2_pos++;
    }
  }
  
  // if the length of the vectors is uneven, 'catch up' the remainder
  // of the longer vector here
  if(v1_pos < v1_count) {
    for(; v1_pos < v1_count; v1_pos++)
      *result += powf(v1->values[v1_pos].value, 2);
  } else if(v2_pos < v2_count) {
    for(; v2_pos < v2_count; v2_pos++)
      *result += powf(v2->values[v2_pos].value, 2);
  }
  
  *result = sqrtf(*result);
  return NO_ERROR;
}
