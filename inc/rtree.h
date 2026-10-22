#ifndef RTREE_H
#define RTREE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

typedef struct rtree_node{
    char *prefix;
    size_t prefix_len;
    struct rtree_node **children;
    size_t n_children;
    size_t children_cap;
    bool is_end;
    uint32_t value;
} rtree_node;

typedef struct{
    rtree_node *root;
    size_t count;
} rtree;


rtree *rtree_create(void);
void rtree_free(rtree *tree);

rtree *rtree_insert(rtree *tree, const char *key, uint32_t value);
rtree_node *rtree_lookup(rtree *tree, const char *key);
rtree *rtree_delete(rtree *tree, const char *key);

void rtree_print(rtree *tree, int level);

#endif