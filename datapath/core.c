#include "core.h"
#include <string.h>
#include <stdio.h>

core_t *init_core(i_mem_t *i_mem)
{
    if(i_mem == NULL || i_mem->cnt == 0)
    {
        fprintf(stderr, "ERROR: init_core received invalid i_mem\n");
        return NULL;
    }

    core_t *core = (core_t *)malloc(sizeof(core_t));
    if (core == NULL)
    {
        fprintf(stderr, "ERROR: Failed to malloc core struct\n");
        return NULL;
    }
    
    core->clk = 0;
    core->PC = 0;
    core->ins_mem = i_mem;
    core->tick = tick_func;

    memset(core->data_mem, 0, MEM_SIZE);
    memset(core->reg_file, 0, NUM_REGISTERS * sizeof(register_t));

    return core;
}

bool tick_func(core_t *core)
{
    // (Step 1) Instruction Fetch
    instruction_t *ins;
    uint32_t bin;

    ins = &core->ins_mem->mem[core->PC / 4];
    bin = ins->bin;
    
    // (Step 2) Instruction Decode
    byte_t opcode, func3, func7;
    control_signals_t ctrl;
    signal_t ALU_ctrl;
    signal_t imm;
    signal_t input0, input1, ALU_ret, zero_ret;
    signal_t rd, rs1, rs2;
    signal_t rd_addr, rs1_addr, rs2_addr;

    opcode = bin & 0x7F;
    func3 = (bin >> 12) & 0x7;
    func7 = (opcode == 0x33 || opcode == 0x3B) ? (bin >> 25) & 0x7F : 0;

    control_unit(opcode, &ctrl);
    ALU_ctrl = ALU_control_unit(ctrl.ALUOp, func7, func3);
    imm = imm_gen(bin);

    rd_addr = (bin >> 7) & 0x1F;
    rs1_addr = (bin >> 15) & 0x1F;
    rs2_addr = (bin >> 20) & 0x1F;

    REG(core->reg_file, rs1_addr, 0, &rs1, 1, 0);
    REG(core->reg_file, rs2_addr, 0, &rs2, 1, 0);

    input0 = rs1;
    input1 = MUX(ctrl.ALUSrc, rs2, imm);

    // (Step 3) Execute
    ALU(input0, input1, ALU_ctrl, &ALU_ret, &zero_ret);
    
    // (Step 4) Memory
    signal_t mem_out;

    MEM(core->data_mem, ALU_ret, rs2, &mem_out, ctrl.MemRead, ctrl.MemWrite);

    // (Step 5) Write Back 
    signal_t reg_data_in;
    reg_data_in = MUX(ctrl.MemtoReg, ALU_ret, mem_out);
    REG(core->reg_file, rd_addr, reg_data_in, NULL, 0, ctrl.RegWrite);

#if VERBOSE == 1
    printf("PC: %d\n", core->PC);
    printf("opcode: 0x%x\n", opcode);
    printf("func3: %d\n", func3);
    printf("func7: %d\n", func7);
    printf("rd: x%d\n", rd_addr);
    printf("rs1: x%d = %d\n", rs1_addr, rs1);
    printf("rs2: x%d = %d\n", rs2_addr, rs2);
    printf("imm: %d\n", imm);
    printf("ALU SRC: %d\n", ctrl.ALUSrc);
    printf("branch: %d\n", ctrl.Branch);
    printf("ALU ctrl: %d\n", ALU_ctrl);
    printf("input0: %d\n", input0);
    printf("input1: %d\n", input1);
    printf("alu zero: %d\n", zero_ret);
    printf("ALU ret: %d\n", ALU_ret);
    if(ctrl.MemWrite) printf("MEM Write: %d -> @%d\n", rs2, ALU_ret);
    if(ctrl.MemRead) printf("MEM Read: %d <- @%d\n", mem_out, ALU_ret);
    if(ctrl.RegWrite) printf("REG Write: %d -> x%d FM:%d\n", reg_data_in, rd_addr, ctrl.MemtoReg);
#endif

    // (Step 6) Increment PC or Branch
    core->PC = Add(core->PC, MUX(ctrl.Branch && zero_ret, 4, imm));

#if VERBOSE == 1
    printf("NEW PC: %d\n", core->PC);
    puts("");
#endif

    core->clk++;
    // Are we reaching the final instruction?
    if (core->PC / 4 >= core->ins_mem->cnt) return false;
    
    return true;
}

void control_unit(signal_t input, control_signals_t *signals)
{
    // For R-type
    if(input == 0x33) 
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
    else if(input == 0x23) // Store instruction
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 1;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    else if(input == 0x03) // Load instruction
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    else if(input == 0x13) // I-Type
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2; 
    }
    else if(input == 0x63) // SB type
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
    }
}

