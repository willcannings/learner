#ifndef __learner_matrix__
#define __learner_matrix__

typedef struct {
  long long index;
  long long rows;
  long long columns;
  int row_buffer_delta;
  int column_buffer_delta;
} learner_matrix;

#endif
