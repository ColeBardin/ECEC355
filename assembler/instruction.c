#include "instruction.h"
#include <stdlib.h>

ins_list_t *ins_list_init()
{
    ins_list_t *l;
    l = malloc(sizeof(ins_list_t));
    if(l == NULL) return NULL;
    l->head = NULL;
    l->tail = NULL;

    return l;
}

int ins_list_delete(ins_list_t *l)
{
    instruction_t *c, *n;
    if(l == NULL) return 1;
    if(l->head != NULL)
    {
        for(c = l->head; c != NULL; c = n)
        {
            n = c->next;
            free(c);
        }
    }
    free(l);
    return 0;
}

int ins_list_add(ins_list_t *l, uint64_t addr, uint32_t bin)
{
    instruction_t *n, *p;

    if(l == NULL) return 1;

    n = malloc(sizeof(instruction_t));
    if(n == NULL) return 2;

    n->addr = addr;
    n->bin = bin;
    n->next = NULL;

    if(l->head == NULL) l->head = n;
    else
    {
        for(p = l->head; p->next != NULL; p = p->next);
        p->next = n;
    }
    return 0;
}

