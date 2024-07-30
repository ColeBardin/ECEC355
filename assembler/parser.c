#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t(* parse_funcs[])(opcode_t *opcode, int tokc, char *tokv[]) =
{
    &parse_NULL_type,
    &parse_R_type, 
    &parse_I_type, 
    &parse_S_type, 
    &parse_SB_type, 
    &parse_U_type, 
    &parse_UJ_type, 
};

ins_list_t *load_instructions(const char *trace)
{
    printf("Loading trace file: %s\n\n", trace);
    FILE *fd = fopen(trace, "r");
    if (fd == NULL) 
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE); 
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *tokv[MAXTOKS];
    int tokc;
    uint32_t bin;
    ins_list_t *l;

    l = ins_list_init();
    if(l == NULL)
    {
        fputs("ERROR: Failed to initialize instruction list", stderr);
        exit(EXIT_FAILURE);
    }

    uint64_t pc = 0;

    while ((read = getline(&line, &len, fd)) != EOF) {
        tokc = tokenize(line, tokv, MAXTOKS, ", \n");
        if(tokc == 0) 
        {
            fprintf(stderr, "Failed to tokenize line\n");
            exit(EXIT_FAILURE);
        }

        bin = handle_instruction(tokc, tokv);
        ins_list_add(l, pc, bin);

        pc += 4;
    }

    fclose(fd);
    return l;
}

uint32_t handle_instruction(int tokc, char *tokv[])
{
    int opi;
    for(opi = 0; opi < NOPS; opi++)
    {
        opcode_t *op = &opcode_map[opi];
        if(!strcmp(tokv[0], op->name))
        {
            if(op->type == NULL_TYPE)
            {
                fprintf(stderr, "Library is broken. My bad\n");
                exit(EXIT_FAILURE);
            }
            return parse_funcs[op->type](op, tokc, tokv); 
        }
    }
    fprintf(stderr, "Failed to parse instruction: %s\n", tokv[0]);
    exit(EXIT_FAILURE);
}

// Parse and assemble R-type instruction 
uint32_t parse_R_type(opcode_t *opcode, int tokc, char *tokv[])
{
    immreg_t immreg;
    int ret = 0;
    uint32_t bin = 0;
    uint32_t opc = opcode->code;
    uint32_t rd = 0;
    uint32_t func3 = opcode->func3;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    uint32_t func7 = opcode->func7;

    if(tokc != 4 || tokv == NULL || opcode == NULL)
    {
        fputs("ERROR: cmon man", stderr);
        exit(EXIT_FAILURE);
    }

    get_reg_imm(tokv[1], &immreg);
    rd = immreg.reg;

    get_reg_imm(tokv[2], &immreg);
    rs1 = immreg.reg;

    get_reg_imm(tokv[3], &immreg);
    rs2 = immreg.reg;

    // Print the tokens 
    puts("R-Type");
    printf("opcode: 0x%x\n", opc);
    printf("rd: %d\n", rd);
    printf("func3: 0x%x\n", func3);
    printf("rs1: %d\n", rs1);
    printf("rs2: %d\n", rs2);
    printf("func7: 0x%x\n", func7);
    puts("");

    // Contruct instruction
    bin |= opc;
    bin |= (rd << 7);
    bin |= (func3 << 12);
    bin |= (rs1 << 15);
    bin |= (rs2 << 20);
    bin |= (func7 << 25);

    return bin;
}

uint32_t parse_I_type(opcode_t *opcode, int tokc, char *tokv[])
{
    immreg_t immreg;
    int ret = 0;
    int ttype;
    uint32_t bin;
    uint32_t opc = opcode->code;
    uint32_t rd = 0;
    uint32_t func3 = opcode->func3;
    uint32_t rs1 = 0;
    uint32_t imm12 = 0;
    uint8_t imm_found = 0;

    if(tokc < 3 || tokc > 4 || tokv == NULL || opcode == NULL)
    {
        fputs("ERROR: cmon man", stderr);
        exit(EXIT_FAILURE);
    }

    ttype = get_reg_imm(tokv[1], &immreg);
    rd = immreg.reg;

    ttype = get_reg_imm(tokv[2], &immreg);
    rs1 = immreg.reg;
    if(ttype > 1)
    {
        imm_found = 1;
        imm12 = immreg.imm;
    }

    if(!imm_found && tokc == 4)
    {
        ttype = get_reg_imm(tokv[3], &immreg);
        imm12 = immreg.imm;
    }
    else if((!imm_found && tokc == 3) || (imm_found && tokc == 4))
    {
        fputs("ERROR: incorrect argument format for I-type", stderr);
        exit(EXIT_FAILURE);
    }

    puts("I-Type");
    printf("opcode: 0x%x\n", opc);
    printf("rd: %u\n", rd);
    printf("funt3: 0x%x\n", func3);
    printf("rs1: %u\n", rs1);
    printf("imm12: %u\n", imm12);
    puts("");

    bin = 0;
    bin |= opc;
    bin |= (rd << 7);
    bin |= (func3 << 12);
    bin |= (rs1 << 15);
    bin |= (imm12 << 20);

    return bin;
}

