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

#ifndef __protomsg_learner_response__
#define __protomsg_learner_response__

#define LEARNER_RESPONSE_MESSAGE_TYPE  2

#pragma pack(push)
#pragma pack(1)
typedef struct {
  char  _version;
  char  _type;
  long long code;
  long long data_length;
} learner_response_header;

typedef struct {
  learner_response_header *header;
  size_t  header_length;
  void    *data;
  size_t  data_length;
} learner_response;
#pragma pack(pop)

// getters
#define get_learner_response_code(msg) (msg->header->code)
#define get_learner_response_data(msg) (msg->data)

// setters
#define set_learner_response_code(msg, val) {msg->header->code = val;}
#define set_learner_response_data(msg, val, length) {\
  msg->data = val;\
  msg->data_length = msg->header->data_length = length;\
}

// lengths
#define learner_response_data_length(msg) (msg->header->data_length)
#define learner_response_length(msg) (msg->header_length + msg->header->data_length)
#define learner_response_variable_length(msg) (msg->header->data_length)

// memory
#define init_learner_response(msg) {\
  msg = (learner_response *) calloc(1, sizeof(learner_response));\
  msg->header = (learner_response_header *) calloc(1, sizeof(learner_response_header));\
  msg->header->_version = PROTO_MESSAGE_VERSION;\
  msg->header->_type = LEARNER_RESPONSE_MESSAGE_TYPE;\
  msg->header_length = sizeof(learner_response_header);\
}

#define free_learner_response(msg) {\
  if(msg->data){free(msg->data);}\
  free(msg->header);\
  free(msg);\
}

#define free_learner_response_structure(msg) {\
  free(msg->header);\
  free(msg);\
}

// io
#define write_learner_response(msg, sock, error) {\
  error = 0;\
  ssize_t _written = writev(sock, (const struct iovec *)msg, 2);\
  if(_written != learner_response_length(msg)){\
    error = errno;\
  }\
}

#define read_learner_response(msg, sock, error) {\
  error = 0;\
  if(!msg) {\
    msg = (learner_response *) calloc(1, sizeof(learner_response));\
    msg->header_length = sizeof(learner_response_header);\
    msg->header = (learner_response_header *)malloc(sizeof(learner_response_header));\
  }\
  safe_read(sock, msg->header, sizeof(learner_response_header), error);\
  if(!error) {\
    if(msg->header->_version != PROTO_MESSAGE_VERSION) {\
      error = INVALID_PROTOMSG_VERSION;\
    } else if(msg->header->_type != LEARNER_RESPONSE_MESSAGE_TYPE) {\
      error = INVALID_MESSAGE_TYPE;\
    } else {\
      if(msg->data)\
        free(msg->data);\
      msg->data = malloc(learner_response_variable_length(msg));\
      msg->data_length = msg->header->data_length;\
      safe_read(sock, msg->data, learner_response_variable_length(msg), error);\
    }\
  }\
}


#endif
