#ifndef PARSER_DATA_H
#define PARSER_DATA_H

#include <stddef.h>
#include <stdint.h>

#define MAX_IDENTIFIER_LEN 256
#define MAX_INIT_SEGMENT_STRING_LEN 256

enum Data_Type { DATA_DWORD, DATA_BYTE, DATA_ERROR };

enum Init_Segment_Type {
  INIT_SEG_VALUE,
  INIT_SEG_DUP,
  INIT_SEG_STRING,
  INIT_SEG_UNINIT
};

// one segment of data declaration
struct Init_Segment {
  enum Init_Segment_Type type;
  union {
    int32_t value; // number
    struct {
      int count;                              // how many
      int32_t value;                          // number - or ? if is_uninit
    } dup;                                    // dup
    char string[MAX_INIT_SEGMENT_STRING_LEN]; // string
  } data;
  int element_count; // length of string/count in dup/1 for number
  int is_uninit;     // is value/dup un-initialized
};

// when delaring an assembler variable
struct Data_Declaration {
  char identifier[MAX_IDENTIFIER_LEN]; // name of variable
  enum Data_Type type;

  struct Init_Segment *segments; // array of segments
  size_t segment_count;

  int total_size;      // total size of all segments->element_count
  int is_fully_uninit; // if 1 if and only if every init segment is_uninit
};

#endif
