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

#ifndef __protomsg_request__
#define __protomsg_request__

#define REQUEST_MESSAGE_TYPE  1

typedef struct {
  char  _version;
  char  _type;
  long long operation;
  int reference_length;
  int data_length;
} request_header;

typedef struct {
  request_header *header;
  size_t  header_length;
  void    *reference;
  size_t  reference_length;
  void    *data;
  size_t  data_length;
} request;


// getters
#define get_request_operation(msg) (msg->header->operation)
#define get_request_reference(msg) (msg->reference)
#define get_request_data(msg) (msg->data)

// setters
#define set_request_operation(msg, val) {msg->header->operation = val;}
#define set_request_reference(msg, val, length) {\
  msg->reference = val;\
  msg->reference_length = msg->header->reference_length = length;\
}
#define set_request_data(msg, val, length) {\
  msg->data = val;\
  msg->data_length = msg->header->data_length = length;\
}

// lengths
#define request_reference_length(msg) (msg->header->reference_length)
#define request_data_length(msg) (msg->header->data_length)
#define request_length(msg) (msg->header_length + msg->header->reference_length + msg->header->data_length)
#define request_variable_length(msg) (msg->header->reference_length + msg->header->data_length)

// memory
#define init_request(msg) {\
  msg = (request *) calloc(1, sizeof(request));\
  msg->header = (request_header *) calloc(1, sizeof(request_header));\
  msg->header->_version = PROTO_MESSAGE_VERSION;\
  msg->header->_type = REQUEST_MESSAGE_TYPE;\
  msg->header_length = sizeof(request_header);\
}

#define free_request(msg) {\
  if(msg->reference){free(msg->reference);}\
  if(msg->data){free(msg->data);}\
  free(msg->header);\
  free(msg);\
}

#define free_request_structure(msg) {\
  free(msg->header);\
  free(msg);\
}

// io
#define write_request(msg, sock, error) {\
  error = 0;\
  ssize_t _written = writev(sock, (const struct iovec *)msg, 3);\
  if(_written != request_length(msg)){\
    error = errno;\
  }\
}

#define read_request(msg, sock, error) {\
  error = 0;\
  if(!msg) {\
    msg = (request *) calloc(1, sizeof(request));\
    msg->header_length = sizeof(request_header);\
    msg->header = (request_header *)malloc(sizeof(request_header));\
  }\
  safe_read(sock, msg->header, sizeof(request_header), error);\
  if(!error) {\
    if(msg->header->_version != PROTO_MESSAGE_VERSION) {\
      error = INVALID_PROTOMSG_VERSION;\
    } else if(msg->header->_type != REQUEST_MESSAGE_TYPE) {\
      error = INVALID_MESSAGE_TYPE;\
    } else {\
      if(msg->reference)\
        free(msg->reference);\
      msg->reference = malloc(request_variable_length(msg));\
      msg->reference_length = msg->header->reference_length;\
      msg->data_length = msg->header->data_length;\
      msg->data = msg->reference + msg->header->reference_length;\
      safe_read(sock, msg->reference, request_variable_length(msg), error);\
    }\
  }\
}


#endif
