#include <tcutil.h>
#include <tchdb.h>
#include "distributed/protocol/protocol.h"

#ifndef __learner_server__
#define __learner_server__

learner_error delete_keyed_value(TCHDB *db, learner_request *req, learner_response *res);
learner_error get_keyed_value(TCHDB *db, learner_request *req, learner_response *res);
learner_error set_keyed_value(TCHDB *db, learner_request *req, learner_response *res);

#endif
