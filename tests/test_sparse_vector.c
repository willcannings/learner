#include "tests.h"

int test_sparse_vector() {
  starting_tests();
  SparseVector *v1 = sparse_vector_new();
  SparseVector *v2 = sparse_vector_new();
  
  /* setting and getting */
  sparse_vector_set(v1, 1, 2.0);
  sparse_vector_set(v1, 0, 1.0);
  sparse_vector_set(v2, 1, 4.0);
  sparse_vector_set(v2, 0, 3.0);
  test_float(sparse_vector_get(v1, 0), 1.0);
  test_float(sparse_vector_get(v1, 1), 2.0);
  test_float(sparse_vector_get(v2, 0), 3.0);
  test_float(sparse_vector_get(v2, 1), 4.0);
  
  /* freezing */
  sparse_vector_freeze(v1);
  sparse_vector_freeze(v2);
  test(sparse_vector_frozen(v1));
  test(sparse_vector_frozen(v2));
  
  /* internal properties */  
  test(fabs(sparse_vector_magnitude(v1) - sqrtf(5.0)) <= FLT_EPSILON);
  test(fabs(sparse_vector_magnitude(v2) - sqrtf(25.0)) <= FLT_EPSILON);
  
  /* calculations with other vectors */
  test(fabs(sparse_vector_euclidean_distance(v1, v2) - sqrtf(4.0 + 4.0)) <= FLT_EPSILON);
  test(fabs(sparse_vector_dot_product(v1, v2) - (3.0 + 8.0)) <= FLT_EPSILON);
  test(fabs(sparse_vector_cosine_similarity(v1, v2) - ((3.0 + 8.0) / (sqrtf(5.0) * sqrtf(25.0)))) <= FLT_EPSILON);
  
  /* cleanup */
  sparse_vector_free(v1);
  sparse_vector_free(v2);
  
  finished_tests();
}
