#include "tests.h"

void test_vector(void) {
  starting_tests();
  Vector *v1 = vector_new(2);
  Vector *v2 = vector_new(2);
  
  v1->values[0] = 1.0;
  v1->values[1] = 2.0;
  v2->values[0] = 3.0;
  v2->values[1] = 4.0;
  
  /* freezing */
  vector_freeze(v1);
  vector_freeze(v2);
  test(vector_frozen(v1));
  test(vector_frozen(v2));
  
  /* internal properties */  
  test((vector_magnitude(v1) - sqrtf(5.0)) <= FLT_EPSILON);
  test((vector_magnitude(v2) - sqrtf(25.0)) <= FLT_EPSILON);
  
  /* calculations with other vectors */
  test((vector_euclidean_distance(v1, v2) - sqrtf(4.0 + 4.0)) <= FLT_EPSILON);
  test((vector_dot_product(v1, v2) - (3.0 + 8.0)) <= FLT_EPSILON);
  test((vector_cosine_similarity(v1, v2) - ((3.0 + 8.0) / (sqrtf(5.0) * sqrtf(25.0)))) <= FLT_EPSILON);
  
  /* cleanup */
  vector_free(v1);
  vector_free(v2);
  
  finished_tests();
}
