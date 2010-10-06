#include "tests.h"

#define test_get_value(v, index, val) {\
  error = vector_get(v, index, &value);\
  test_error(error);\
  test_float(value, val);\
}

#define test_set_value(v, index, val) {\
  error = vector_set(v, index, val);\
  test_error(error);\
}

int test_vector() {
  starting_tests();
  learner_error error;
  Vector *v1, *v2;
  
  // creating
  error = vector_new(2, &v1);
  test_error(error);
  error = vector_new(2, &v2);
  test_error(error);
  
  // setting
  test_set_value(v1, 1, 2.0);
  test_set_value(v1, 0, 1.0);
  test_set_value(v2, 0, 3.0);
  test_set_value(v2, 1, 4.0);
  
  // getting
  float value;
  test_get_value(v1, 0, 1.0);
  test_get_value(v1, 1, 2.0);
  test_get_value(v2, 0, 3.0);
  test_get_value(v2, 1, 4.0);
  
  // freezing
  int frozen;
  error = vector_freeze(v1);
  test_error(error);
  error = vector_freeze(v2);
  test_error(error);
  error = vector_frozen(v1, &frozen);
  test_error(error);
  test(frozen);
  error = vector_frozen(v2, &frozen);
  test_error(error);
  test(frozen);
  
  // magnitude
  error = vector_magnitude(v1, &value);
  test_error(error);
  test_float(value, sqrtf(5.0));
  error = vector_magnitude(v2, &value);
  test_error(error);
  test_float(value, sqrtf(25.0));
  
  // euclidean distance
  error = vector_euclidean_distance(v1, v2, &value);
  test_error(error);
  test_float(value, sqrtf(4.0 + 4.0));
  
  // dot product
  error = vector_dot_product(v1, v2, &value);
  test_error(error);
  test_float(value, (3.0 + 8.0));
  
  // cosine similarity
  error = vector_cosine_similarity(v1, v2, &value);
  test_error(error);
  test_float(value, ((3.0 + 8.0) / (sqrtf(5.0) * sqrtf(25.0))));
  
  // cleanup
  error = vector_free(v1);
  test_error(error);
  error = vector_free(v2);
  test_error(error);
  
  finished_tests();
}
