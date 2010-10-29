#include "distributed/server/config.h"
#include "core/logging.h"
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// default config
learner_config learner_default_config = {
  #define option(name, type, value) value,
  #include "config_options.h"
  #undef option
};

// ------------------------------------------
// helpers
// ------------------------------------------
#define skip_whitespace(data) {\
  while (*data <= 32 && *data != 0)\
    data++;\
}

#define read_name(name, data) {\
  name = data;\
  while (((*data >= 'A' && *data <= 'Z') || (*data >= 'a' && *data <= 'z') || *data == '_') && *data != 0)\
    data++;\
  *data = 0;\
  data++;\
}

#define skip_whitespace_and_colon(data) {\
  while ((*data <= 32 || *data == ':') && *data != 0)\
    data++;\
}

#define read_value(value, data) {\
  value = data;\
  if (*data == '"') {\
    while (*data != '"' && *data != 0)\
      data++;\
  } else {\
    while (*data > 32 && *data != 0)\
      data++;\
  }\
  *data = 0;\
  data++;\
}

int file_exists(char *path) {
  struct stat result;
  return (stat(path, &result) == 0);
}

int file_length(FILE *file) {
  int length = 0;
  if (fseek(file, 0, SEEK_END) == -1) {
    warn_with_errno("Error seeking end of configuration file");
  }
  if ((length = ftell(file)) == -1) {
    warn_with_errno("Error reading end position of configuration file");
  }
  rewind(file);
  return length;
}


// ------------------------------------------
// parser
// ------------------------------------------
learner_error read_configuration_file(char *path, learner_config *config) {
  debug_with_format("Loading configuration from %s", path);
  FILE *file = fopen(path, "r");
  if (!file) {
    warn_with_errno("Error reading configuration file");
    return FILE_IO_ERROR;
  }
  
  int length = file_length(file);
  if (length <= 0) {
    warn("Unable to read the configuration file length, or the configuration file is empty");
    return FILE_IO_ERROR;
  }
  
  char *data = (char *) malloc(length + 1);
  int bytes = fread(data, 1, length, file);
  if (bytes < length) {
    if (feof(file)) {
      warn("Unexpected end of file while reading the configuration file");
      return FILE_IO_ERROR;
    } else if (ferror(file)) {
      warn("Unexpected error while reading the configuration file");
      return FILE_IO_ERROR;
    }
  }
  
  char *name = NULL, *value = NULL;
  int line = 0;
  skip_whitespace(data);
  
  while (*data) {
    line++;
    read_name(name, data);
    skip_whitespace_and_colon(data);
    read_value(value, data);
    skip_whitespace(data);
    
    // sanity check
    if (strlen(name) == 0) {
      warn_with_format("Unable to read option name in configuration file, line %i", line);
      return PARSE_ERROR;
    }
    if (strlen(value) == 0) {
      warn_with_format("Unable to read option value in configuration file, line %i", line);
      return PARSE_ERROR;
    }
        
    // FIXME: for now, all config options can only be ints...
    // allow each of the options to handle the line
    #define option(opt_name, opt_type, opt_value)  if (strcmp(#opt_name, name) == 0) config->opt_name = atoi(value);
    #include "config_options.h"
    #undef option
  }
  
  return NO_ERROR;
}


// ------------------------------------------
// public interface
// ------------------------------------------
learner_error find_and_read_configuration_file(char *path, learner_config *config) {
  // always load defaults
  memcpy(config, &learner_default_config, sizeof(learner_config));
  
  // try loading one of the known paths
  if (path && file_exists(path)) {
    return read_configuration_file(path, config);
  } else if (file_exists(ETC_PATH)) {
    return read_configuration_file(ETC_PATH, config);
  } else if (file_exists(USR_LOCAL_PATH)) {
    return read_configuration_file(USR_LOCAL_PATH, config);
  } else if (file_exists(REL_PATH)) {
    return read_configuration_file(REL_PATH, config);
  }
  return FILE_NOT_FOUND;
}
