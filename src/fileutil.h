#ifndef FILE_UTIL_H
#define FILE_UTIL_H

// Return 1 if path exists, 0 otherwise.
int fu_path_exists(const char *path);

// Return 1 if path exists and is file.
int fu_is_file(const char *path);

// Return 1 if path exists and is directory.
int fu_is_dir(const char *path);

// Return 1 if current process can read the path.
int fu_can_read(const char *path);

// Return 1 if current process can write into the path.
int fu_can_write(const char *path);

// Return absolute path, caller must free.
char *fu_cannonical_path(const char *path);

#endif
