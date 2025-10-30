#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdio.h>

#define FU_GETLINE_INIT_LEN 128

// Return 1 if path exists, 0 otherwise.
int fu_path_exists(const char *path);

// Return 1 if path exists and is file.
int fu_is_file(const char *path);

// Return 1 if path exists and is directory.
int fu_is_dir(const char *path);

// Return 1 if current process can read the path.
int fu_can_read(const char *path);

// Return 1 if current process can write into the path.
// Meaning if path is file, write into file OR in the same directory.
// If path is dir, if can create files in it.
int fu_can_write(const char *path);

// Return 1 if can write INTO file (path must be file).
int fu_can_write_file(const char *path);

// Return 1 if can create files in directory (path must be dir).
int fu_can_write_dir(const char *path);

// Return 1 if can create files in parent directory, path don't have to be
// valid.
int fu_can_write_parent_dir(const char *path);

// Read a line from stream. Reallocate *lineptr as needed (like POSIX getline).
// On success, return number of read bytes (not including terminator '\0').
// *n is updated to the current buffer size.
// On EOF/no bytes read/error return -1.
//
// If *lineptr is NULL or *n is 0, allocate a buffer(caller must free).
int fu_getline(char **lineptr, size_t *n, FILE *stream);

#endif
