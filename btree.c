/** this file implements a b+ tree
*/
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define ORDER 20
#define LEAF_NODE 1
#define INTERNAL_NODE 0
#define SZ 100000

char map[SZ];

struct BTreeNode {
    int is_leaf;
    int n;
    unsigned* keys;
    struct BTreeNode* parent;
    struct BTreeNode** children;
    int height;
    struct BTreeNode* prev;
    struct BTreeNode* next;
};
unsigned int leaf_node_cnt = 0;
unsigned int internal_node_cnt = 0;

struct BTreeNode* create_btree_node(int is_leaf, struct BTreeNode* parent, int height) {
    struct BTreeNode* new_node = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));
    new_node->is_leaf = is_leaf;
    new_node->n = 0;
    new_node->height = height;
    // what if we allocate more?
    new_node->keys = (int*)malloc(sizeof(int) * (ORDER * 2 + 1));
    new_node->children = (struct BTreeNode**)malloc(sizeof(struct BTreeNode*) * ((ORDER * 2) + 2));
    new_node->parent = parent;
    if (is_leaf) {
        leaf_node_cnt++;
    } else {
        internal_node_cnt++;
    }
    new_node->prev = new_node->next = NULL;
    return new_node;
}

int is_full(struct BTreeNode* node) {
    if (node->n + 1 > (ORDER * 2)) {
        return 1;
    }
    return 0;
}

void print_node(struct BTreeNode* n) {
    printf("------------------------------\n");
    printf("addr: %p\n", n);
    printf("is_leaf: %s\n", n->is_leaf == 1 ? "true" : "false");
    printf("n = %d\n", n->n);
    printf("keys=");
    for (int i = 0; i < n->n; i++) {
        printf("%d ", n->keys[i]);
    }
    printf("\n");
    if (!n->is_leaf) {
        printf("children=");
        for (int i = 0; i < n->n + 1; i++) {
            printf("%p ", n->children[i]);
        }
    }
    printf("\n");
    printf("------------------------------\n");
}

void print_tree(struct BTreeNode* root) {
    int level = 0;
    printf("**********LEVEL:%d START***********\n", level);
    print_node(root);
    printf("**********LEVEL:%d END***********\n", level);
    level += 1;
    printf("**********LEVEL:%d START***********\n", level);
    for (int i = 0; i < root->n + 1; i++) {
        print_node(root->children[i]);
    }
    printf("**********LEVEL:%d END***********\n", level);
    level += 1;
    printf("**********LEVEL:%d START***********\n", level);
    for (int i = 0; i < root->n + 1; i++) {
        struct BTreeNode* child = root->children[i];
        for (int i = 0; i < child->n + 1; i++) {
            print_node(child->children[i]);
        }
    }
    printf("**********LEVEL:%d END***********\n", level);


}




void level_print_tree(struct BTreeNode* root) {
    struct Queue* q = create_queue(40960000);
    enqueue(q, (void*)root);
    while (queue_size(q)) {
        struct BTreeNode* node = (struct BTreeNode*)dequeue(q);
        print_node(node);
        if (node->is_leaf) {
            continue;
        }
        for (int i = 0; i < node->n + 1; i++) {
            enqueue(q, (void*)node->children[i]);
        }
    }
}
// need to optimize
void insert_to_leaves(struct BTreeNode* node, int key) {
    
    int i = node->n - 1;
    
    while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        i--;
    }
    node->keys[i + 1] = key;
    node->n += 1;
}

struct BTreeNode* traverse_down(struct BTreeNode* root, int key) {
    if (root->is_leaf) {
        return root;
    }
    int i = 0;
    while (i < (root->n) && root->keys[i] <= key) {
        i++;
    }
    return traverse_down(root->children[i], key);
}


// return index after which key should be inserted with total array kept sorted
int get_insert_pos(int* all, int lo, int hi, int key) {
    int i = hi - 1;
    while (i >= lo) {
        if (key < all[i]) {
            i--;
        } else {
            break;
        }
    }
    return i;
}

int array_insert(int* arr, int sz, int key) {
    int pos = get_insert_pos(arr, 0, sz, key);
    for (int i = sz - 1; i > pos; i--) {
        arr[i + 1] = arr[i];
    }
    arr[pos + 1] = key;
    return pos + 1;
}

void array_copy(int* from_arr, int from_lo, int* to_arr, int to_lo, int sz) {
    for (int i = 0; i < sz; i++) {
        to_arr[to_lo +i] = from_arr[from_lo + i];
    }
}

// insert a pointer after pos
void adjust_ptr(struct BTreeNode** ptr_arr, int sz, int pos, struct BTreeNode* node) {
    for (int i = sz - 1; i > pos; i--) {
        ptr_arr[i + 1] = ptr_arr[i];
    }
    ptr_arr[pos + 1] = node;
}

