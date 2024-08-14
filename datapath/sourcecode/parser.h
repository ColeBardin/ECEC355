#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruction_memory.h"
#include "registers.h"

void load_instructions(instruction_memory_t *i_mem, const char *trace);
void parse_R_type(char *opr, instruction_t *instr);
int reg_index(char *reg);
