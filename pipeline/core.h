#ifndef __CORE_H__
#define __CORE_H__

#include "instruction.h"

#include <stdbool.h>
#include <stdlib.h> 
#include <stdint.h>
#define BOOL bool
#define MEM_SIZE 1024       // Size of memory in bytes 
#define NUM_REGISTERS 32    // Size of register file 

typedef uint8_t byte_t;
typedef int64_t signal_t;
typedef int64_t register_t;

typedef struct core_s core_t;
typedef enum aluctrl_e aluctrl_t;

enum aluctrl_e
{
    ALUCTRL_AND = 0,  // 0000
    ALUCTRL_OR,       // 0001
    ALUCTRL_ADD,      // 0010
    ALUCTRL_SUB = 6,  // 0110
    ALUCTRL_LT,       // 0111
    ALUCTRL_SRL,      // 1000
    ALUCTRL_SLL,      // 1001
    ALUCTRL_SRA = 10, // 1010
    ALUCTRL_XOR = 13  // 1101
};

// Definition of the various control signals
typedef struct control_signals_s {
    signal_t Branch;
    signal_t MemRead;
    signal_t MemtoReg;
    signal_t ALUOp;
    signal_t MemWrite;
    signal_t ALUSrc;
    signal_t RegWrite;
} control_signals_t;

typedef struct IF_ID_s
{
    bool valid;
    addr_t PC;
    uint32_t ins;
} IF_ID_t;

typedef struct ID_EX_s
{
    bool valid;
    control_signals_t ctrl;
    addr_t PC;
    register_t rs1_addr;
    register_t rs1;
    register_t rs2_addr;
    register_t rs2;
    register_t rd_addr;
    register_t imm;
    byte_t func3;
    byte_t func7;
} ID_EX_t;

typedef struct EX_MEM_s
{
    bool valid;
    signal_t RegWrite;
    signal_t MemtoReg;
    signal_t MemWrite;
    signal_t MemRead;
    register_t ALU_ret;
    register_t rs2;
    register_t rd_addr;
} EX_MEM_t;

typedef struct MEM_WB_s
{
    bool valid;
    signal_t RegWrite;
    signal_t MemtoReg;
    register_t reg_data_in;
    register_t ALU_ret;
    register_t rd_addr;
} MEM_WB_t;

typedef struct PC_reg_s
{
    signal_t PCSrc;
    addr_t PC_imm_sum;
} PC_reg_t;

typedef struct HDU_ctrl_s
{
    byte_t PCWrite;
    byte_t IF_ID_Write;
    byte_t ctrl_clear;
} HDU_ctrl_t;

typedef struct fwd_ctrl_s
{
    byte_t fwdA;
    byte_t fwdB;
    register_t reg_data_in;
} fwd_ctrl_t;

// Definition of the RISC-V core
struct core_s {
    tick_t clk;                         // Core clock
    addr_t PC;                          // Program counter
    i_mem_t *ins_mem;                   // Instruction memory 
    byte_t data_mem[MEM_SIZE];          // Data memory
    register_t reg_file[NUM_REGISTERS]; // Register file.
    IF_ID_t IF_ID;
    ID_EX_t ID_EX;
    EX_MEM_t EX_MEM;
    MEM_WB_t MEM_WB;
    PC_reg_t PC_reg;
    fwd_ctrl_t fwd_ctrl;
    bool (*tick)(struct core_s *core);  // Simulate function 
};

core_t *init_core(i_mem_t *i_mem);
bool tick_func(core_t *core);
void hazard_detection_unit(register_t D_rs1_addr, register_t D_rs2_addr, register_t E_rd_addr, signal_t D_MemRead, HDU_ctrl_t *HDU_ctrl); 
void IF(addr_t PC, i_mem_t *ins_mem, IF_ID_t *IF_ID);
void ID(IF_ID_t *IF_ID, register_t reg_file[], ID_EX_t *ID_EX);
void EX(ID_EX_t *ID_EX, fwd_ctrl_t *fwd_ctrl, EX_MEM_t *EX_MEM, PC_reg_t *PC_reg);
void MEM(EX_MEM_t *EX_MEM, byte_t data_mem[], MEM_WB_t *MEM_WB, fwd_ctrl_t *fwd_ctrl);
void WB(MEM_WB_t *MEM_WB, register_t reg_file[]);
void PC(PC_reg_t *PC_reg, addr_t *PC);
bool running(core_t *core); 
void forwarding_unit(ID_EX_t *ID_EX, EX_MEM_t *EX_MEM, MEM_WB_t *MEM_WB, fwd_ctrl_t *fwd_ctrl);
void control_unit(signal_t input, control_signals_t *signals);
signal_t ALU_control_unit(signal_t ALUOp, signal_t funct7, signal_t funct3);
signal_t imm_gen(signal_t input);
void ALU(signal_t input_0, signal_t input_1, signal_t ALU_ctrl_signal, signal_t *ALU_result, signal_t *zero);
void MEMORY(byte_t data_mem[], signal_t addr, signal_t data_in, signal_t *data_out, signal_t read, signal_t write);
void REG(register_t reg_file[], signal_t addr, register_t data_in, register_t *data_out, signal_t read, signal_t write);
signal_t MUX(signal_t sel, signal_t input_0, signal_t input_1);
signal_t Add(signal_t input_0, signal_t input_1);
signal_t ShiftLeft1(signal_t input);
void print_core_state(core_t *core);
void print_data_memory(core_t *core, unsigned int start, unsigned int end);

#endif

