#include "instruction.h"
#include <stdlib.h>

i_mem_t *i_mem_init()
{
    i_mem_t *m;
    m = malloc(sizeof(i_mem_t));
    if(m == NULL) return NULL;
    m->cnt= 0;
}

int i_mem_delete(i_mem_t *m)
{
    if(m == NULL) return 1;
    free(m);
    return 0;
}

int i_mem_add(i_mem_t *m, uint64_t addr, uint32_t bin, opcode_t *opc)
{
    instruction_t *i;
    uint64_t index;

    if(m == NULL || opc == NULL) return 1;

    index = m->cnt;
    if(index >= IMEMSZ) return 1;

    i = &m->mem[index];
    i->addr = addr;
    i->bin = bin;
    i->opc = *opc;
    m->cnt++;

    return 0;
}

