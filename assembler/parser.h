#define GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "instruction_memory.h"
#include "registers.h"

void load_instructions(instruction_memory_t *i_mem, const char *trace);
void parse_R_type(char *opr, instruction_t *instr);
void parse_I_type(char *opr, instruction_t *instr);
void parse_SB_type(char *opr, instruction_t *instr);
unsigned int get_register_number(char *reg);
