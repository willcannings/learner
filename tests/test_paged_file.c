#include "datastore/paged_file.h"
#include "tests.h"

int test_paged_file() {
  starting_tests();
  pf_error error;
  paged_file *file = NULL;
  
  error = paged_file_open("test_file.db", 1024, &file);
  test(error == PF_NO_ERROR);
  
  paged_file_set_attribute(file, 0, 10);
  test(paged_file_get_attribute(file, 0) == 10);
  
  error = paged_file_flush(file);
  test(error == PF_NO_ERROR);
  
  error = paged_file_close(file);
  test(error == PF_NO_ERROR);
  
/*
  // writing
  pf_error paged_file_write_offset(paged_file *file, uint64_t index, uint64_t offset, void *data, uint64_t length);
  pf_error paged_file_write_new(paged_file *file, uint64_t *index, void *data, uint64_t length);
  pf_error paged_file_free(paged_file *file, uint64_t index, uint64_t count);
  #define  paged_file_write(file, index, data, length)  paged_file_write_offset(file, index, 0, data, length)

  // reading
  pf_error paged_file_read_offset(paged_file *file, uint64_t index, uint64_t offset, void **data, uint64_t length);
  #define  paged_file_read(file, index, data, length) paged_file_read_offset(file, index, 0, data, length)
*/
  
  remove("test_file.db");
  finished_tests();
}
