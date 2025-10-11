#ifndef ARGS_H
#define ARGS_H

#include "common.h"

/* Parse all arguments given and write the results into
  the given Config structure. Returns EXIT_SUCCESS/FAILURE.*/
int args_parse(int argc, char **argv, struct Config *config);

/* Free target in config.
 * */
void args_free_config(struct Config *config);

#endif
