#include "distributed/protocol.h"
#include "core/logging.h"

// ------------------------------------------
// miscellaneous
// ------------------------------------------
void log_request(learner_request *request, learner_logging_level level) {
  if(current_learner_logging_level > level)
    return;
  
  learner_request_header *req = request->header;
  char *operation = learner_operation_names(req->operation);
  char *item = learner_item_names(req->item);
  char *attribute = learner_attribute_names(req->attribute);
  learner_log_with_format(level, "Sending request %s %s %s");
  
  if(req->item == KEY_VALUE) {
    learner_log_with_format("name length: %i", req->name_length);
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
    warn_with_string_and_int("Error connecting to server '%s': %i", host, error);
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
    hash = ((hash << 5) + hash) + name[i];
  return hash;
}

// deterministicly select a server from the pool to field a request
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
learner_error _send_request(learner_request *req, learner_response *res) {
  int error = 0, server = 0;
  res = NULL;
  
  // find the appropriate server
  log_request(req, DEBUG);
  error = server_for(req, &server);
  debug_with_format("to server: %i", server);
  if(error) {
    warn_with_error("Failure in server_for", error);
    return error;
  }
}

learner_error _delete_operation(learner_request *req) {
  learner_response *res = NULL;
  int error = 0, server = 0;
  
  // find the appropriate server
  error = server_for(req, &server);
  if(error) {
    warn_with_error("Failure in server_for", error);
    return error;
  }

  // write the request
  write_request(req, server, error);
  if(error) {
    warn_with_int("Error sending request to server in _delete_operation: %i", error);
    return COMMUNICATION_ERROR;
  }
  
  // wait for a response
  read_response(res, server, error);
  if(error) {
    warn_with_int("Error receiving response from server in _delete_operation: %i", error);
    free_learner_response(res);
    return COMMUNICATION_ERROR;
  }
  
  // successful request, return the response code
  error = get_learner_response_code(res);
  free_learner_response(res);
  return error;
}


// ------------------------------------------
// key/value operations
// ------------------------------------------
learner_error set_key_value(void *key, long long key_length, void *value, long long value_length) {
  
}

learner_error get_key_value(void *key, long long key_length, void **value, long long *value_length) {
  
}

learner_error delete_key_value(void *key, long long key_length) {
  
}


learner_error index_by_name(Reference *ref, index_t *index) {
  request   *req = NULL;
  response  *res = NULL;
  int       error = 0, server = 0;
  
  // create the request
  init_request(req);
  set_request_operation(req, GET);
  set_request_reference(req, ref, sizeof_reference(ref));
  set_request_data(req, ref->reference->name->buffer, ref->reference->name->length)
  
  // send to the appropriate server
  error = server_for(req, &server);
  if(error) {
    warn_with_error("Failure in server_for called from index_reference_from_name", error);
    return error;
  }
  write_request(req, server, error);
  if(error) {
    warn_with_int("Error sending request to server in index_reference_from_name: %i", error);
    return COMMUNICATION_ERROR;
  }
  
  // wait for a response
  read_response(res, server, error);
  if(error) {
    warn_with_int("Error receiving response from server in index_reference_from_name: %i", error);
    return COMMUNICATION_ERROR;
  }
  if(get_response_code(res) == NO_ERROR) {
    *index = *((index_ref *)get_response_data(res));
    return NO_ERROR;
  } else {
    note_with_error("Error received from server in response to index_reference_from_name", error);
    return error;
  }
}

learner_error row_index_by_name(index_t matrix, name_t name, int name_length, index_t *row) {
  Reference ref;
  int error = 0;
  
  ref.matrix = matrix;
  ref.type = ROW_INDEX_BY_NAME;
  ref.reference.name.length = name_length;
  ref.reference.name.buffer = name;  
  return index_by_name(&ref, row);
}

learner_error column_index_by_name(index_t matrix, name_t name, int name_length, index_t *column) {
  Reference ref;
  int error = 0;
  
  ref.matrix = matrix;
  ref.type = COLUMN_INDEX_BY_NAME;
  ref.reference.name.length = name_length;
  ref.reference.name.buffer = name;  
  return index_by_name(&ref, column);
}
