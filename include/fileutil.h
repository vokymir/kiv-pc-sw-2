#ifndef FILE_UTIL_H
#define FILE_UTIL_H
/* Module for working with file, File Utilities. */

#include <stdio.h>

// Return 1 if path exists, 0 otherwise.
int fu_path_exists(const char *path);

// Return 1 if path exists and is file.
int fu_is_file(const char *path);

// Return 1 if path exists and is directory.
int fu_is_dir(const char *path);

// Return 1 if current process can read the path.
int fu_can_read(const char *path);

// Return 1 if current process can write into the path.
// Meaning if path is a file, write into the file if exists OR in the same
// directory. If path is dir, if can create files in it.
int fu_can_write(const char *path);

// Return 1 if can write INTO file (path must be file).
int fu_can_write_file(const char *path);

// Return 1 if can create files in directory (path must be dir).
int fu_can_write_dir(const char *path);

// Return 1 if can create files in parent directory. Doesn't check the path
// validity.
int fu_can_write_parent_dir(const char *path);

// Open file from path.
// Return 1 on success, 0 on failure.
int fu_open(const char *path, FILE **f, const char *modifiers);

// Read a line from stream. Reallocate *lineptr as needed (like POSIX getline).
// On success, return number of read bytes (not including terminator '\0').
// *n is updated to the current buffer size.
// On EOF/no bytes read/error return -1.
//
// If *lineptr is NULL or *n is 0, allocate a buffer(caller must free).
long fu_getline(char **lineptr, size_t *n, FILE *stream);

// Write <count> bytes from <buf> to file.
// Returns 1 on success, 0 on failure.
int fu_write_bytes(FILE *f, const void *buf, size_t count);

#endif
