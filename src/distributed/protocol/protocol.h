#include <sys/types.h>
#include "distributed/response_message.h"
#include "distributed/request_message.h"
#include "distributed/protomsg.h"
#include "structures/sparse_vector"
#include "core/errors.h"

#ifndef __learner_protocol__
#define __learner_protocol__

// ------------------------------------------
// protocol type definitions
// ------------------------------------------
// operations
typedef enum {
  GET,
  SET,
  DELETE
} learner_operation;
extern char *learner_operation_names[];

// operations happen to items
typedef enum {
  KEY_VALUE,
  MATRIX,
  ROW,
  COLUMN,
  CELL
} learner_item;
extern char *learner_item_names[];

// operations occur on attributes of items
typedef enum {
  VALUE,
  NAME,
  INDEX
} learner_attribute;
extern char *learner_attribute_names[];

// ------------------------------------------
// API
// ------------------------------------------
// miscellaneous
learner_error add_server(char *host, int weight);
char          *request_to_string(learner_request *req);

// key/value operations
learner_error set_key_value(void *key, long long key_length, void *value, long long value_length);
learner_error get_key_value(void *key, long long key_length, void **value, long long *value_length);
learner_error delete_key_value(void *key, long long key_length);

// matrix operations
learner_error get_matrix_index(void *name, long long length, long long *index);
learner_error get_matrix_name(long long matrix, void **name, long long *length);
learner_error set_matrix_name(long long matrix, void *name, long long length);
learner_error delete_matrix(long long matrix);

// get operations within a matrix
learner_error get_row_value(long long matrix, long long row, sparse_vector **vector);
learner_error get_column_value(long long matrix, long long column, sparse_vector **vector);
learner_error get_cell_value(long long matrix, long long row, long long column, float *value);
learner_error get_row_name(long long matrix, long long row, void **name, long long *length);
learner_error get_column_name(long long matrix, long long column, void **name, long long *length);
learner_error get_row_index(long long matrix, void *name, long long length, long long *index);
learner_error get_column_index(long long matrix, void *name, long long length, long long *index);

// set operations within a matrix
learner_error set_row_value(long long matrix, long long row, sparse_vector *vector);
learner_error set_column_value(long long matrix, long long column, sparse_vector *vector);
learner_error set_cell_value(long long matrix, long long row, long long column, float value);
learner_error set_row_name(long long matrix, long long row, void *name, long long length);
learner_error set_column_name(long long matrix, long long column, void *name, long long length);

// delete operations within a matrix
learner_error delete_row_value(long long matrix, long long row);
learner_error delete_column_value(long long matrix, long long column);
learner_error delete_cell_value(long long matrix, long long row, long long column);
learner_error delete_row_name(long long matrix, long long row);
learner_error delete_column_name(long long matrix, long long column);
#endif
