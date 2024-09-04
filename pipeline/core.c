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
    // Make copy of inter-stage registers
    IF_ID_t IF_ID = core->IF_ID;
    ID_EX_t ID_EX = core->ID_EX;
    EX_MEM_t EX_MEM = core->EX_MEM;
    MEM_WB_t MEM_WB = core->MEM_WB;
    // (Step 0) Determine data hazards & forwarding
    hazard_detection_unit(&ID_EX, &EX_MEM, &core->HDU_ctrl);
    forwarding_unit(&ID_EX, &EX_MEM, &MEM_WB, &core->fwd_ctrl);
    // (Step 1) Instruction Fetch
    IF(core->PC, core->ins_mem, &core->HDU_ctrl, &core->IF_ID);
    // (Step 5) Write Back 
    WB(&MEM_WB, core->reg_file);
    // (Step 2) Instruction Decode
    if(&core->HDU_ctrl.stall) IF_ID = core->IF_ID;
    ID(&IF_ID, core->reg_file, &core->HDU_ctrl, &core->ID_EX);
    // (Step 3) Execute
    EX(&ID_EX, &core->fwd_ctrl, &core->HDU_ctrl, &core->EX_MEM, &core->PC_reg);
    // (Step 4) Memory
    MEM(&EX_MEM, core->data_mem, &core->MEM_WB, &core->fwd_ctrl);
    // (Step 6) Increment PC or Branch from EX
    PC(&core->PC_reg, &core->PC, &core->HDU_ctrl);

    core->clk++;
#if VERBOSE == 1
    puts("");
#endif
    // Are we reaching the final instruction?
    if (core->PC / 4 >= core->ins_mem->cnt) return running(core);
    
    return true;
}

void hazard_detection_unit(ID_EX_t *ID_EX, EX_MEM_t *EX_MEM, HDU_ctrl_t *HDU_ctrl)
{
    HDU_ctrl->stall = 0;
    HDU_ctrl->PCWrite = 1;
    HDU_ctrl->IF_ID_Write = 1;
    HDU_ctrl->ctrl_clear = 0;

    if(!ID_EX->valid || !EX_MEM->valid) return;
    if(!EX_MEM->MemRead || !EX_MEM->RegWrite) return;
    if(ID_EX->rs1_addr == EX_MEM->rd_addr || ID_EX->rs2_addr == EX_MEM->rd_addr)
    {
        HDU_ctrl->stall = 1;
        HDU_ctrl->PCWrite = 0;
        HDU_ctrl->IF_ID_Write = 0;
        HDU_ctrl->ctrl_clear = 1;
#if VERBOSE == 1
    puts("STALL\n");
#endif
    }
    return;
}

void IF(addr_t PC, i_mem_t *ins_mem, HDU_ctrl_t *HDU_ctrl, IF_ID_t *IF_ID)
{
    uint32_t bin;
    bool valid;
    bool IF_ID_Write = HDU_ctrl->IF_ID_Write;
    bool stall = HDU_ctrl->stall;
    
    if(!IF_ID_Write) PC -= 4;
    if(PC / 4 >= ins_mem->cnt)
    {
        bin = 0;
        valid = false; 
    }
    else
    {
        bin = ins_mem->mem[PC / 4].bin;
        valid = true;
    }

    IF_ID->valid = valid;
    IF_ID->PC =    PC;
    IF_ID->ins =   bin;   
#if VERBOSE == 1
    puts("FETCH:");
    if(valid) puts("\tVALID");
    printf("\tPC: %d\n", PC);
    printf("\tbin: 0x%08x\n", bin);
    printf("\tIF_ID_Write: %d\n", IF_ID_Write);
    printf("\tSTALL: %d\n", stall);
#endif
    return;
}

