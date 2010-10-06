#include <stdlib.h>
#include <math.h>
#include "logging.h"
#include "errors.h"
#include "vector.h"

learner_error vector_new(int length, Vector **vector) {
  if(length <= 0) return INVALID_LENGTH;
  *vector = (Vector *) calloc(1, sizeof(Vector));
  (*vector)->header.length = length;
  (*vector)->values = (float *) calloc(sizeof(float), length);
  return NO_ERROR;
}


learner_error vector_free(Vector *vector) {
  if(!vector) return MISSING_VECTOR;
  if(!vector->values) return MISSING_VALUES;
  free(vector->values);
  free(vector);
  return NO_ERROR;
}


learner_error vector_freeze(Vector *vector) {
  if(!vector) return MISSING_VECTOR;
  if(!vector->header._frozen) {
    learner_error error = vector_magnitude(vector, &vector->header._magnitude);
    if(error) return error;
    vector->header._frozen = 1;
  }
  return NO_ERROR;
}


learner_error vector_frozen(Vector *vector, int *frozen) {
  if(!vector) return MISSING_VECTOR;
  *frozen = (vector->header._frozen == 1);
  return NO_ERROR;
}


learner_error vector_unfreeze(Vector *vector) {
  if(!vector) return MISSING_VECTOR;
  vector->header._frozen = 0;
  return NO_ERROR;
}


learner_error vector_set(Vector *vector, int index, float value) {
  if(!vector) return MISSING_VECTOR;
  if(index < 0 || index > (vector->header.length - 1)) return INDEX_OUT_OF_RANGE;
  vector->values[index] = value;
  return NO_ERROR;
}


learner_error vector_get(Vector *vector, int index, float *value) {
  if(!vector) return MISSING_VECTOR;
  if(index < 0 || index > (vector->header.length - 1)) return INDEX_OUT_OF_RANGE;
  *value = vector->values[index];
  return NO_ERROR;
}


learner_error vector_dot_product(Vector *v1, Vector *v2, float *result) {
  if(!v1 || !v2) return MISSING_VECTOR;
  if(v1->header.length != v2->header.length) return VECTORS_NOT_OF_EQUAL_LENGTH;
  *result = 0.0;
  for(int i = 0, length = v1->header.length; i < length; i++)
    *result += v1->values[i] * v2->values[i];
  return NO_ERROR;
}


learner_error vector_magnitude(Vector *vector, float *result) {
  if(!vector) return MISSING_VECTOR;
  if(vector->header._frozen) {*result = vector->header._magnitude; return NO_ERROR;}
  *result = 0.0;
  for(int i = 0, length = vector->header.length; i < length; i++)
    *result += powf(vector->values[i], 2);
  *result = sqrtf(*result);
  return NO_ERROR;
}


// TODO: add optimisation for euclidean normal 1 vectors (just return dot product)
learner_error vector_cosine_similarity(Vector *v1, Vector *v2, float *result) {
  float dot_product, magnitude_v1, magnitude_v2;
  learner_error error;
  
  if(error = vector_dot_product(v1, v2, &dot_product))
    return error;
  
  if(error = vector_magnitude(v1, &magnitude_v1))
    return error;
  
  if(error = vector_magnitude(v2, &magnitude_v2))
    return error;
  
  *result = dot_product / (magnitude_v1 * magnitude_v2);
  return NO_ERROR;
}


learner_error vector_euclidean_distance(Vector *v1, Vector *v2, float *result) {
  if(!v1 || !v2) return MISSING_VECTOR;
  if(v1->header.length != v2->header.length) return VECTORS_NOT_OF_EQUAL_LENGTH;
  *result = 0.0;
  for(int i = 0, length = v1->header.length; i < length; i++)
    *result += powf(v1->values[i] - v2->values[i], 2);
  *result = sqrtf(*result);
  return NO_ERROR;
}
