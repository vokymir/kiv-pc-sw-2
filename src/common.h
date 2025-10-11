#ifndef COMMON_H
#define COMMON_H

// Holds information needed throughout the whole program.
struct Config {
  char flag_verbose;
  char flag_instruction;
  char *source;
  char *target;
};

#endif