void ID(IF_ID_t *IF_ID, register_t reg_file[], HDU_ctrl_t *HDU_ctrl, ID_EX_t *ID_EX)
{
    byte_t opcode, func3, func7;
    control_signals_t ctrl = {0};
    register_t imm;
    signal_t rd_addr, rs1_addr, rs2_addr;
    register_t rs1, rs2;
    addr_t PC =    IF_ID->PC;
    uint32_t bin = IF_ID->ins;
    bool stall = HDU_ctrl->stall;

    opcode = bin & 0x7F;
    func3 = (bin >> 12) & 0x7;
    func7 = (opcode == 0x33 || opcode == 0x3B) ? (bin >> 25) & 0x7F : 0;
    control_unit(opcode, &ctrl);
    imm = imm_gen(bin);

    rd_addr = (bin >> 7) & 0x1F;
    rs1_addr = (bin >> 15) & 0x1F;
    rs2_addr = (bin >> 20) & 0x1F;

    REG(reg_file, rs1_addr, 0, &rs1, 1, 0);
    REG(reg_file, rs2_addr, 0, &rs2, 1, 0);

    ID_EX->valid =    IF_ID->valid;
    ID_EX->PC =       PC;
    ID_EX->func3 =    func3;
    ID_EX->func7 =    func7;
    ID_EX->ctrl =     ctrl;
    ID_EX->imm =      imm;
    ID_EX->rs1_addr = rs1_addr; 
    ID_EX->rs1 =      rs1;
    ID_EX->rs2_addr = rs2_addr; 
    ID_EX->rs2 =      rs2;
    ID_EX->rd_addr =  rd_addr;
#if VERBOSE == 1
    puts("DECODE:");
    if(IF_ID->valid) puts("\tVALID");
    printf("\topcode: 0x%x\n", opcode);
    printf("\tfunc3: %d\n", func3);
    printf("\tfunc7: %d\n", func7);
    printf("\trd: x%d\n", rd_addr);
    printf("\trs1: x%d = %d\n", rs1_addr, rs1);
    printf("\trs2: x%d = %d\n", rs2_addr, rs2);
    printf("\timm: %d\n", imm);
    printf("\tALU SRC: %d\n", ctrl.ALUSrc);
    printf("\tbranch: %d\n", ctrl.Branch);
    printf("\tRegWrite: %d\n", ctrl.RegWrite);
    printf("\tSTALL: %d\n", stall);
#endif
    return;
}

void EX(ID_EX_t *ID_EX, fwd_ctrl_t *fwd_ctrl, HDU_ctrl_t *HDU_ctrl, EX_MEM_t *EX_MEM, PC_reg_t *PC_reg)
{ 
    signal_t PCSrc;
    addr_t PC_imm_sum;
    signal_t input0, input1;
    register_t ALU_ret;
    signal_t ALU_zero;
    byte_t ALU_ctrl;
    bool stall = HDU_ctrl->stall;
    register_t rd_addr = ID_EX->rd_addr;
    addr_t PC =          ID_EX->PC;
    byte_t func3 =       ID_EX->func3;
    byte_t func7 =       ID_EX->func7;
    signal_t rs1 =       ID_EX->rs1;
    signal_t rs2 =       ID_EX->rs2;
    signal_t imm =       ID_EX->imm;
    if(stall) memset(&ID_EX->ctrl, 0, sizeof(control_signals_t));
    signal_t ALUOp =     ID_EX->ctrl.ALUOp;
    signal_t ALUSrc =    ID_EX->ctrl.ALUSrc;
    signal_t Branch =    ID_EX->ctrl.Branch;

    ALU_ctrl = ALU_control_unit(ALUOp, func7, func3);
    switch(fwd_ctrl->fwdA)
    {
        case 0: // Normal operation
            rs1 = rs1;
            break;
        case 1: // EX hazard
            rs1 = EX_MEM->ALU_ret;
            puts("rs1 fwd from ex");
            break;
        case 2: // MEM hazard
            rs1 = fwd_ctrl->reg_data_in;
            puts("rs1 fwd from mem");
            break;
    }
    switch(fwd_ctrl->fwdB)
    {
        case 0: // Normal operation
            rs2 = rs2;
            break;
        case 1: // EX hazard
            rs2 = EX_MEM->ALU_ret;
            puts("rs2 fwd from ex");
            break;
        case 2: // MEM hazard
            rs2 = fwd_ctrl->reg_data_in;
            puts("rs2 fwd from mem");
            break;
    }
    input0 = rs1;
    input1 = MUX(ALUSrc, rs2, imm);
    ALU(input0, input1, ALU_ctrl, &ALU_ret, &ALU_zero);
    PC_imm_sum = Add(PC, imm);
    PCSrc = Branch && ALU_zero;
    if(stall) ALU_ret = 0;

    EX_MEM->valid =      ID_EX->valid;
    EX_MEM->ALU_ret =    ALU_ret;
    EX_MEM->RegWrite =   ID_EX->ctrl.RegWrite;
    EX_MEM->MemtoReg =   ID_EX->ctrl.MemtoReg;
    EX_MEM->MemWrite =   ID_EX->ctrl.MemWrite;
    EX_MEM->MemRead =    ID_EX->ctrl.MemRead;
    EX_MEM->rd_addr =    rd_addr;
    EX_MEM->rs2 =        rs2;
    PC_reg->PCSrc =      PCSrc;
    PC_reg->PC_imm_sum = PC_imm_sum;
#if VERBOSE == 1
    puts("EXECUTE:");
    if(ID_EX->valid) puts("\tVALID");
    printf("\tALU ctrl: %d\n", ALU_ctrl);
    printf("\tinput0: %d\n", input0);
    printf("\tinput1: %d\n", input1);
    printf("\tALU zero: %d\n", ALU_zero);
    printf("\tALU ret: %d\n", ALU_ret);
    printf("\tRegWrite: %d\n", ID_EX->ctrl.RegWrite);
    printf("\tSTALL: %d\n", stall);
#endif
    return;
}

