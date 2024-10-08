/* Simulator for the RISC-V pipeline
 *
 * Build as follows:
 *  $make clean && make [VERBOSE=(0|1)]
 *
 * Execute as follows: 
 *  $./RISCV_core <trace file>
 *
 * Modified by: Naga Kandasamy
 * Date: September 9, 2024
 *
 * Student name(s): Cole Bardin
 * Date: September 3, 2024

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

#if VERBOSE == 1
    puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
#endif

    core_t *core = init_core(m);
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

