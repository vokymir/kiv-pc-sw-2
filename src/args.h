#ifndef ARGS_H
#define ARGS_H

#include "common.h"

/* Parse all arguments given and write the results into
  the given Config structure. Returns EXIT_SUCCESS/FAILURE.*/
int parse_args(int argc, char **argv, struct Config *config);

#endif