void copy_children(struct BTreeNode* from, int lo, struct BTreeNode* to, int sz) {
    for (int i = 0; i < sz; i++) {
        to->children[i] = from->children[lo + i];
    }
}
// 
struct BTreeNode* split_leaf_node_and_insert(struct BTreeNode* to_split, int key) {
    struct BTreeNode* sibling_node = create_btree_node(LEAF_NODE, to_split->parent, to_split->height);
    sibling_node->next = to_split->next;
    if (to_split->next) {
        to_split->next->prev = sibling_node;
    }
    to_split->next = sibling_node;
    sibling_node->prev = to_split;
    array_insert(to_split->keys, to_split->n, key);
    array_copy(to_split->keys, ORDER, sibling_node->keys, 0, ORDER + 1);
    sibling_node->n = ORDER + 1;
    to_split->n = ORDER;
    int copied_key = sibling_node->keys[0];
    // printf("copied key = %d\n", copied_key);

    struct BTreeNode* parent = to_split->parent;
    if (!parent) {
        parent = create_btree_node(INTERNAL_NODE, NULL, to_split->height + 1);
        parent->keys[parent->n] = copied_key;
        parent->n++;
        parent->children[0] = to_split;
        parent->children[1] = sibling_node;
        to_split->parent = parent;
        sibling_node->parent = parent;
        return parent;
    }
    int parent_overflow = parent->n == 2 * ORDER;
    int ptr_pos = array_insert(parent->keys, parent->n, copied_key);
    adjust_ptr(parent->children, parent->n + 1, ptr_pos, sibling_node);
    parent->n += 1;
    while (parent_overflow) {
        // split_internal
        // split parent to two nodes and get moved key
        // move key to parent's parent
        // print_node(parent);
        struct BTreeNode* parent_sibling = create_btree_node(INTERNAL_NODE, parent->parent, parent->height);
        int moved_key = parent->keys[ORDER];
        struct BTreeNode* promoted_child_ptr = parent->children[ORDER + 1];
        array_copy(parent->keys, ORDER + 1, parent_sibling->keys, 0, ORDER);
        copy_children(parent, ORDER + 1, parent_sibling, ORDER + 1);
        parent->n = ORDER;
        parent_sibling->n = ORDER;
        struct BTreeNode* pp = parent->parent;
        if (!pp) {
            pp = create_btree_node(INTERNAL_NODE, NULL, parent->height + 1);
            pp->keys[pp->n] = moved_key;
            pp->children[0] = parent;
            pp->children[1] = parent_sibling;
            pp->n += 1;
            parent->parent = parent_sibling->parent = pp;
            // print_node(pp);
            return pp;
        }
        parent_overflow = pp->n == 2 * ORDER;
        int pos = array_insert(pp->keys, pp->n, moved_key);
        // pp->children[pos + 1] = parent_sibling;
        adjust_ptr(pp->children, pp->n + 1, pos, parent_sibling);
        pp->n += 1;
        parent = pp;
    }
    // adjust pointer
    while (parent->parent) {
        parent = parent->parent;
    }
    return parent;
    // now spill to parent
}

void split_internal_node(struct BTreeNode* to_split) {

}
// return new root of the btree
struct BTreeNode* insert(struct BTreeNode* root, int key) {
    struct BTreeNode* leaf_node = traverse_down(root, key);
    
    if (is_full(leaf_node)) {
        // should split leaf_node to two leaf_node and copy key to parent
        return split_leaf_node_and_insert(leaf_node, key);
    } else {
        insert_to_leaves(leaf_node, key);
        return root;
    }  
}

void free_node(struct BTreeNode* node) {
    free(node->keys);
    free(node->children);
    free(node);
}

struct BTreeNode* get_first_node(struct BTreeNode* root) {
    struct BTreeNode* iter = root;
    while (!iter->is_leaf) {
        iter = (iter->children)[0];
    }
    return iter;
}
int main() {
    struct BTreeNode* root = create_btree_node(1, NULL, 0);
    for (int i = 0; i < SZ; i++) {
        unsigned key = rand() % SZ;
        if (map[key]) {
            continue;
        } else {
            map[key] = 1;
            root = insert(root, key);
        }
    }
    // level_print_tree(root);
    printf("height : %d\n", root->height);
    printf("leaf node: %u, internal node: %u\n", leaf_node_cnt, internal_node_cnt);
    struct BTreeNode* first = get_first_node(root);
    int leaf_cnt = 0;
    int min = (first->keys)[0];
    while (first) {
        if ((first->keys)[0] < min) {
            printf("error!\n");
            break;
        }
        leaf_cnt++;
        first = first->next;
    }
    printf("\n");
    printf("leaf_cnt = %d\n", leaf_cnt);
}