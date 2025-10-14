#include <sys/stat.h>
#if defined(_WIN32)
#include <io.h>        // for _access WIN
#define stat _stat     // rename to UNIX
#define access _access // rename to UNIX
#define R_OK 4         // read permission WIN
#else
#include <unistd.h> // for access UNIX
#endif

#include "fileutil.h"

int fu_path_exists(const char *path) {
  struct stat st;
  return (stat(path, &st) == 0);
}

int fu_is_file(const char *path) {
  struct stat st;
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
  struct stat st;
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
