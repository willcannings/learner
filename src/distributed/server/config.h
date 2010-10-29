#include "core/errors.h"

#ifndef __learner_server_config__
#define __learner_server_config__

// default config locations
#define REL_PATH        "learner.conf"
#define ETC_PATH        "/etc/learner/learner.conf"
#define USR_LOCAL_PATH  "/usr/local/learner/learner.conf"

// format
#define WHITESPACE    " \t\r\n"
#define NEW_LINE      "\r\n"

// config type
typedef struct {
  #define option(name, type, value) type name;
  #include "config_options.h"
  #undef option
} learner_config;

// public interface
learner_error find_and_read_configuration_file(char *path, learner_config *config);

#endif
