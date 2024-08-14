/* RISC-V assembler implementation.
 *
 * Build executable as follows: make clean && make 
 * Execute as follows: ./assembler trace_1 
 *
 * Modified: Naga Kandasamy
 * Date: July 16, 2024
 *
 * Student name(s): Cole Bardin
 * Date: August 2, 2024
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(int argc, char **argv)
{	
    uint64_t PC = 0;
    ins_list_t *l;
    instruction_t *ins;

    if (argc != 2) 
    {
        printf("Usage: %s <trace-file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    l = load_instructions(argv[1]);
    if(l == NULL)
    {
        fprintf(stderr, "ERROR: Failed to initialize instruction list\n");
        exit(EXIT_FAILURE);
    }

    for(ins = l->head; ins != NULL; ins = ins->next)
    {
        printf("Instruction at PC: %llu\n", PC);
        unsigned mask = (1 << 31);
        for(int i = 31; i >= 0; i--)
        {
            if(ins->bin & mask) printf("1");
            else printf("0"); 
            if(!(i % 4)) printf(" ");

            mask >>= 1;
        }
        puts("");

        PC += 4;
    }

    ins_list_delete(l);
    exit(EXIT_SUCCESS);
}

