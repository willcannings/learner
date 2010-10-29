#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "structures/sparse_vector.h"

// hint provided to indicate if an index is, or would be,
// in the upper or lower half of the value array
#define LOWER_HALF_HINT 0
#define UPPER_HALF_HINT 1


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


// internal binary search through a sparse vector to find if an element
// exists, and what position it exists in. also supply a hint to the
// calling function indicating if the index is, or would be, in the upper
// or lower half of the array (used to determine algorithm strategies)
int sparse_vector_value_index(SparseVector *vector, u_int32_t index, int *hint) {
  if(vector->header.count == 0) {
    *hint = UPPER_HALF_HINT;
    return -1;
  }
  
  int low = 0;
  int high = vector->header.count - 1;
  int mid = (high / 2) + 1;
  
  // set the hint
  if(index <= vector->values[mid].index) {
    *hint = LOWER_HALF_HINT;
  } else {
    *hint = UPPER_HALF_HINT;
  }
  
  // single comparison binary search
  while(low < high) {
    if (vector->values[mid].index < index) {
      low = mid + 1;
    } else {
      high = mid;
    }
    mid = low + ((high - low) / 2);
  }

  // since we can't early terminate from the loop, 'fix' our check against index here
  if((low < vector->header.count) && (vector->values[low].index == index)) {
    return low;
  } else {
    return -1;
  }
}

// TODO: the length of other rows/columns (or even a param) can be a hint we
// can use to pre-allocate a vector so reallocations are reduced
learner_error sparse_vector_set(SparseVector *vector, u_int32_t index, float value) {
  if(!vector) return MISSING_VECTOR;
  if(index < 0) return INDEX_OUT_OF_RANGE;
  
  // there are three strategies we take for setting a value:
  // - index already exists: simply set the value
  // - index is in the lower half of the existing values: copy the array to
  // a new allocation and set the value (faster than a shift)
  // - index is in the upper half of the existing values: realloc the array and
  // shift the existing values larger than the new value up one (faster than copy)
  
  int hint = -1;
  int i = sparse_vector_value_index(vector, index, &hint);
  
  if(i != -1) {
    vector->values[i].value = value;
    return NO_ERROR;
  // FIXME: lower half copy needs to be implemented
  } else {
    // resize the values array to hold the new value
    vector->values = realloc(vector->values, (vector->header.count + 1) * sizeof(sparse_vector_value));
  
    // shift values with indexes greater than index up one position. we
    // need to iterate backwards so we don't obliterate values as we
    // copy them (i.e shifting A, B, C up one position would copy A over
    // B if we performed the shift in ascending order)
    i = vector->header.count;
    for(; vector->values[i - 1].index > index; i--) {
      vector->values[i].value = vector->values[i - 1].value;
      vector->values[i].index = vector->values[i - 1].index;
    }
  }
  
  // set the new value
  vector->values[i].index = index;
  vector->values[i].value = value;
  
  // update min/max indexes, and count
  if(index < vector->header.min_index || vector->header.min_index == -1)
    vector->header.min_index = index;
  if(index > vector->header.max_index || vector->header.max_index == -1)
    vector->header.max_index = index;
  vector->header.count++;
  
  return NO_ERROR;
}


learner_error sparse_vector_get(SparseVector *vector, u_int32_t index, float *value) {
  if(!vector) return MISSING_VECTOR;
  if(index < vector->header.min_index || index > vector->header.max_index) return INDEX_OUT_OF_RANGE;
  
  u_int32_t i = 0;
  int hint = 0;
  i = sparse_vector_value_index(vector, index, &hint);
  
  if(i != -1) {
    *value = vector->values[i].value;
    return NO_ERROR;
  } else {
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
