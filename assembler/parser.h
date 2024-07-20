#define GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "instruction_memory.h"
#include "registers.h"

typedef struct
{
    uint32_t reg;
    uint32_t imm;
} immreg_t;

int load_instructions(instruction_memory_t *i_mem, const char *trace);
int parse_R_type(char *opr, instruction_t *instr, opcode_t *opcode);
int parse_I_type(char *opr, instruction_t *instr, opcode_t *opcode);
int parse_S_type(char *opr, instruction_t *instr, opcode_t *opcode);
int parse_SB_type(char *opr, instruction_t *instr, opcode_t *opcode);
int parse_U_type(char *opr, instruction_t *instr, opcode_t *opcode);
int parse_UJ_type(char *opr, instruction_t *instr, opcode_t *opcode);
int get_reg_imm(char *tok, immreg_t *dest);
uint32_t get_register_number(char *reg);
