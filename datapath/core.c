#include "core.h"

core_t *init_core(ins_list_t *i_mem)
{
    if(i_mem == NULL || i_mem->head == NULL)
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
    core->i_mem = i_mem;
    core->tick = tick_func;
    core->i_cur = i_mem->head;

    // FIXME, initialize data memory here.
    memset(core->data_mem, 0, MEM_SIZE);

    // FIXME, initialize register file here.
    memset(core->reg_file, 0, NUM_REGISTERS * 8);

    return core;
}

// FIXME Implement simulatoe main function 
bool tick_func(core_t *core)
{
    // TODO: yeah revert things back to an array for constant time shit. but dont revert back to the old data structures. just use my new ones 

    // (Step 1) Fetch instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    
    // (Step 2) Decode instruction and execute it
    
    // (Step N) Increment PC. FIXME Account for branch statement.
    core->PC += 4;

    ++core->clk;
    // Are we reaching the final instruction?
    if (core->PC > core->instr_mem->last->addr) {
        return false;
    }
    
    return true;
}

// FIXME Implement the control Unit. Refer to Figure 4.18 in textbook.
void control_unit(signal_t input, control_signals_t *signals)
{
    // For R-type
    if (input == 51) {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
}

// FIXME Implement ALU control unit. Refer to Figure 4.12 in textbook.
signal_t ALU_control_unit(signal_t ALUOp, signal_t Funct7, signal_t Funct3)
{
    // For add
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0) {
        return 2;
    }
}

// FIXME Implement immediate generation.
signal_t imm_gen(signal_t input)
{

}

// FIXME Implement ALU operations.
void ALU(signal_t input_0, signal_t input_1, signal_t ALU_ctrl_signal, signal_t *ALU_result, signal_t *zero)
{
    // Addition
    if (ALU_ctrl_signal == 2) {
        *ALU_result = (input_0 + input_1);
        if (*ALU_result == 0) 
            *zero = 1; 
        else 
            *zero = 0; 
    }
}

// 2x1 MUX
signal_t MUX(signal_t sel, signal_t input_0, signal_t input_1)
{
    if (sel == 0)  
        return input_0;
    else 
        return input_1;
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
