#include "distributed/server/server.h"
#include "core/logging.h"

learner_error get_keyed_value(TCHDB *db, learner_request *req, learner_response *res) {
  const void *name = (const void *) get_learner_request_name(req);
  int name_length = (int) learner_request_name_length(req);
  
  int value_length = 0;
  void *value = NULL;
  value = tchdbget(db, name, name_length, &value_length);
  
  if(value != NULL) {
    set_learner_response_data(res, value, value_length);
    set_learner_response_code(res, NO_ERROR);
  } else {
    warn_with_format("Database error from get_keyed_value: %s", tchdberrmsg(tchdbecode(db)));
    set_learner_response_code(res, UNKNOWN_KEY);
  }
  
  free_learner_request(req);
  return NO_ERROR;
}


learner_error set_keyed_value(TCHDB *db, learner_request *req, learner_response *res) {
  const void *name = (const void *) get_learner_request_name(req);
  int name_length = (int) learner_request_name_length(req);
  const void *value = (const void *) get_learner_request_data(req);
  int value_length = (int) learner_request_data_length(req);
  
  if(tchdbput(db, name, name_length, value, value_length)) {
    set_learner_response_code(res, NO_ERROR);
  } else {
    warn_with_format("Database error from set_keyed_value: %s", tchdberrmsg(tchdbecode(db)));
    set_learner_response_code(res, DATABASE_ERROR);
  }
  
  free_learner_request(req);
  return NO_ERROR;
}


learner_error delete_keyed_value(TCHDB *db, learner_request *req, learner_response *res) {
  const void *name = (const void *) get_learner_request_name(req);
  int name_length = (int) learner_request_name_length(req);
  
  if(tchdbout(db, name, name_length)) {
    set_learner_response_code(res, NO_ERROR);
  } else {
    warn_with_format("Database error from delete_keyed_value: %s", tchdberrmsg(tchdbecode(db)));
    set_learner_response_code(res, DATABASE_ERROR);
  }
  
  free_learner_request(req);
  return NO_ERROR;
}
