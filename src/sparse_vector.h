#ifndef __learner_sparse_vector__
#define __learner_sparse_vector__

typedef struct {
  int index;
  float value;
} sparse_vector_value;

typedef struct {
	sparse_vector_value *values;
	int   count;
	int   klass;
	
  char  _frozen;
  float _magnitude;
  int   _min_index;
  int   _max_index;
} SparseVector;

SparseVector *sparse_vector_new(void);
void    sparse_vector_free(SparseVector *vector);
void    sparse_vector_freeze(SparseVector *vector);
char    sparse_vector_frozen(SparseVector *vector);
float   sparse_vector_dot_product(SparseVector *v1, SparseVector *v2);
float   sparse_vector_magnitude(SparseVector *vector);
float   sparse_vector_cosine_similarity(SparseVector *v1, SparseVector *v2);
float   sparse_vector_euclidean_distance(SparseVector *v1, SparseVector *v2);

#endif