void MEM(EX_MEM_t *EX_MEM, byte_t data_mem[], MEM_WB_t *MEM_WB, fwd_ctrl_t *fwd_ctrl)
{
    signal_t mem_out;
    signal_t reg_data_in;
    register_t ALU_ret = EX_MEM->ALU_ret;
    register_t rs2 =     EX_MEM->rs2;
    signal_t RegWrite =  EX_MEM->RegWrite;
    signal_t MemtoReg =  EX_MEM->MemtoReg;
    signal_t MemRead =   EX_MEM->MemRead;
    signal_t MemWrite =  EX_MEM->MemWrite;
    register_t rd_addr = EX_MEM->rd_addr;

    MEMORY(data_mem, ALU_ret, rs2, &mem_out, MemRead, MemWrite);
    reg_data_in = MUX(MemtoReg, ALU_ret, mem_out);

    MEM_WB->valid =       EX_MEM->valid;
    MEM_WB->RegWrite =    RegWrite;
    MEM_WB->MemtoReg =    MemtoReg;
    MEM_WB->reg_data_in = reg_data_in;
    MEM_WB->ALU_ret =     ALU_ret;
    MEM_WB->rd_addr =     rd_addr;
    fwd_ctrl->reg_data_in = reg_data_in;
#if VERBOSE == 1
    puts("MEMORY:");
    if(EX_MEM->valid) puts("\tVALID");
    if(MemWrite) printf("\tMEM Write: %d -> @%d\n", rs2, ALU_ret);
    if(MemRead) printf("\tMEM Read: %d <- @%d\n", mem_out, ALU_ret);
    printf("\tData to WB: %d\n", reg_data_in);
    printf("\tRegWrite: %d\n", RegWrite);
    printf("\tHolding ALU_ret: %d\n", ALU_ret);
    printf("\tHolding rs2: %d\n", rs2);
    printf("\tHolding rd_addr: %d\n", rd_addr);
#endif
    return;
}

