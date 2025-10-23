#ifndef PARSER_DATA_H
#define PARSER_DATA_H

#include <stddef.h>
#include <stdint.h>

#define MAX_IDENTIFIER_LEN 256

enum Data_Type { DATA_DWORD, DATA_BYTE, DATA_ERROR };

enum Init_Segment_Type {
  INIT_SEG_VALUE,
  INIT_SEG_DUP,
  INIT_SEG_STRING,
  INIT_SEG_UNINIT
};

struct Init_Segment {
  enum Init_Segment_Type type;
  union {
    int32_t value;
    struct {
      int count;
      int32_t value;
    } dup;
    char *string;
  } data;
  int element_count;
  int is_uninit;
};

struct Data_Declaration {
  char identifier[MAX_IDENTIFIER_LEN];
  enum Data_Type type;

  struct Init_Segment *segments;
  size_t segment_count;

  int total_size;
  int is_fully_uninit;
};

// NOT YET SURE
struct Data_Declaration *datad_create(void);

int datad_init(struct Data_Declaration *dd);

void datad_deinit(struct Data_Declaration *dd);

void datad_free(struct Data_Declaration **dd);

#endif
