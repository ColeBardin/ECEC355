#include "parser.h"


// FIXME: add support to identify and parse I-type and SB-type instructions 
int load_instructions(instruction_memory_t *i_mem, const char *trace)
{
    int ret = 0;
    printf("Loading trace file: %s\n\n", trace);
    FILE *fd = fopen(trace, "r");
    if (fd == NULL) 
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE); 
    }
    // Iterate over all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    addr_t PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;

    while ((read = getline(&line, &len, fd)) != -1) {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation or opcode from the assembly instruction 
        char *raw_instr = strtok(line, " ");
        int opi;
        for(opi = 0; opi < NOPS; opi++)
        {
            opcode_t *op = &opcode_map[opi];
            if(!strcmp(raw_instr, op->name))
            {
                switch(op->type)
                {
                    case R_TYPE:
                        ret |= parse_R_type(raw_instr, &(i_mem->instructions[IMEM_index]), op);
                        break;
                    case I_TYPE:
                        ret |= parse_I_type(raw_instr, &(i_mem->instructions[IMEM_index]), op);
                        break;
                    case S_TYPE:
                        ret |= parse_S_type(raw_instr, &(i_mem->instructions[IMEM_index]), op);
                        break;
                    case SB_TYPE:
                        ret |= parse_SB_type(raw_instr, &(i_mem->instructions[IMEM_index]), op);
                        break;
                    case U_TYPE:
                        ret |= parse_U_type(raw_instr, &(i_mem->instructions[IMEM_index]), op);
                        break;
                    case UJ_TYPE:
                        ret |= parse_UJ_type(raw_instr, &(i_mem->instructions[IMEM_index]), op);
                        break;
                    case NULL_TYPE:
                    default:
                        fprintf(stderr, "Library is broken. My bad\n");
                        exit(EXIT_FAILURE);
                }
                i_mem->last = &(i_mem->instructions[IMEM_index]);
                break;
            }
        }
        if(opi == NOPS)
        {
            fprintf(stderr, "Failed to parse line: %s\n", line);
            exit(EXIT_FAILURE);
        }

        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
    return ret;
}

// Parse and assemble R-type instruction 
int parse_R_type(char *opr, instruction_t *instr, opcode_t *opcode)
{
    instr->instruction = 0;
    immreg_t immreg;
    int ret = 0;
    char *reg;
    uint32_t opc = opcode->code;
    uint32_t rd = 0;
    uint32_t func3 = opcode->func3;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    uint32_t func7 = opcode->func7;

    reg = strtok(NULL, ", ");
    get_reg_imm(reg, &immreg);
    rd = immreg.reg;

    reg = strtok(NULL, ", ");
    get_reg_imm(reg, &immreg);
    rs1 = immreg.reg;

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    get_reg_imm(reg, &immreg);
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
    instr->instruction |= opc;
    instr->instruction |= (rd << 7);
    instr->instruction |= (func3 << 12);
    instr->instruction |= (rs1 << 15);
    instr->instruction |= (rs2 << 20);
    instr->instruction |= (func7 << 25);

    return ret;
}


// FIXME: parse and assemble I-type instruction 
int parse_I_type(char *opr, instruction_t *instr, opcode_t *opcode)
{
    instr->instruction = 0;
    immreg_t immreg;
    int ret = 0;
    char *reg;
    int ttype;
    uint32_t opc = opcode->code;
    uint32_t rd = 0;
    uint32_t func3 = opcode->func3;
    uint32_t rs1 = 0;
    uint32_t imm12 = 0;
    uint8_t imm_found = 0;

    reg = strtok(NULL, ", ");
    if(reg != NULL)
    {
        ttype = get_reg_imm(reg, &immreg);
        rd = immreg.reg;
    }

    reg = strtok(NULL, ", ");
    if(reg != NULL)
    {
        ttype = get_reg_imm(reg, &immreg);
        rs1 = immreg.reg;
        if(ttype > 1)
        {
            imm_found = 1;
            imm12 = immreg.imm;
        }
    }

    if(!imm_found)
    {
        reg = strtok(NULL, ", ");
        if(reg != NULL)
        {
            ttype = get_reg_imm(reg, &immreg);
            imm12 = immreg.imm;
        }
    }

    puts("I-Type");
    printf("opcode: 0x%x\n", opc);
    printf("rd: %u\n", rd);
    printf("funt3: 0x%x\n", func3);
    printf("rs1: %u\n", rs1);
    printf("imm12: %u\n", imm12);
    puts("");

    instr->instruction |= opc;
    instr->instruction |= (rd << 7);
    instr->instruction |= (func3 << 12);
    instr->instruction |= (rs1 << 15);
    instr->instruction |= (imm12 << 20);

    return ret;
}

int parse_S_type(char *opr, instruction_t *instr, opcode_t * opcode)
{
}

// FIXME: parse and assemble SB-type instruction 
int parse_SB_type(char *opr, instruction_t *instr, opcode_t *opcode)
{
    instr->instruction = 0;
    immreg_t immreg;
    int ret = 0;
    char *reg;
    int ttype;
    uint32_t opc = opcode->code;
    uint32_t func3 = opcode->func3;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    uint32_t imm12 = 0;
    uint8_t imm_found = 0;    

    reg = strtok(NULL, ", ");
    if(reg != NULL)
    {
        ttype = get_reg_imm(reg, &immreg);
        rs1 = immreg.reg;
    }

    reg = strtok(NULL, ", ");
    if(reg != NULL)
    {
        ttype = get_reg_imm(reg, &immreg);
        rs2 = immreg.reg;
    }

    if(!imm_found)
    {
        reg = strtok(NULL, ", ");
        if(reg != NULL)
        {
            ttype = get_reg_imm(reg, &immreg);
            imm12 = immreg.imm;
        }
    }

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

    instr->instruction |= opc;
    instr->instruction |= (imm_11 << 7);
    instr->instruction |= (imm_4_1 << 8);
    instr->instruction |= (func3 << 12);
    instr->instruction |= (rs1 << 15);
    instr->instruction |= (rs2 << 20);
    instr->instruction |= (imm_10_5 << 25);
    instr->instruction |= (imm_12 << 31);

    return ret;
}

int parse_U_type(char *opr, instruction_t *instr, opcode_t *opcode)
{
}

int parse_UJ_type(char *opr, instruction_t *instr, opcode_t *opcode)
{
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

