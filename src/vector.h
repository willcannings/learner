#ifndef __learner_vector__
#define __learner_vector__

typedef struct {
	float *values;
	int   length;
	int   klass;
	
  char  _frozen;
  float _magnitude;
} Vector;

Vector *vector_new(int length);
void    vector_free(Vector *vector);
void    vector_freeze(Vector *vector);
char    vector_frozen(Vector *vector);
float   vector_dot_product(Vector *v1, Vector *v2);
float   vector_magnitude(Vector *vector);
float   vector_cosine_similarity(Vector *v1, Vector *v2);
float   vector_euclidean_distance(Vector *v1, Vector *v2);

#endif
