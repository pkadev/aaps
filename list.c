#include <stdlib.h>

struct list_node_t
{
    int data;
    struct list_node_t *next;
};

struct list_node_t *list_init()
{
    struct list_node_t *p;
    p = malloc(sizeof(struct list_node_t));
    if (p)
        return p;
    else
        return 0;
}

