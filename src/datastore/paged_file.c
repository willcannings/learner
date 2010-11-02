#include "paged_file.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>

// ------------------------------------------
// cleanup handlers
// ------------------------------------------
#define initialise_cleanup()    int return_err = PF_NO_ERROR; int cleanup_level = 0;
#define push_cleanup_handler(i) cleanup_level++;
#define pop_cleanup_handler()   cleanup_level--;
#define cleanup(i)              if(cleanup_level >= i)
#define finish()                return return_err;


// ------------------------------------------
// error & precondition helpers
// ------------------------------------------
#define set_error(error)                return_err = error;
#define error_for(conditions, error)    if(conditions) {set_error(error); goto cleanups;}
#define test_for_uninitialised_pf()     if(!file || !file->file || !file->lock) {return PF_UNINITIALISED;}
#define test_for_missing_data()         if(!data) {return PF_MISSING_DATA;}
#define test_for_missing_path()         if(!path) {return PF_MISSING_PATH;}
#define obtain_write_lock(i)            error_for(pthread_rwlock_wrlock(file->lock), PF_PTHREAD_ERROR); push_cleanup_handler(i);
#define obtain_read_lock(i)             error_for(pthread_rwlock_rdlock(file->lock), PF_PTHREAD_ERROR); push_cleanup_handler(i);
#define cleanup_lock()                  {if(pthread_rwlock_unlock(file->lock)) set_error(PF_PTHREAD_ERROR);}


// ------------------------------------------
// open/close & flush functions
// ------------------------------------------
pf_error paged_file_open(char *path, uint64_t page_size, paged_file **file) {
  test_for_missing_path();
  int error = 0, bytes = 0;
  initialise_cleanup();
  
  // create a new paged file object
  *file = (paged_file *) calloc(sizeof(paged_file), 1);
  error_for(!*file, PF_MEMORY_ERROR);
  push_cleanup_handler(1);
  
  // initialise the read/write lock
  (*file)->lock = (pthread_rwlock_t *) malloc(sizeof(pthread_rwlock_t));
  error_for(!(*file)->lock, PF_MEMORY_ERROR);
  push_cleanup_handler(2);
  error = pthread_rwlock_init((*file)->lock, NULL);
  error_for(error, PF_PTHREAD_ERROR);
  push_cleanup_handler(3);
  
  if(access(path, F_OK) == 0) {
    // open the file and read the header
    (*file)->file = open(path, O_RDWR);
    error_for((*file)->file == -1, PF_IO_ERROR);
    push_cleanup_handler(4);
    bytes = read((*file)->file, &((*file)->header), sizeof(paged_file_header));
    error_for(bytes < 0, PF_IO_ERROR);
    error_for(bytes < sizeof(paged_file_header), PF_TRUNCATED_FILE);
    
    // check the header format
    error_for((*file)->header.magic != PAGED_FILE_MAGIC_COOKIE, PF_WRONG_FORMAT);
    error_for((*file)->header.version != PAGED_FILE_VERSION, PF_WRONG_FORMAT);
        
  } else {
    // initialise the header & object
    (*file)->header.magic     = PAGED_FILE_MAGIC_COOKIE;
    (*file)->header.version   = PAGED_FILE_VERSION;
    (*file)->header.page_size = page_size || DEFAULT_PAGE_SIZE;
    
    // create the db file and write the header
    (*file)->file = open(path, O_RDWR | O_CREAT | O_TRUNC);
    error_for((*file)->file == -1, PF_IO_ERROR);
    push_cleanup_handler(4);
    bytes = write((*file)->file, &((*file)->header), sizeof(paged_file_header));
    error_for(error < sizeof(paged_file_header), PF_IO_ERROR);
  }
  
  // cleanup handlers
  cleanups:
  cleanup(4) close((*file)->file);
  cleanup(3) pthread_rwlock_destroy((*file)->lock);
  cleanup(2) free((*file)->lock);
  cleanup(1) free(*file);
  finish();
}

