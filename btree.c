/** this file implements a b+ tree
*/
#include <stdio.h>
#include <stdlib.h>

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
    new_node->keys = (int*)malloc(sizeof(int) * (ORDER * 2));
    new_node->children = (struct BTreeNode**)malloc(sizeof(struct BTreeNode*) * ((ORDER * 2) + 1));
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
    printf("------------------------------\n");
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
    while (i < (root->n + 1) && root->keys[i] <= key) {
        i++;
    }
    return traverse_down(root->children[i], key);
}



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

void split_leaf_node_and_insert(struct BTreeNode* to_split, int key) {
    struct BTreeNode* sibling_node = create_btree_node(LEAF_NODE, to_split->parent);
    int insert_pos = get_insert_pos(to_split->keys, 0, to_split->n, key);
    int* tmp = (int*)malloc(sizeof(int) * (2 * ORDER + 1));
    for (int i = 0, j = 0; i < (2 * ORDER + 1); i++) {
        if (i == insert_pos + 1) {
            tmp[i] = key;
        } else {
            tmp[i] = to_split->keys[j];
            j++;
        }
    }
    for (int i = ORDER + 1, j = 0; i < (2 * ORDER + 1); i++, j++) {
        sibling_node->keys[j] = tmp[i];
    }
    for (int i = 0, j = 0; i <= ORDER; i++, j++) {
        to_split->keys[j] = tmp[i];
    }
    sibling_node->n = ORDER;
    to_split->n = ORDER + 1;
    int copied_key = sibling_node->keys[0];
    printf("copied key = %d\n", copied_key);
    print_node(sibling_node);

    // now spill to parent
    struct BTreeNode* parent = to_split->parent;
    if (!parent) {
        parent = create_btree_node(INTERNAL_NODE, NULL);
    }

    insert_pos = get_insert_pos(parent->keys, 0, parent->n, copied_key);

}

void split_internal_node(struct BTreeNode* to_split) {

}
// return new root of the btree
struct BTreeNode* insert(struct BTreeNode* root, int key) {
    struct BTreeNode* leaf_node = traverse_down(root, key);
    
    if (is_full(leaf_node)) {
        // should split leaf_node to two leaf_node and copy key to parent
        split_leaf_node_and_insert(leaf_node, key);
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
    print_node(root);
    insert(root, 7);
    print_node(root);
    insert(root, 6);
    print_node(root);
    insert(root, 5);
    print_node(root);
    insert(root, 4);
    print_node(root);
    insert(root, 3);
    print_node(root);
    insert(root, 2);
    print_node(root);

    insert(root, 1);
    print_node(root);
    free_node(root);
}