#include "core/logging.h"
#include "distributed/protocol.h"
static int *sockets = NULL;
static int sockets_count = 0;

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


learner_error server_for(Reference ref, int *server) {
  if(ref.matrix < 0) return INDEX_OUT_OF_RANGE;  
  if(ref.type == ROW_NAME || ref.type == COLUMN_NAME) {
    *server = sockets[ref.matrix % sockets_count];
  } else {
    // the operation may be acting on a column, not a row, but it doesn't matter
    // all we're using rows/cols for is to distribute requests
    if(ref.row < 0) return INDEX_OUT_OF_RANGE;
    *server = sockets[(ref.matrix + ref.row) % sockets_count];
  }
  return NO_ERROR;
}


learner_error index_reference_from_name(matrix_ref matrix, name_ref name, int name_length, learner_operation operation, index_ref *index) {
  request   *req = NULL;
  response  *res = NULL;
  int       error = 0, server = 0;
  
  // create the request
  init_request(req);
  set_request_matrix(req, matrix);
  set_request_operation(req, operation);
  set_request_reference(req, name, name_length);
  
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

learner_error row_reference_from_name(index_t matrix, name_t name, int name_length, index_t *row) {
  return index_reference_from_name(matrix, name, name_length, GET_ROW_FROM_NAME, (index_ref *)row);
}

learner_error column_reference_from_name(matrix_ref matrix, name_ref name, int name_length, column_ref *column) {
  return index_reference_from_name(matrix, name, name_length, GET_COLUMN_FROM_NAME, (index_ref *)column);
}
