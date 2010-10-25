#include "distributed/protocol/protocol.h"
#include "core/logging.h"

// ------------------------------------------
// miscellaneous
// ------------------------------------------
void log_request(learner_request *request) {
  if(current_learner_logging_level > DEBUG)
    return;
  
  learner_request_header *req = request->header;
  char *operation = learner_operation_names[req->operation];
  char *item = learner_item_names[req->item];
  char *attribute = learner_attribute_names[req->attribute];
  
  // delete operations don't operate on an attribute
  if(req->operation != DELETE) {
    debug_with_format("Sending request %s %s %s", operation, item, attribute);
  } else {
    debug_with_format("Sending request %s %s", operation, item);
  }
  
  // row, column and cell operations
  if(req->item != MATRIX && req->item != KEY_VALUE) {
    if(req->attribute == INDEX) {
      debug_with_format("matrix: %i, name length: %i", req->matrix, req->name_length);

    } else if(req->operation == SET && req->attribute == NAME) {
      if(req->item == ROW) {
        debug_with_format("matrix: %i, row: %i, name length: %i", req->matrix, req->row, req->name_length);
      } else {
        debug_with_format("matrix: %i, column: %i, name length: %i", req->matrix, req->column, req->name_length);
      }
      
    } else {
      if(req->item == ROW) {
        debug_with_format("matrix: %i, row: %i", req->matrix, req->row);
      } else if(req->item == COLUMN) {
        debug_with_format("matrix: %i, column: %i", req->matrix, req->column);
      } else {
        debug_with_format("matrix: %i, row: %i, column: %i", req->matrix, req->row, req->column);
      }
    }
    
  // key/value operations
  } else if(req->item == KEY_VALUE) {
    if(req->operation == SET) {
      debug_with_format("name length: %i, value length: %i", req->name_length, req->data_length);
    } else {
      debug_with_format("name length: %i", req->name_length);
    }
  
  // matrix operations
  } else if(req->item == MATRIX) {
    if(req->operation == DELETE || (req->operation == GET && req->attribute == NAME)) {
      debug_with_format("matrix: %i", req->matrix);
    } else if(req->attribute == INDEX) {
      debug_with_format("name length: %i", req->name_length);
    } else if(req->operation == SET && req->attribute == NAME) {
      debug_with_format("matrix: %i, name length: %i", req->matrix, req->name_length);
    }
  }
}

// ------------------------------------------
// server distribution
// ------------------------------------------
static int *sockets = NULL;
static int sockets_count = 0;

// add a server to the distribution pool
learner_error add_server(char *host, int weight) {
  int new_socket = 0, error = 0;
  connect_to_server(host, 3579, new_socket, error);
  if(error) {
    warn_with_format("Error connecting to server '%s': %i", host, error);
    return COMMUNICATION_ERROR;
  }
  
  sockets = (int *) realloc(sockets, (sockets_count + weight) * sizeof(int));
  for(int i = 0; i < weight; i++)
    sockets[sockets_count++] = new_socket;
  return NO_ERROR;
}

// bernstein hash function
unsigned long long hash_name(void *name, long long length) {
  unsigned long long hash = 5381;
  for(long long i = 0; i < length; i++)
    hash = ((hash << 5) + hash + ((char *)name)[i]);
  return hash;
}

// deterministically select a server from the pool to field a request
learner_error server_for(learner_request *request, int *server) {
  learner_request_header *req = request->header;
  if(req->matrix < 0) return INDEX_OUT_OF_RANGE;
  
  // matrix operations, and name operations on rows/cols
  if(req->attribute == INDEX || req->attribute == NAME || req->item == MATRIX) {
    *server = sockets[req->matrix % sockets_count];

  // all other row & cell operations
  } else if(req->item == ROW || req->item == CELL) {
    if(req->row < 0) return INDEX_OUT_OF_RANGE;
    *server = sockets[(req->matrix + req->row) % sockets_count];

  // all other column operations
  } else if(req->item == COLUMN) {
    if(req->column < 0) return INDEX_OUT_OF_RANGE;
    *server = sockets[(req->matrix + req->column) % sockets_count];

  // key value operations
  } else if(req->item == KEY_VALUE) {
    if(req->name_length <= 0) return NAME_MISSING;
    *server = sockets[hash_name(request->name, req->name_length) % sockets_count];
  } else {
    return UNKNOWN_OPERATION;
  }
  return NO_ERROR;
}



// ------------------------------------------
// generic internal API helpers
// ------------------------------------------
learner_error _send_request(learner_request *req, learner_response **response) {
  int error = 0, server = 0;
  learner_response *res = NULL;
  
  // find the appropriate server
  if(current_learner_logging_level == DEBUG)
    log_request(req);
  error = server_for(req, &server);
  debug_with_format("to server: %i", server);
  if(error) {
    warn_with_error("Failure in server_for", error);
    return error;
  }
  
  // write request
  debug("Sending request");
  write_learner_request(req, server, error);
  if(error) {
    warn_with_format("Error sending request to server: %i", error);
    return COMMUNICATION_ERROR;
  }
  
  // wait for a response
  debug("Reading response");
  read_learner_response(res, server, error);
  if(error) {
    warn_with_format("Error receiving response from server: %i", error);
    return COMMUNICATION_ERROR;
  }
  
  debug("Request complete");
  free_learner_request_structure(req);
  *response = res;
  return NO_ERROR;
}

learner_error _null_response_operation(learner_request *req) {
  learner_response *res = NULL;
  int error = 0, code = 0;
  
  error = _send_request(req, &res);
  if(error) {
    free_learner_response(res);
    return error;
  }
  
  // successful request, return the response code
  code = get_learner_response_code(res);
  free_learner_response(res);
  return code;
}

learner_error _response_operation(learner_request *req, void **data, long long *length) {
  learner_response *res = NULL;
  int error = 0, code = 0;
  
  error = _send_request(req, &res);
  if(error) {
    free_learner_response(res);
    return error;
  }
  
  // set references to the data section
  *data = get_learner_response_data(res);
  *length = learner_response_data_length(res);
  
  // successful request, return the response code
  code = get_learner_response_code(res);
  free_learner_response_structure(res);
  return code;
}


// ------------------------------------------
// key/value operations
// ------------------------------------------
learner_error set_key_value(void *key, long long key_length, void *value, long long value_length) {
  learner_request *req = NULL;
  init_learner_request(req);
  set_learner_request_operation(req, SET);
  set_learner_request_item(req, KEY_VALUE);
  set_learner_request_name(req, key, key_length);
  set_learner_request_data(req, value, value_length);
  return _null_response_operation(req);
}

learner_error get_key_value(void *key, long long key_length, void **value, long long *value_length) {
  learner_request *req = NULL;
  init_learner_request(req);
  set_learner_request_operation(req, GET);
  set_learner_request_item(req, KEY_VALUE);
  set_learner_request_name(req, key, key_length);
  return _response_operation(req, value, value_length);
}

learner_error delete_key_value(void *key, long long key_length) {
  learner_request *req = NULL;
  init_learner_request(req);
  set_learner_request_operation(req, DELETE);
  set_learner_request_item(req, KEY_VALUE);
  set_learner_request_name(req, key, key_length);
  return _null_response_operation(req);
}
