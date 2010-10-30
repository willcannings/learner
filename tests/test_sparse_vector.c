#include "tests.h"

#define test_get_value(v, index, val) {\
  error = sparse_vector_get(v, index, &value);\
  test_error(error);\
  test_float(value, val);\
}

#define test_set_value(v, index, val) {\
  error = sparse_vector_set(v, index, val);\
  test_error(error);\
}


int test_sparse_vector() {
  starting_tests();
  learner_error error;
  SparseVector *v1, *v2;
  
  // create a generic matrix
  Matrix *m;
  error = matrix_new(&m);
  
  // creating
  error = sparse_vector_new(&v1, m);
  test_error(error);
  error = sparse_vector_new(&v2, m);
  test_error(error);
  
  // setting
  test_set_value(v1, 1, 2.0);
  test_set_value(v1, 0, 1.0);
  test_set_value(v2, 0, 3.0);
  test_set_value(v2, 3, 4.0);
  test_set_value(v2, 2, 5.0);
  
  // getting
  float value;
  test_get_value(v1, 0, 1.0);
  test_get_value(v1, 1, 2.0);
  test_get_value(v2, 0, 3.0);
  test_get_value(v2, 2, 5.0);
  test_get_value(v2, 3, 4.0);
  
  // freezing
  int frozen;
  error = sparse_vector_freeze(v1);
  test_error(error);
  error = sparse_vector_freeze(v2);
  test_error(error);
  error = sparse_vector_frozen(v1, &frozen);
  test_error(error);
  test(frozen);
  error = sparse_vector_frozen(v2, &frozen);
  test_error(error);
  test(frozen);
  
  // magnitude
  error = sparse_vector_magnitude(v1, &value);
  test_error(error);
  test_float(value, sqrtf(5.0));
  error = sparse_vector_magnitude(v2, &value);
  test_error(error);
  test_float(value, sqrtf(50.0));
  
  // euclidean distance
  error = sparse_vector_euclidean_distance(v1, v2, &value);
  test_error(error);
  test_float(value, sqrtf(4.0 + 4.0 + 25.0 + 16.0));
  
  // dot product
  error = sparse_vector_dot_product(v1, v2, &value);
  test_error(error);
  test_float(value, 3.0);
  
  // cosine similarity
  error = sparse_vector_cosine_similarity(v1, v2, &value);
  test_error(error);
  test_float(value, ((3.0) / (sqrtf(5.0) * sqrtf(50.0))));
  
  // setting existing values
  test_set_value(v1, 1, 12.0);
  test_set_value(v1, 0, 11.0);
  test_set_value(v2, 0, 13.0);
  test_set_value(v2, 3, 14.0);
  test_set_value(v2, 2, 15.0);
  
  // getting changed values
  test_get_value(v1, 0, 11.0);
  test_get_value(v1, 1, 12.0);
  test_get_value(v2, 0, 13.0);
  test_get_value(v2, 2, 15.0);
  test_get_value(v2, 3, 14.0);
  
  // cleanup
  error = sparse_vector_free(v1);
  test_error(error);
  error = sparse_vector_free(v2);
  test_error(error);
  finished_tests();
}
