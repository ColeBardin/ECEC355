#include "parser.h"


// FIXME: add support to identify and parse I-type and SB-type instructions 
void load_instructions(instruction_memory_t *i_mem, const char *trace)
{
    printf("Loading trace file: %s\n\n", trace);
    FILE *fd = fopen(trace, "r");
    if (fd == NULL) {
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
        if (strcmp(raw_instr, "add") == 0 ||
            strcmp(raw_instr, "sub") == 0 ||
            strcmp(raw_instr, "sll") == 0 ||
            strcmp(raw_instr, "srl") == 0 ||
            strcmp(raw_instr, "xor") == 0 ||
            strcmp(raw_instr, "or")  == 0 ||
            strcmp(raw_instr, "and") == 0) {
            parse_R_type(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }

        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}

// Parse and assemble R-type instruction 
void parse_R_type(char *opr, instruction_t *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "add") == 0) {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = get_register_number(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = get_register_number(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned rs_2 = get_register_number(reg);

    // Print the tokens 
    printf("Opcode: %u\n", opcode);
    printf("funct3: %u\n", funct3);
    printf("funct7: %u\n", funct7);
    printf("Source register 1: %u\n", rs_1);
    printf("Source register 2: %u\n", rs_2);
    printf("Destination register: %u\n", rd);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}


// FIXME: parse and assemble I-type instruction 
void parse_I_type(char *opr, instruction_t *instr)
{
     
}

// FIXME: parse and assemble SB-type instruction 
void parse_SB_type(char *opr, instruction_t *instr)
{
     
}

unsigned int get_register_number(char *reg)
{
    unsigned i = 0;

    for (i; i < NUM_OF_REGS; i++) {
        if (strcmp(REGISTER_NAME[i], reg) == 0)
            break;
    }

    return i;
}
