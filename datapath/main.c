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
#include <stdlib.h>
#include "core.h"
#include "parser.h"

int main(int argc, char **argv)
{	

    uint64_t PC = 0;
    i_mem_t *m;
    instruction_t *ins;

    if (argc != 2) 
    {
        printf("Usage: %s <trace-file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    m = load_instructions(argv[1]);
    if(m == NULL)
    {
        fprintf(stderr, "ERROR: Failed to initialize instruction list\n");
        exit(EXIT_FAILURE);
    }

    core_t* core = init_core(m);
    if(core == NULL)
    {
        fprintf(stderr, "ERROR: Failed to initialize core\n");
        exit(EXIT_FAILURE);
    }

    while (core->tick(core));
    puts("Simulation complete.\n");

    print_core_state(core);
    puts("");

    // Print data memory in the address range [start, end). start address is inclusive, end address is exclusive
    unsigned int start = 0;
    unsigned int end = 32;
    print_data_memory(core, start, end);

    i_mem_delete(m);
    free(core);
    exit(EXIT_SUCCESS);
}