uint32_t parse_S_type(opcode_t *opcode, int tokc, char *tokv[])
{
    fputs("WARNING: S-Type not implemented yet, filling 0", stderr);
    return 0;
}

uint32_t parse_SB_type(opcode_t *opcode, int tokc, char *tokv[])
{
    immreg_t immreg;
    int ttype;
    uint32_t bin;
    uint32_t opc = opcode->code;
    uint32_t func3 = opcode->func3;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    uint32_t imm12 = 0;
    uint8_t imm_found = 0;    

    if(tokc != 4 || tokv == NULL || opcode == NULL)
    {
        fputs("ERROR: cmon man", stderr);
        exit(EXIT_FAILURE);
    }

    ttype = get_reg_imm(tokv[1], &immreg);
    rs1 = immreg.reg;

    ttype = get_reg_imm(tokv[2], &immreg);
    rs2 = immreg.reg;

    ttype = get_reg_imm(tokv[3], &immreg);
    imm12 = immreg.imm;

    puts("SB-Type");
    printf("opcode: 0x%x\n", opc);
    printf("funt3: 0x%x\n", func3);
    printf("rs1: %u\n", rs1);
    printf("rs2: %u\n", rs2);
    printf("imm12: 0x%x\n", imm12);
    puts("");

    uint32_t imm_4_1 = (imm12 >> 1) & 0xF;
    uint32_t imm_10_5 = (imm12 >> 5) & 0x3F;
    uint32_t imm_11 = (imm12 >> 11) & 0x1;
    uint32_t imm_12 = (imm12 >> 12) & 0x1;

    bin = 0;
    bin |= opc;
    bin |= (imm_11 << 7);
    bin |= (imm_4_1 << 8);
    bin |= (func3 << 12);
    bin |= (rs1 << 15);
    bin |= (rs2 << 20);
    bin |= (imm_10_5 << 25);
    bin |= (imm_12 << 31);

    return bin;
}

uint32_t parse_U_type(opcode_t *opcode, int tokc, char *tokv[])
{
    fputs("WARNING: U-Type not implemented yet, filling 0", stderr);
    return 0;
}

uint32_t parse_UJ_type(opcode_t *opcode, int tokc, char *tokv[])
{
    fputs("WARNING: UJ-Type not implemented yet, filling 0", stderr);
    return 0;
}

uint32_t parse_NULL_type(opcode_t *opcode, int tokc, char *tokv[])
{
    fputs("ERROR: Tried to parse NULL type. Something is very wrong", stderr);
    exit(EXIT_FAILURE);
}

int get_reg_imm(char *tok, immreg_t *dest)
{
    char *p;
    char *r;
    if(tok[0] == 'x' || tok[0] == 'f') // pure reg
    {
        dest->reg = get_register_number(tok);
        return 1;
    }
    else if((p = strchr(tok, '(')) != NULL) // combo
    {
        *p = '\0';
        r = p + 1;
        r[strlen(r) - 2] = '\0';
        dest->imm = atoi(tok);
        dest->reg = get_register_number(r);
        r[strlen(r)] = ')';
        *p = '(';
        return 2;
    }
    else // pure imm
    {
        dest->imm = atoi(tok);
        return 3;
    }
}

uint32_t get_register_number(char *reg)
{
    int i;
    for(i = 0; i < NUM_OF_REGS; i++) 
    {
        if(strcmp(REGISTER_NAME[i], reg) == 0) break;
    }
    if(i == NUM_OF_REGS)
    {
        fprintf(stderr, "Failed to resolve register: %s\n", reg);
        exit(EXIT_FAILURE);
    }
    return i;
}

int tokenize(char *s, char *tokv[], int maxtokv, char *delim)
{
    char *p;
    int i = 0;

    tokv[i] = strtok(s, delim); 
    while(tokv[i++] != NULL)
    {
        if(i >= maxtokv - 1){
            tokv[i] = NULL;
        }
        else
        {
            tokv[i] = strtok(NULL, delim);
        }
    }
    return i - 1;
}

