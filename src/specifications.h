#ifndef SPECIFICATIONS_H
#define SPECIFICATIONS_H

/*This file is a specification of KMA computer and assembler, transfered into
 * code.*/

#include "instruction.h"
#include "parser_data.h"
#include <stddef.h>

enum Register {
  A,  // Accumulator 1
  B,  // Accumulator 2
  C,  // Counter
  D,  // Destination
  S,  // Source
  SP, // Stack pointer
  IP, // Instruction pointer (isn't free to use)
};

enum Kma_Segment_Type {
  CS, // Code segment
  DS, // Data segment
  SS, // Stack segment
};

struct Kma_Segment {
  enum Kma_Segment_Type type;
  size_t size; // in bytes
};

struct Kma_Computer {
  enum Register registers[7];
  struct Kma_Segment segments[3];
};

struct Kma_Assembler {
  enum Data_Type data_types[2];
  struct Instruction_Descriptor instructions[53];
};

static const struct Kma_Computer KMA_COMPUTER = {
    .registers[0] = A,
    .registers[1] = B,
    .registers[2] = C,
    .registers[3] = D,
    .registers[4] = S,
    .registers[5] = SP,
    .segments[0] =
        {
            .type = CS,
            .size = 256 * 1024,
        },
    .segments[1] =
        {
            .type = DS,
            .size = 256 * 1024,
        },
    .segments[2] =
        {
            .type = SS,
            .size = 16 * 1024,
        },
};

static const struct Kma_Assembler KMA_ASSEMBLER = {
    .data_types[0] = DATA_BYTE,
    .data_types[1] = DATA_DWORD,
    // instructions are in instruction.c
};

#endif
