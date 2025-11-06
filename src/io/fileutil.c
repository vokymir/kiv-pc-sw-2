#include <stddef.h>
#include <stdio.h>
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

#include "common.h"
#include "fileutil.h"
#include "memory.h"

int fu_path_exists(const char *path) {
  struct stat st = {0};
  if (!path) {
    PRINT_ERR("Tried checking if path exist but gave NULL pointer.");
    return 0;
  }

  return (stat(path, &st) == 0);
}

int fu_is_file(const char *path) {
  struct stat st = {0};
  if (!path) {
    PRINT_ERR("Tried checking if path is file but gave NULL pointer.");
    return 0;
  }

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
  if (!path) {
    PRINT_ERR("Tried checking if path is directory but gave NULL pointer.");
    return 0;
  }

  if (stat(path, &st) != 0) {
    return 0;
  }
#if defined(_WIN32)
  return (st.st_mode & S_IFDIR) != 0;
#else
  return S_ISDIR(st.st_mode);
#endif
}

int fu_can_read(const char *path) {
  if (!path) {
    PRINT_ERR("Tried checking if path can be read but gave NULL pointer.");
    return 0;
  }

  return access(path, R_OK) == 0;
}

int fu_can_write(const char *path) {
  if (!path) {
    PRINT_ERR(
        "Tried checking if path can be written into but gave NULL pointer.");
    return 0;
  }

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

int fu_can_write_file(const char *path) {
  if (!path) {
    PRINT_ERR("Tried checking if file on path can be written into but gave "
              "NULL pointer.");
    return 0;
  }

  if (!fu_is_file(path)) {
    return 0;
  }
  return access(path, W_OK) == 0;
}

int fu_can_write_dir(const char *path) {
  if (!path) {
    PRINT_ERR(
        "Tried checking if directory on path can be written into but gave "
        "NULL pointer.");
    return 0;
  }

  if (!fu_is_dir(path)) {
    return 0;
  }
  return access(path, W_OK) == 0;
}

int fu_can_write_parent_dir(const char *path) {
  char *dup, *slash = NULL;
  int res = 0;
  if (!path) {
    PRINT_ERR("Tried checking if can be written into the parent directory of "
              "given path but gave NULL pointer.");
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

int fu_open(const char *path, FILE **f, const char *modifiers) {
  if (!path || !f) {
    PRINT_ERR("Tried opening file on path but gave NULL pointer to path or "
              "file descriptor.");
    return 0;
  }

  const char *mode = modifiers ? modifiers : "r";

  // If reading, ensure file exists and can be read
  if (mode[0] == 'r') {
    if (!fu_is_file(path) || !fu_can_read(path)) {
      return 0;
    }
  }
  // If writing, ensure we can write in parent dir
  else if (mode[0] == 'w' || mode[0] == 'a') {
    if (!fu_can_write_parent_dir(path)) {
      return 0;
    }
  }

  *f = fopen(path, mode);
  if (!*f) {
    return 0;
  }

  return 1;
}

long fu_getline(char **lineptr, size_t *n, FILE *stream) {
  size_t pos = 0, new_n = 0;
  int ch = 0;
  char *tmp = NULL;
  if (!lineptr || !n || !stream) { // WARN: if changing this check, also change
                                   // FIST_NULL one line bellow
    PRINT_ERR("Tried getting line from file stream but gave NULL pointer as "
              "argument at %zu. position.",
              FIRST_NULL(lineptr, n, stream));
    return -1;
  }

  // Allocate if *lineptr doesn't exist
  if (*lineptr == NULL || *n == 0) {
    *n = FU_GETLINE_INIT_LEN;
    *lineptr = jalloc(*n);
    if (!*lineptr) {
      PRINT_ERR(
          "Couldn't allocate a string of size '%zu' to save read line into.",
          *n);
      return -1;
    }
  }

  // while have something to read
  while ((ch = fgetc(stream)) != EOF) {
    // extend if needed
    if (pos + 1 >= *n) {
      new_n = (*n) * FU_GETLINE_CAP_MULT;
      tmp = jealloc(*lineptr, new_n);
      if (!tmp) {
        PRINT_ERR("Couldn't reallocate a string of size '%zu' to save read "
                  "line into.",
                  new_n);
        return -1;
      }
      *lineptr = tmp;
      *n = new_n;
      tmp = NULL;
    }

    // set actual character & optionally exit
    (*lineptr)[pos] = (char)ch;
    pos++;

    if (ch == '\n') {
      break;
    }
  }

  // if found EOF & nothing is read
  if (pos == 0 && ch == EOF) {
    return -1;
  }

  // NULL terminate & return
  (*lineptr)[pos] = '\0';
  return (long)pos;
}

int fu_write_bytes(FILE *f, const void *buf, size_t count) {
  if (fwrite(buf, 1, count, f) != count) {
    PRINT_ERR("Didn't write '%zu' bytes into file.", count);
    return 0;
  }
  return 1;
}
