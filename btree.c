/** this file implements a b+ tree
*/
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define ORDER 3
#define LEAF_NODE 1
#define INTERNAL_NODE 0

struct BTreeNode {
    int is_leaf;
    int n;
    int* keys;
    struct BTreeNode* parent;
    struct BTreeNode** children;
};


struct BTreeNode* create_btree_node(int is_leaf, struct BTreeNode* parent) {
    struct BTreeNode* new_node = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));
    new_node->is_leaf = is_leaf;
    new_node->n = 0;

    // what if we allocate more?
    new_node->keys = (int*)malloc(sizeof(int) * (ORDER * 2 + 1));
    new_node->children = (struct BTreeNode**)malloc(sizeof(struct BTreeNode*) * ((ORDER * 2) + 2));
    new_node->parent = parent;
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
    struct Queue* q = create_queue(4096);
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
    struct BTreeNode* sibling_node = create_btree_node(LEAF_NODE, to_split->parent);
    array_insert(to_split->keys, to_split->n, key);
    array_copy(to_split->keys, ORDER, sibling_node->keys, 0, ORDER + 1);
    sibling_node->n = ORDER + 1;
    to_split->n = ORDER;
    int copied_key = sibling_node->keys[0];
    printf("copied key = %d\n", copied_key);

    struct BTreeNode* parent = to_split->parent;
    if (!parent) {
        parent = create_btree_node(INTERNAL_NODE, NULL);
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
        // split_internaal
        // split parent to two nodes and get moved key
        // move key to parent's parent
        struct BTreeNode* parent_sibling = create_btree_node(INTERNAL_NODE, parent->parent);
        int moved_key = parent->keys[ORDER];
        struct BTreeNode* promoted_child_ptr = parent->children[ORDER + 1];
        array_copy(parent->keys, ORDER + 1, parent_sibling->keys, 0, ORDER);
        copy_children(parent, ORDER + 1, parent_sibling, ORDER + 1);
        parent->n = ORDER;
        parent_sibling->n = ORDER;
        struct BTreeNode* pp = parent->parent;
        if (!pp) {
            pp = create_btree_node(INTERNAL_NODE, NULL);
            pp->keys[pp->n] = moved_key;
            pp->children[0] = parent;
            pp->children[1] = parent_sibling;
            pp->n += 1;
            parent->parent = parent_sibling->parent = pp;
            return pp;
        }
        parent_overflow = pp->n == 2 * ORDER;
        int pos = array_insert(pp->keys, pp->n, moved_key);
        pp->children[pos + 1] = parent_sibling;
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

int main() {
    struct BTreeNode* root = create_btree_node(1, NULL);
    for (int i = 0; i < 50; i++) {
        root = insert(root, 0 - i);
    }
    level_print_tree(root);
}