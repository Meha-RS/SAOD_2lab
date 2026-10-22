#include "rtree.h"


// Создание
static rtree_node *node_create(const char *prefix, size_t len){
    rtree_node *node = (rtree_node *)calloc(1, sizeof(*node));
    if(!node)
        return NULL;
    if(len > 0){
        node->prefix = strndup(prefix, len);
        if(!node->prefix){
            free(node);
            return NULL;
        }
        node->prefix_len = len;
    }
    return node;
}

rtree *rtree_create(void){
    rtree *tree = (rtree *)calloc(1, sizeof(*tree));
    if(!tree)
        return NULL;
    tree->root = node_create(NULL, 0);
    if(!tree->root){
        free(tree);
        return NULL;
    }
    tree->count = 0;
    return tree;
}

// Очистка
static void node_free(rtree_node *node){
    if(!node)
        return;
    free(node->prefix);
    free(node->children);
    free(node);
}

static void node_free_rec(rtree_node *node){
    if(!node)
        return;
    for(size_t i = 0; i < node->n_children; ++i)
        node_free_rec(node->children[i]);
    node_free(node);
}

void rtree_free(rtree *tree){
    if(!tree)
        return;
    node_free_rec(tree->root);
    free(tree);
}

// Поиск
static int child_find(rtree_node *node, char c){
    for(size_t i = 0; i < node->n_children; ++i)
        if(node->children[i]->prefix[0] == c)
            return (int)i;
    return -1;
}

rtree_node *rtree_lookup(rtree *tree, const char *key){
    if(!tree || !key || key[0] == '\0')
        return NULL;
    rtree_node *cur = tree->root;
    size_t i = 0;

    while(1){
        int idx = child_find(cur, key[i]);
        if(idx == -1)
            return NULL;
        rtree_node *child = cur->children[idx];

        if(strncmp(&key[i], child->prefix, child->prefix_len) != 0)
            return NULL;

        i += child->prefix_len;

        if(key[i] == '\0')
            return child->is_end? child : NULL;
        
        cur = child;
    }
}

// Добавление
static void child_insert_sorted(rtree_node *parent, rtree_node *child) {
    parent->children = realloc(parent->children, (parent->n_children + 1) * sizeof(rtree_node*));
    
    size_t pos = 0;
    while (pos < parent->n_children && parent->children[pos]->prefix[0] < child->prefix[0]) {
        pos++;
    }
    
    for (size_t i = parent->n_children; i > pos; i--) {
        parent->children[i] = parent->children[i - 1];
    }
    
    parent->children[pos] = child;
    parent->n_children++;
}

rtree *rtree_insert(rtree *tree, const char *key, uint32_t value) {
    if (!tree || !key || key[0] == '\0')
        return tree;

    rtree_node *cur = tree->root;
    size_t i = 0;

    while (1) {
        int idx = child_find(cur, key[i]);

        // Случай 1: Нету ребёнка
        if (idx == -1) {
            rtree_node *node = node_create(&key[i], strlen(&key[i]));
            node->is_end = true;
            node->value = value;
            child_insert_sorted(cur, node);
            tree->count++;
            return tree;
        }

        rtree_node *child = cur->children[idx];

        size_t match = 0;
        while (match < child->prefix_len && key[i + match] == child->prefix[match])
            match++;

        // Случай 2: Полностью совпал
        if (match == child->prefix_len) {
            i += match;
            if (key[i] == '\0') {
                if (!child->is_end)
                    tree->count++;
                child->is_end = true;
                child->value = value;
                return tree;
            }
            cur = child;
            continue;
        }

        // Случай 3: Частично совпал    
        rtree_node *mid = node_create(child->prefix, match);
        
        char *old = child->prefix;
        child->prefix = strdup(&old[match]);
        child->prefix_len -= match;
        free(old);

        child_insert_sorted(mid, child);

        if (key[i + match] == '\0') {
            mid->is_end = true;
            mid->value = value;
            tree->count++;
        } else {
            rtree_node *new_node = node_create(&key[i + match], strlen(&key[i + match]));
            new_node->is_end = true;
            new_node->value = value;
            child_insert_sorted(mid, new_node);
            tree->count++;
        }

        cur->children[idx] = mid;
        return tree;
    }
}


// Удаление
static void child_remove(rtree_node *node, int idx) {
    for (size_t i = idx; i < node->n_children - 1; i++)
        node->children[i] = node->children[i + 1];
    node->n_children--;
}

static void merge_with_child(rtree_node *node) {
    rtree_node *child = node->children[0];
    
    size_t new_len = node->prefix_len + child->prefix_len;
    char *new_pref = malloc(new_len + 1);
    strcpy(new_pref, node->prefix);
    strcat(new_pref, child->prefix);
    
    free(node->prefix);
    free(child->prefix);
    
    node->prefix = new_pref;
    node->prefix_len = new_len;
    node->is_end = child->is_end;
    node->value = child->value;
    
    free(node->children);
    node->children = child->children;
    node->n_children = child->n_children;
    node->children_cap = child->children_cap;
    
    free(child);
}

rtree *rtree_delete(rtree *tree, const char *key) {
    if (!tree || !key || key[0] == '\0')
        return tree;

    rtree_node *cur = tree->root;
    size_t i = 0;
    while (1) {
        int idx = child_find(cur, key[i]);
        if (idx == -1)
            return tree;

        rtree_node *child = cur->children[idx];
        
        if (strncmp(&key[i], child->prefix, child->prefix_len) != 0) 
            return tree;

        i += child->prefix_len;

        if (key[i] == '\0') {
            // Случай 0: Нету ключа
            if (!child->is_end)
                return tree;

            // Для случая 3: Когда больше 1 ребёнка
            child->is_end = false;
            tree->count--;

            // Случай 1: Нету детей
            if (child->n_children == 0) {
                child_remove(cur, idx);
                node_free(child);
            }
            // Случай 2: Есть 1 ребёнок
            else if (child->n_children == 1)
                merge_with_child(child);
            return tree;
        }
        cur = child;
    }
}


// Вывод древа
static void print_rec(rtree_node *node, int level) {
    if (!node)
        return;
    for (int i = 0; i < level; i++)
        printf("  ");
    
    printf("%s", node->prefix_len > 0 ? node->prefix : "*");
    if (node->is_end)
        printf(" (%u)", node->value);
    printf("\n");

    for (size_t i = 0; i < node->n_children; i++)
        print_rec(node->children[i], level + 1);
}

void rtree_print(rtree *tree, int level) {
    if (tree)
        print_rec(tree->root, level);
}