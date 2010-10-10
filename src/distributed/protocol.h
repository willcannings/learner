#include <sys/types.h>
#include "distributed/protomsg.h"
#include "distributed/request_message.h"
#include "distributed/response_message.h"
#include "core/errors.h"

#ifndef __learner_protocol__
#define __learner_protocol__

// operations
typedef enum {
  GET,
  SET,
  DELETE
} learner_operation;

// types
typedef u_int32_t index_t;
typedef void *    name_t;

// refernec types
typedef enum {
  ROW,
  COLUMN,
  CELL,
  ROW_NAME,
  COLUMN_NAME
} learner_reference_t;

// references
#pragma pack(push)
#pragma pack(1)
typedef struct {
  index_t               matrix;
  learner_reference_t   type : 8;
  union {
    index_t row;
    index_t column;
    struct {
      index_t row;
      index_t column;
    } cell;
    struct {
      index_t length;
      name_t  buffer;
    } name;
  } reference;
} Reference;
#pragma pack(pop)

#define sizeof_reference(ref) {\
  sizeof(index_t) + sizeof(learner_reference_t) +\
  ((ref.type == ROW_NAME || ref.type == COLUMN_NAME) ? 0 : sizeof(index_t) +\
  (ref.type == CELL ? sizeof(index_t) : 0))\
}

// api
learner_error add_server(char *host, int weight);
learner_error server_for(Reference ref, int *server);
learner_error row_reference_from_name(index_t matrix, name_t name, int name_length, index_t *row);
learner_error column_reference_from_name(index_t matrix, name_t name, int name_length, index_t *column);

#endif
