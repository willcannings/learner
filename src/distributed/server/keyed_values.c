#include "distributed/protocol/protocol.h"
#include "distributed/server/server.h"
#include "core/logging.h"

// the key is: [learner_item:KEY_VALUE][name]
void *key_for_request(learner_request *req, int *key_length) {
  *key_length = (int) learner_request_name_length(req) + 1;
  char *key  = (char *) malloc(*key_length);
  memcpy(key + 1, get_learner_request_name(req), *key_length - 1);
  key[0] = (char) KEY_VALUE;
  return (void *) key;
}


learner_error handle_get_key_value(learner_request *req, learner_response *res) {
  int key_length = 0, value_length = 0;
  void *value = NULL, *key = NULL;
  
  key = key_for_request(req, &key_length);
  value = tchdbget(db, key, key_length, &value_length);
  
  if(value != NULL) {
    set_learner_response_data(res, value, value_length);
    set_learner_response_code(res, NO_ERROR);
  } else {
    warn_with_format("Database error from get_key_value: %s", tchdberrmsg(tchdbecode(db)));
    set_learner_response_code(res, UNKNOWN_KEY);
  }
  return NO_ERROR;
}


learner_error handle_set_key_value(learner_request *req, learner_response *res) {
  int key_length = 0, value_length = (int) learner_request_data_length(req);
  void *key = NULL, *value = get_learner_request_data(req);  
  key = key_for_request(req, &key_length);
  
  if(tchdbput(db, key, key_length, value, value_length)) {
    set_learner_response_code(res, NO_ERROR);
  } else {
    warn_with_format("Database error from set_key_value: %s", tchdberrmsg(tchdbecode(db)));
    set_learner_response_code(res, DATABASE_ERROR);
  }
  return NO_ERROR;
}


learner_error handle_delete_key_value(learner_request *req, learner_response *res) {
  int key_length = 0;
  void *key = key_for_request(req, &key_length);
  
  if(tchdbout(db, key, key_length)) {
    set_learner_response_code(res, NO_ERROR);
  } else {
    warn_with_format("Database error from delete_key_value: %s", tchdberrmsg(tchdbecode(db)));
    set_learner_response_code(res, DATABASE_ERROR);
  }
  return NO_ERROR;
}
