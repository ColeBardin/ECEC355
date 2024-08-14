/* Simulator for the RISC-V single cycle datapath.
 *
 * Build as follows:
 *  $make clean && make
 *
 * Execute as follows: 
 *  $./RISCV_core <trace file>
 *
 * Modified by: Naga Kandasamy
 * Date: August 8, 2024
 *
 * Student name(s):
 * Date:

 */

#include <stdio.h>

#include "core.h"
#include "parser.h"

int main(int argc, const char **argv)
{	
    if (argc != 2) {
        printf("Usage: %s %s\n", argv[0], "<trace-file>");
        exit(EXIT_SUCCESS);
    }

    // FIXME Translate assembly instructions into binary format; store binary instructions into instruction memory.
    instruction_memory_t instr_mem;
    instr_mem.last = NULL;
    load_instructions(&instr_mem, argv[1]);

    // FIXME Implement core.{h,c}
    core_t* core = init_core(&instr_mem);

    // FIXME Simulate core 
    while (core->tick(core));
    printf("Simulation complete.\n");
    printf("\n");

    // Print register file 
    print_core_state(core);
    printf("\n");

    // Print data memory in the address range [start, end). start address is inclusive, end address is exclusive
    unsigned int start = 0;
    unsigned int end = 32;
    print_data_memory(core, start, end);

    free(core);    
}
