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
#define obtain_write_lock()             error_for(pthread_rwlock_wrlock(file->lock), PF_PTHREAD_ERROR);
#define obtain_read_lock()              error_for(pthread_rwlock_rdlock(file->lock), PF_PTHREAD_ERROR);
#define cleanup_lock()                  if(pthread_rwlock_unlock(file->lock)) set_error(PF_PTHREAD_ERROR);
#define release_lock()                  error_for(pthread_rwlock_unlock(file->lock), PF_PTHREAD_ERROR);


// ------------------------------------------
// free page helpers
// ------------------------------------------
inline int _is_free_page(paged_file *file, uint64_t index) {
  uint64_t width  = 8 * file->header.page_size; // the number of pages in a sector
  uint64_t sector = index / width;
  uint64_t offset = index - (sector * width);
  uint64_t byte   = offset / 8;
  char     bit    = offset - (byte * 8);
  return 0;
}

// ------------------------------------------
// private functions
// ------------------------------------------
pf_error _pf_write(paged_file *file, uint64_t start, void *data, uint64_t length) {
  ssize_t bytes = pwrite(file->file, data, length, start);
  if(bytes != length)
    return PF_IO_ERROR;
  else
    return PF_NO_ERROR;
}

pf_error _pf_sync_header(paged_file *file) {
  ssize_t bytes = pwrite(file->file, &(file->header), sizeof(paged_file_header), 0);
  if(bytes != sizeof(paged_file_header))
    return PF_IO_ERROR;
  else
    return PF_NO_ERROR;
}

pf_error _pf_sync_sector(paged_file *file, uint64_t sector) {
  ssize_t bytes = pwrite(file->file, file->free_pages[sector], file->header.page_size, sector * file->sector_offset);
  if(bytes != file->header.page_size)
    return PF_IO_ERROR;
  else
    return PF_NO_ERROR;
}


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
    error_for(_pf_sync_header(*file), PF_IO_ERROR);
  }
  
  // cache useful calculations rather than performing a calc per call
  (*file)->sector_length = 8 * page_size;
  (*file)->sector_offset = 1 + (8 * page_size);
  (*file)->length = sizeof(paged_file_header) + ((*file)->header.pages * page_size);
  
  // cleanup handlers for errors only
  return PF_NO_ERROR;
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
  
  obtain_write_lock();
  push_cleanup_handler(1);
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
  // preconditions
  test_for_uninitialised_pf();
  test_for_missing_data();
  initialise_cleanup();
  
  // calculate offsets
  obtain_write_lock();
  push_cleanup_handler(1);
  int64_t start  = sizeof(paged_file_header) + (index * file->header.page_size) + offset;
  int64_t pages  = ceil((float)(offset + length) / file->header.page_size);
  
  // ensure none of the pages will overwrite a sector start page
  for(int i = 0; i < pages; i++)
    error_for(((index + i) % file->sector_offset) == 0, PF_INVALID_REGION);
  
  // perform the write and return any errors
  pf_error error = _pf_write(file, start, data, length);
  set_error(error);
  
  cleanups:
  cleanup(1) cleanup_lock();
  finish();
}

pf_error paged_file_write_new(paged_file *file, uint64_t *index, void *data, uint64_t length) {
  // preconditions
  test_for_uninitialised_pf();
  test_for_missing_data();
  initialise_cleanup();
  
  // before reading/writing, make sure we have a complete write lock
  obtain_write_lock();
  push_cleanup_handler(1);
  
  // pad the buffer to be a multiple of page size if necessary
  void *buffer = data;
  if((length % file->header.page_size) != 0) {
    uint64_t data_length = length;
    length += file->header.page_size - (length % file->header.page_size);
    
    buffer = malloc(length);
    error_for(!buffer, PF_MEMORY_ERROR);
    push_cleanup_handler(2);
    
    memcpy(buffer, data, data_length);
    memset(buffer + length, 0, (length - data_length) + 1);
  }
  
  // perform the write and set our return error to be the response
  uint64_t pages = 0;
  pf_error error = _pf_write(file, (*index) * file->header.page_size, data, length);
  set_error(error);  
  
  // cleanup
  cleanups:
  cleanup(2) free(buffer);
  cleanup(1) cleanup_lock();
  finish();
}

pf_error paged_file_free(paged_file *file, uint64_t index, uint64_t count) {
  test_for_uninitialised_pf();
  initialise_cleanup();
  obtain_write_lock();
  push_cleanup_handler(1);
  
  uint64_t min_sector = -1, max_sector = -1;
  uint64_t sector = 0, offset = 0, byte = 0;
  char     bit = 0;
  
  // naive implementation
  for(int i = index; i < index + count; i++) {
    sector = index / file->sector_length;
    offset = index - (sector * file->sector_length);
    byte   = offset / 8;
    bit    = offset - (byte * 8);
    file->free_pages[sector][byte] -= (1 << bit);
    
    if(sector < min_sector || min_sector == -1)
      min_sector = sector;
    if(sector > max_sector)
      max_sector = sector;
  }
  
  // write the modified sectors to disk
  for(int i = min_sector; i <= max_sector; i++)
    error_for(_pf_sync_sector(file, i), PF_IO_ERROR);
  
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
  obtain_read_lock();
  push_cleanup_handler(1);
  
  // ensure we don't read past EOF
  length = (length) ? length : file->header.page_size;
  int64_t start  = sizeof(paged_file_header) + (index * file->header.page_size) + offset;
  error_for(start >= file->length || (start + length) > file->length, PF_INDEX_OUT_OF_RANGE);
  
  // we create the buffer for the user
  *data = malloc(length);
  error_for(!*data, PF_MEMORY_ERROR);
  push_cleanup_handler(2);
  ssize_t bytes = pread(file->file, *data, length, start);
  error_for(bytes != length, PF_IO_ERROR);
  
  cleanups:
  cleanup(2) {free(*data); *data = NULL;}
  cleanup(1) cleanup_lock();
  finish();
}