pf_error paged_file_close(paged_file *file) {
  // flush remaining changes to disk. no need to do precondition
  // checks here as flush will do the same checks anyway
  int error = paged_file_flush(file);
  if(error) return error;
  
  // close file and release memory
  if(close(file->file))
    return PF_IO_ERROR;
  if(pthread_rwlock_destroy(file->lock))
    return PF_PTHREAD_ERROR;
  free(file->lock);
  free(file);
  return PF_NO_ERROR;
}

pf_error paged_file_flush(paged_file *file) {
  test_for_uninitialised_pf();
  initialise_cleanup();

  obtain_write_lock(1);
  int error = fsync(file->file);
  error_for(error, PF_IO_ERROR);
  
  cleanups:
  cleanup(1) cleanup_lock();
  finish();
}


// ------------------------------------------
// writing
// ------------------------------------------
pf_error paged_file_write_offset(paged_file *file, uint64_t index, uint64_t offset, void *data, uint64_t length) {
  test_for_uninitialised_pf();
  test_for_missing_data();
  initialise_cleanup();
  obtain_write_lock(1);
  
  // calculate offsets. If the write will extend the file, make sure length
  // is a multiple of the file page size so the total length is consistent
  void *buffer   = data;
  int64_t flen   = sizeof(paged_file_header) + (file->header.pages * file->header.page_size);
  int64_t start  = sizeof(paged_file_header) + (index * file->header.page_size) + offset;
  int64_t finish = start + length;
  int64_t pages  = ceil((float)length / file->header.page_size);
  int64_t full   = pages * file->header.page_size;
  ssize_t bytes  = 0;
  
  // TODO: check here if index...index+pages runs over any sector start pages
  // TODO: also check that these pages have been 'allocated' and are marked as not free
  
  // if we need a full buffer, zero 0..offset, copy the user data, and
  // zero the remainder of the buffer
  if((start >= flen || finish > flen) && ((length + offset) != full)) {
    buffer = malloc(full);
    error_for(!buffer, PF_MEMORY_ERROR);
    push_cleanup_handler(2);
    
    memset(buffer, 0, offset);
    memcpy(buffer + offset, data, length);
    memset(buffer + offset + length, 0, (full - length - offset) + 1);
    
    // realign start and length since we're writing a full buffer now
    start -= offset;
    length = full;
  }
  
  // perform the write
  bytes = pwrite(file->file, buffer, length, start);
  error_for(bytes != length, PF_IO_ERROR);
  
  // cleanup
  cleanups:
  cleanup(2) free(buffer);
  cleanup(1) cleanup_lock();
  finish();
}

pf_error paged_file_write_new(paged_file *file, uint64_t *index, void *data, uint64_t length) {
  test_for_uninitialised_pf();
  initialise_cleanup();
  obtain_read_lock(1);
  
  int64_t pages  = ceil((float)length / file->header.page_size);
  
  // cleanup
  cleanups:
  cleanup(1) cleanup_lock();
  finish();
}

pf_error paged_file_free(paged_file *file, uint64_t index, uint64_t count) {
  test_for_uninitialised_pf();
  initialise_cleanup();
  obtain_write_lock(1);
  
  // cleanup
  cleanups:
  cleanup(1) cleanup_lock();
  finish();
}


// ------------------------------------------
// reading
// ------------------------------------------
pf_error paged_file_read_offset(paged_file *file, uint64_t index, uint64_t offset, void **data, uint64_t length) {
  test_for_uninitialised_pf();
  initialise_cleanup();
  obtain_read_lock(1);
  
  int64_t flen   = sizeof(paged_file_header) + (file->header.pages * file->header.page_size);
  int64_t start  = sizeof(paged_file_header) + (index * file->header.page_size) + offset;
  int64_t finish = start + length;
  int64_t pages  = ceil((float)length / file->header.page_size);
  ssize_t bytes  = 0;
  
  // TODO: check here if index...index+pages runs over any sector start pages
  // TODO: also check that these pages have been 'allocated' and are marked as not free
  
  error_for(start >= flen || finish > flen, PF_INDEX_OUT_OF_RANGE);
  bytes = pread(file->file, data, length, start);
  error_for(bytes != length, PF_IO_ERROR);
  
  cleanups:
  cleanup(1) cleanup_lock();
  finish();  
}
