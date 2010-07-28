#ifndef __learner_vector__
#define __learner_vector__

typedef struct {
  int   length;
  int   klass;
  char  _frozen;
  float _magnitude;
  float *values;
} Vector;

// core functions
Vector *vector_new(int length);
void    vector_free(Vector *vector);
void    vector_freeze(Vector *vector);
char    vector_frozen(Vector *vector);
void    vector_unfreeze(Vector *vector);

// getter & setter to match the sparse matrix functions
void    vector_set(Vector *vector, int index, float value);
float   vector_get(Vector *vector, int index);

// calculations
float   vector_dot_product(Vector *v1, Vector *v2);
float   vector_magnitude(Vector *vector);
float   vector_cosine_similarity(Vector *v1, Vector *v2);
float   vector_euclidean_distance(Vector *v1, Vector *v2);

#endif