void WB(MEM_WB_t *MEM_WB, register_t reg_file[])
{
    signal_t reg_data_in = MEM_WB->reg_data_in;
    signal_t MemtoReg =    MEM_WB->MemtoReg;
    signal_t RegWrite =    MEM_WB->RegWrite;
    register_t ALU_ret =   MEM_WB->ALU_ret;
    register_t rd_addr =   MEM_WB->rd_addr;

    REG(reg_file, rd_addr, reg_data_in, NULL, 0, RegWrite);

#if VERBOSE == 1
    puts("WRITE BACK:");
    if(MEM_WB->valid) puts("\tVALID");
    if(RegWrite) printf("\tREG Write: %d -> x%d FromMem:%d\n", reg_data_in, rd_addr, MemtoReg);
    printf("\tHolding rd_addr: %d\n", rd_addr);
    printf("\tHolding ALU_ret: %d\n", ALU_ret);
#endif
    return;
}

void PC(PC_reg_t *PC_reg, addr_t *PC, HDU_ctrl_t *HDU_ctrl)
{
    addr_t new_PC;
    addr_t old_PC = *PC;
    addr_t PC_inc = Add(old_PC, 4);
    addr_t PC_imm_sum = PC_reg->PC_imm_sum;
    signal_t PCSrc = PC_reg->PCSrc;
    bool PCWrite = HDU_ctrl->PCWrite;

    new_PC = MUX(PCSrc, PC_inc, PC_imm_sum);

    if(PCWrite) *PC = new_PC;
#if VERBOSE == 1
    puts("PROGRAM COUNTER:");
    printf("\tPC = %d + %s\n", old_PC, PCWrite ? PCSrc ? "imm" : "4" : "0");
    printf("\tNEW PC: %d\n", new_PC);
    printf("\tPCWrite: %d\n", PCWrite);
#endif
    return;
}

bool running(core_t *core)
{
    bool valid = 0;
    valid |= core->IF_ID.valid;
    valid |= core->ID_EX.valid;
    valid |= core->EX_MEM.valid;
    valid |= core->MEM_WB.valid;
    return valid;
}

void forwarding_unit(ID_EX_t *ID_EX, EX_MEM_t *EX_MEM, MEM_WB_t *MEM_WB, fwd_ctrl_t *fwd_ctrl)
{
    fwd_ctrl->fwdA = 0;
    fwd_ctrl->fwdB = 0;

    // Detect MEM forwarding
    if(MEM_WB->RegWrite && ID_EX->valid && MEM_WB->valid)
    {
        if(ID_EX->rs1_addr == MEM_WB->rd_addr) fwd_ctrl->fwdA = 2;
        if(ID_EX->rs2_addr == MEM_WB->rd_addr) fwd_ctrl->fwdB = 2;
    }
    // Detect EX forwarding
    if(EX_MEM->RegWrite && ID_EX->valid && EX_MEM->valid && !EX_MEM->MemRead)
    {
        if(ID_EX->rs1_addr == EX_MEM->rd_addr) fwd_ctrl->fwdA = 1;
        if(ID_EX->rs2_addr == EX_MEM->rd_addr) fwd_ctrl->fwdB = 1;
    }

    return;
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

    // Zero stage
    if(input == 0) return 0;
    // R-Type
    if(opcode == 0x33 || opcode == 0x3B) return 0;
    // I-Type
    if(opcode == 0x03 || opcode == 0x13)
    {
        im = input >> 20;
        if(im & (1 << 11)) im |= ~(0xFFF);
        return im;
    }
    // S-Type
    if(opcode == 0x23)
    {
        im = ((input >> 7) & 0x1F) | ((input >> 20) & 0xFE0);
        if(im & (1 << 11)) im |= ~(0xFFF);
        return im;
    }
    // SB-Type
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
    printf("opcode: 0x%x\n", opcode);
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
void MEMORY(byte_t data_mem[], signal_t addr, signal_t data_in, signal_t *data_out, signal_t read, signal_t write)
{
    if(!data_mem)
    {
        fputs("ERROR: MEM received a NULL pointer\n", stderr);
        exit(EXIT_FAILURE);
    }

    if(read && data_out)
    {
        uint32_t out = 0;
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
     for (int i = start; i < end; i++) printf("%d: \t %02x\n", i, core->data_mem[i]);
}

