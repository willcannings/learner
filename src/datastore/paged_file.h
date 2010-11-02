#include <pthread.h>
#include <stdint.h>

#ifndef __learner_paged_file__
#define __learner_paged_file__

// ------------------------------------------
// defaults
// ------------------------------------------
#define PAGED_FILE_MAGIC_COOKIE   'Pfil'
#define PAGED_FILE_VERSION        1
#define DEFAULT_PAGE_SIZE         1024


// ------------------------------------------
// errors
// ------------------------------------------
typedef enum {
  PF_NO_ERROR = 0,
  PF_MISSING,
  PF_UNINITIALISED,
  PF_INDEX_OUT_OF_RANGE,
  PF_LENGTH_INVALID,
  PF_MISSING_PATH,
  PF_MISSING_DATA,
  PF_TRUNCATED_FILE,
  PF_WRONG_FORMAT,
  PF_IO_ERROR,
  PF_MEMORY_ERROR,
  PF_PTHREAD_ERROR,
  PF_INVALID_REGION
} pf_error;


// ------------------------------------------
// types
// ------------------------------------------
// 1kb header that appears at the beginning of every paged file
#pragma pack(push)
#pragma pack(1)
  typedef struct {
    uint32_t  magic;              // magic cookie to type this file as a paged file
    uint32_t  version;            // paged file version number
    uint32_t  page_size;          // size of pages in bytes
    uint32_t  free_pages;         // number of free pages
    uint64_t  sectors;            // number of sectors in the file (a sector is (8 * page_size) pages long)
    uint64_t  pages;              // number of written pages in the file (includes sector start pages)
    uint64_t  attributes[12];     // other datastore specific header attributes
  } paged_file_header;
#pragma pack(pop)

// reference to a paged file
typedef struct {
  pthread_rwlock_t  *lock;        // pthread read/write locks are used for concurrency control
  paged_file_header header;       // store of the complete header of a paged file
  void              **free_pages; // sector start pages; a bit array of pages indicating if they are free
  int               file;         // file descriptor
  
  // rather than recalculating these each call, we cache
  // a number of useful numbers in the object
  uint64_t          sector_length;
  uint64_t          sector_offset;
  uint64_t          length;
} paged_file;


// ------------------------------------------
// api
// ------------------------------------------
// open, close & flush
pf_error paged_file_open(char *path, uint64_t page_size, paged_file **file);
pf_error paged_file_close(paged_file *file);
pf_error paged_file_flush(paged_file *file);

// writing
pf_error paged_file_write_offset(paged_file *file, uint64_t index, uint64_t offset, void *data, uint64_t length);
pf_error paged_file_write_new(paged_file *file, uint64_t *index, void *data, uint64_t length);
pf_error paged_file_free(paged_file *file, uint64_t index, uint64_t count);
#define  paged_file_write(file, index, data, length)  paged_file_write_offset(file, index, 0, data, length)
#define  paged_file_set_attribute(paged_file, index, value) (paged_file->header.attributes[index] = value)

// reading
pf_error paged_file_read_offset(paged_file *file, uint64_t index, uint64_t offset, void **data, uint64_t length);
#define  paged_file_read(file, index, data, length) paged_file_read_offset(file, index, 0, data, length)
#define  paged_file_get_attribute(paged_file, index) (paged_file->header.attributes[index])

#endif
