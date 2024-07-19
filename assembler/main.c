/* RISC-V assembler implementation.
 *
 * Build executable as follows: make clean && make 
 * Execute as follows: ./assembler trace_1 
 *
 * Modified: Naga Kandasamy
 * Date: July 16, 2024
 *
 * Student name(s): Cole Bardin
 * Date: 
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char **argv)
{	
    if (argc != 2) {
        printf("Usage: %s %s\n", argv[0], "<trace-file>");
        exit(EXIT_SUCCESS);
    }
    
    // Load instructions from the trace file into memory 
    instruction_memory_t instr_mem;
    instr_mem.last = NULL;
    load_instructions(&instr_mem, argv[1]);

    // Print instructions in binary format 
    unsigned PC = 0;
    while (1) {
        instruction_t *instr = &(instr_mem.instructions[PC / 4]);
        printf("\nInstruction at PC: %u\n", PC);
        unsigned mask = (1 << 31);
        for (int i = 31; i >= 0; i--) {
            if (instr->instruction & mask) 
                printf("1 ");
            else 
                printf("0 "); 

            mask >>= 1;
        }
        printf("\n");
        
        // Check if we have reached the end of instruction memory
        if (instr == instr_mem.last) 
            break; 

        PC += 4;    // Increment PC to point to next instruction 
    }

    exit(EXIT_SUCCESS);
}
