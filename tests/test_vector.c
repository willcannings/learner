#include "tests.h"

int test_vector() {
  starting_tests();
  Vector *v1 = vector_new(2);
  Vector *v2 = vector_new(2);
  
  /* setting and getting */
  vector_set(v1, 1, 2.0);
  vector_set(v1, 0, 1.0);
  vector_set(v2, 1, 4.0);
  vector_set(v2, 0, 3.0);
  test_float(vector_get(v1, 0), 1.0);
  test_float(vector_get(v1, 1), 2.0);
  test_float(vector_get(v2, 0), 3.0);
  test_float(vector_get(v2, 1), 4.0);
  
  /* freezing */
  vector_freeze(v1);
  vector_freeze(v2);
  test(vector_frozen(v1));
  test(vector_frozen(v2));
  
  /* internal properties */  
  test(fabs(vector_magnitude(v1) - sqrtf(5.0)) <= FLT_EPSILON);
  test(fabs(vector_magnitude(v2) - sqrtf(25.0)) <= FLT_EPSILON);
  
  /* calculations with other vectors */
  test(fabs(vector_euclidean_distance(v1, v2) - sqrtf(4.0 + 4.0)) <= FLT_EPSILON);
  test(fabs(vector_dot_product(v1, v2) - (3.0 + 8.0)) <= FLT_EPSILON);
  test(fabs(vector_cosine_similarity(v1, v2) - ((3.0 + 8.0) / (sqrtf(5.0) * sqrtf(25.0)))) <= FLT_EPSILON);
  
  /* cleanup */
  vector_free(v1);
  vector_free(v2);
  
  finished_tests();
}
