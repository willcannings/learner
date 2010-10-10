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

#ifndef __protomsg_response__
#define __protomsg_response__

#define RESPONSE_MESSAGE_TYPE  2

typedef struct {
  char  _version;
  char  _type;
  long long code;
  int data_length;
} response_header;

typedef struct {
  response_header *header;
  size_t  header_length;
  void    *data;
  size_t  data_length;
} response;


// getters
#define get_response_code(msg) (msg->header->code)
#define get_response_data(msg) (msg->data)

// setters
#define set_response_code(msg, val) {msg->header->code = val;}
#define set_response_data(msg, val, length) {\
  msg->data = val;\
  msg->data_length = msg->header->data_length = length;\
}

// lengths
#define response_data_length(msg) (msg->header->data_length)
#define response_length(msg) (msg->header_length + msg->header->data_length)
#define response_variable_length(msg) (msg->header->data_length)

// memory
#define init_response(msg) {\
  msg = (response *) calloc(1, sizeof(response));\
  msg->header = (response_header *) calloc(1, sizeof(response_header));\
  msg->header->_version = PROTO_MESSAGE_VERSION;\
  msg->header->_type = RESPONSE_MESSAGE_TYPE;\
  msg->header_length = sizeof(response_header);\
}

#define free_response(msg) {\
  if(msg->data){free(msg->data);}\
  free(msg->header);\
  free(msg);\
}

#define free_response_structure(msg) {\
  free(msg->header);\
  free(msg);\
}

// io
#define write_response(msg, sock, error) {\
  error = 0;\
  ssize_t _written = writev(sock, (const struct iovec *)msg, 2);\
  if(_written != response_length(msg)){\
    error = errno;\
  }\
}

#define read_response(msg, sock, error) {\
  error = 0;\
  if(!msg) {\
    msg = (response *) calloc(1, sizeof(response));\
    msg->header_length = sizeof(response_header);\
    msg->header = (response_header *)malloc(sizeof(response_header));\
  }\
  safe_read(sock, msg->header, sizeof(response_header), error);\
  if(!error) {\
    if(msg->header->_version != PROTO_MESSAGE_VERSION) {\
      error = INVALID_PROTOMSG_VERSION;\
    } else if(msg->header->_type != RESPONSE_MESSAGE_TYPE) {\
      error = INVALID_MESSAGE_TYPE;\
    } else {\
      if(msg->data)\
        free(msg->data);\
      msg->data = malloc(response_variable_length(msg));\
      msg->data_length = msg->header->data_length;\
      safe_read(sock, msg->data, response_variable_length(msg), error);\
    }\
  }\
}


#endif
