#ifndef __PARSER_H__
#define __PARSER_H__

#define GNU_SOURCE

#include <stdint.h>

#include "instruction.h"
#include "registers.h"

#define MAXTOKS 32

typedef struct
{
    uint32_t reg;
    uint32_t imm;
} immreg_t;

ins_list_t *load_instructions(const char *trace);
uint32_t handle_instruction(int tokc, char *tokv[]);
uint32_t parse_R_type(opcode_t *opcode, int tokc, char *tokv[]);
uint32_t parse_I_type(opcode_t *opcode, int tokc, char *tokv[]);
uint32_t parse_S_type(opcode_t *opcode, int tokc, char *tokv[]);
uint32_t parse_SB_type(opcode_t *opcode, int tokc, char *tokv[]);
uint32_t parse_U_type(opcode_t *opcode, int tokc, char *tokv[]);
uint32_t parse_UJ_type(opcode_t *opcode, int tokc, char *tokv[]);
int get_reg_imm(char *tok, immreg_t *dest);
uint32_t get_register_number(char *reg);
int tokenize(char *s, char *toks[], int maxtoks, char *delim);

#endif // __PARSER_H__

