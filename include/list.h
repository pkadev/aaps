#ifndef LIST_H__
#define LIST_H__

struct list_node_t *list_init();

struct list_node_t
{
    int data;
    struct list_node_t *next;
};
#endif
