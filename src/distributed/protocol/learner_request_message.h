#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <unistd.h>
#include "protomsg.h"

#ifndef __protomsg_learner_request__
#define __protomsg_learner_request__

#define LEARNER_REQUEST_MESSAGE_TYPE  1

#pragma pack(push)
#pragma pack(1)
typedef struct {
  char  _version;
  char  _type;
  long long operation;
  long long item;
  long long attribute;
  long long matrix;
  long long row;
  long long column;
  long long name_length;
  long long data_length;
} learner_request_header;

typedef struct {
  learner_request_header *header;
  size_t  header_length;
  void    *name;
  size_t  name_length;
  void    *data;
  size_t  data_length;
} learner_request;
#pragma pack(pop)

// getters
#define get_learner_request_operation(msg) (msg->header->operation)
#define get_learner_request_item(msg) (msg->header->item)
#define get_learner_request_attribute(msg) (msg->header->attribute)
#define get_learner_request_matrix(msg) (msg->header->matrix)
#define get_learner_request_row(msg) (msg->header->row)
#define get_learner_request_column(msg) (msg->header->column)
#define get_learner_request_name(msg) (msg->name)
#define get_learner_request_data(msg) (msg->data)

// setters
#define set_learner_request_operation(msg, val) {msg->header->operation = val;}
#define set_learner_request_item(msg, val) {msg->header->item = val;}
#define set_learner_request_attribute(msg, val) {msg->header->attribute = val;}
#define set_learner_request_matrix(msg, val) {msg->header->matrix = val;}
#define set_learner_request_row(msg, val) {msg->header->row = val;}
#define set_learner_request_column(msg, val) {msg->header->column = val;}
#define set_learner_request_name(msg, val, length) {\
  msg->name = val;\
  msg->name_length = msg->header->name_length = length;\
}
#define set_learner_request_data(msg, val, length) {\
  msg->data = val;\
  msg->data_length = msg->header->data_length = length;\
}

// lengths
#define learner_request_name_length(msg) (msg->header->name_length)
#define learner_request_data_length(msg) (msg->header->data_length)
#define learner_request_length(msg) (msg->header_length + msg->header->name_length + msg->header->data_length)
#define learner_request_variable_length(msg) (msg->header->name_length + msg->header->data_length)

// memory
#define init_learner_request(msg) {\
  msg = (learner_request *) calloc(1, sizeof(learner_request));\
  msg->header = (learner_request_header *) calloc(1, sizeof(learner_request_header));\
  msg->header->_version = PROTO_MESSAGE_VERSION;\
  msg->header->_type = LEARNER_REQUEST_MESSAGE_TYPE;\
  msg->header_length = sizeof(learner_request_header);\
}

#define free_learner_request(msg) {\
  if(msg->name){free(msg->name);}\
  if(msg->data){free(msg->data);}\
  free(msg->header);\
  free(msg);\
}

#define free_learner_request_structure(msg) {\
  free(msg->header);\
  free(msg);\
}

// io
#define write_learner_request(msg, sock, error) {\
  error = 0;\
  ssize_t _written = writev(sock, (const struct iovec *)msg, 3);\
  if(_written != learner_request_length(msg)){\
    error = errno;\
  }\
}

#define read_learner_request(msg, sock, error) {\
  error = 0;\
  if(!msg) {\
    msg = (learner_request *) calloc(1, sizeof(learner_request));\
    msg->header_length = sizeof(learner_request_header);\
    msg->header = (learner_request_header *)malloc(sizeof(learner_request_header));\
  }\
  safe_read(sock, msg->header, sizeof(learner_request_header), error);\
  if(!error) {\
    if(msg->header->_version != PROTO_MESSAGE_VERSION) {\
      error = INVALID_PROTOMSG_VERSION;\
    } else if(msg->header->_type != LEARNER_REQUEST_MESSAGE_TYPE) {\
      error = INVALID_MESSAGE_TYPE;\
    } else {\
      if(msg->name)\
        free(msg->name);\
      msg->name = malloc(learner_request_variable_length(msg));\
      msg->name_length = msg->header->name_length;\
      msg->data_length = msg->header->data_length;\
      msg->data = msg->name + msg->header->name_length;\
      safe_read(sock, msg->name, learner_request_variable_length(msg), error);\
    }\
  }\
}


#endif