signal_t ALU_control_unit(signal_t ALUOp, signal_t Funct7, signal_t Funct3)
{
    if(ALUOp == 0) return ALUCTRL_ADD;
    if(ALUOp == 1) return ALUCTRL_SUB;
    if(ALUOp == 2)
    {
        if(Funct7 == 0 && Funct3 == 0)  return ALUCTRL_ADD;
        if(Funct7 == 0 && Funct3 == 1)  return ALUCTRL_SLL;
        if(Funct7 == 0 && Funct3 == 7) return ALUCTRL_AND;
        if(Funct7 == 0 && Funct3 == 6) return ALUCTRL_OR;
        if(Funct7 == 0x20 && Funct3 == 0) return ALUCTRL_SUB;
    }
    fputs("ALU Control Unit failed to parse signal\n", stderr); 
}

signal_t imm_gen(signal_t input)
{
    signal_t im = 0;
    byte_t opcode = input & 0x7F;

    if(opcode == 0x33 || opcode == 0x3B) return 0;
    if(opcode == 0x03 || opcode == 0x13)
    {
        im = input >> 20;
        if(im & (1 << 11)) im |= ~(0xFFF);
        return im;
    }
    if(opcode == 0x23)
    {
        im = ((input >> 7) & 0x1F) | ((input >> 20) & 0xFE0);
        if(im & (1 << 11)) im |= ~(0xFFF);
        return im;
    }
    if(opcode == 0x63)
    {
        signal_t i_12 = (input >> 31) & 0x1;
        signal_t i_11 = (input >> 7) & 0x1;
        signal_t i_10_5 = (input >> 25) & 0x3F;
        signal_t i_4_1 = (input >> 8) & 0xF;

        im |= i_12 << 12;
        im |= i_11 << 11;
        im |= i_10_5 << 5; 
        im |= i_4_1 << 1;
        im |= i_12 ? ~(0x1FFF) : 0;
        return im;
    }
    puts("BOY WHAT THE HEEEELLLLLLL");
    return 0;
}

void ALU(signal_t input_0, signal_t input_1, signal_t ALU_ctrl_signal, signal_t *ALU_result, signal_t *zero)
{
    if(!ALU_result || !zero) 
    {
        fputs("ERROR: ALU received NULL pointer\n", stderr);
        exit(EXIT_FAILURE);
    }

    switch(ALU_ctrl_signal)
    {
        case ALUCTRL_AND:
            *ALU_result = input_0 & input_1;
            break;
        case ALUCTRL_OR:
            *ALU_result = input_0 | input_1;
            break;
        case ALUCTRL_ADD:
            *ALU_result = input_0 + input_1;
            break;
        case ALUCTRL_SUB:
            *ALU_result = input_0 - input_1;
            break;
        case ALUCTRL_SRL:
            *ALU_result = input_0 >> input_1;
            break;
        case ALUCTRL_SLL:
            *ALU_result = input_0 << input_1;
            break;
        default:
            fputs("ERROR: Unrecognized ALUCTRL\n", stderr);
            break;
    }

    *zero = *ALU_result == 0;
}

// Perform read and write memory operations
void MEM(byte_t data_mem[], signal_t addr, signal_t data_in, signal_t *data_out, signal_t read, signal_t write)
{
    if(!data_mem)
    {
        fputs("ERROR: MEM received a NULL pointer\n", stderr);
        exit(EXIT_FAILURE);
    }

    if(read && data_out)
    {
        uint32_t out;
        memcpy(&out, &data_mem[addr], 1);
        *data_out = out;
    }

    if(write) memcpy(&data_mem[addr], &data_in, 4);
}

// Perform read and write register operations
void REG(register_t reg_file[], signal_t addr, register_t data_in, register_t *data_out, signal_t read, signal_t write)
{
    if(!reg_file) return;

    if(read && data_out) *data_out = reg_file[addr];
    if(write) reg_file[addr] = data_in;
}

// 2x1 MUX
signal_t MUX(signal_t sel, signal_t input_0, signal_t input_1)
{
    return sel ? input_1 : input_0;
}

// Add
signal_t Add(signal_t input_0, signal_t input_1)
{
    return (input_0 + input_1);
}

// ShiftLeft1
signal_t ShiftLeft1(signal_t input)
{
    return input << 1;
}

// Print the contents of the register file
void print_core_state(core_t *core)
{
    printf("Register file\n");
    int i;
    for (i = 0; i < NUM_REGISTERS; i++)
        printf("x%d \t: %lld\n", i, core->reg_file[i]);
}

// Dump contents of data memory from [start, end). The start of the range is inclusive, end is exclusive. 
void print_data_memory(core_t *core, unsigned int start, unsigned int end)
{
     if ((start >= MEM_SIZE) || (end > MEM_SIZE)) {
          printf("Address range [%d, %d) is invalid\n", start, end);
          return;
     }

     printf("Data memory: bytes (in hex) within address range [%d, %d)\n", start, end);
     int i;
     for (i = start; i < end; i++)
         printf("%d: \t %02x\n", i, core->data_mem[i]);
}
