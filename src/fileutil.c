#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <io.h>        // for _access WIN
#define stat _stat     // rename to UNIX
#define access _access // rename to UNIX
#define R_OK 4         // read permission WIN
#define W_OK 2         // write permission WIN
#else
#include <unistd.h> // for access UNIX
#endif

#include "fileutil.h"
#include "memory.h"

int fu_path_exists(const char *path) {
  struct stat st = {0};
  return (stat(path, &st) == 0);
}

int fu_is_file(const char *path) {
  struct stat st = {0};
  if (stat(path, &st) != 0) {
    return 0;
  }
#if defined(_WIN32)
  return (st.st_mode & S_IFREG) != 0;
#else
  return S_ISREG(st.st_mode);
#endif
}

int fu_is_dir(const char *path) {
  struct stat st = {0};
  if (stat(path, &st) != 0) {
    return 0;
  }
#if defined(_WIN32)
  return (st.st_mode & S_IFDIR) != 0;
#else
  return S_ISDIR(st.st_mode);
#endif
}

int fu_can_read(const char *path) { return access(path, R_OK) == 0; }

int fu_can_write(const char *path) {
  if (fu_is_dir(path)) {
    return fu_can_write_dir(path);
  } else if (fu_is_file(path)) {
    return fu_can_write_file(path);
  } else if (!fu_path_exists(
                 path)) { // file doesn't exist, but maybe can be created
    return fu_can_write_parent_dir(path);
  } else { // path is neither dir nor file (maybe link or smth)
    return 0;
  }
}

int fu_can_write_file(const char *path) { return access(path, W_OK) == 0; }

int fu_can_write_dir(const char *path) { return access(path, W_OK) == 0; }

int fu_can_write_parent_dir(const char *path) {
  char *dup, *slash = NULL;
  int res = 0;
  if (!path) {
    return 0;
  }
  dup = jtrdup(path);
  if (!dup) {
    return 0;
  }

  slash = strrchr(dup, '/');
#if defined(_WIN32)
  char *backslash = strrchr(dup, '\\'); // alternative on WIN
  if (backslash &&
      (!slash ||
       backslash >
           slash)) { // if path is using backslash and slash either doesn't even
                     // is in path, or backslash is later in the path
    slash = backslash;
  }
#endif

  if (slash) {
    *slash = '\0';
    if (dup[0] == '\0') { // root directory
      res = fu_can_write_dir("/");
    } else {
      res = fu_can_write_dir(dup);
    }
  } else { // current directory
    res = fu_can_write_dir(".");
  }

  jree(dup);
  return res;
}
