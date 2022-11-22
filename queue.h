#ifndef QUEUE
#define QUEUE
#include <stdlib.h>
#include <assert.h>

struct Queue {
    void** ele;
    size_t lo, hi;
};


struct Queue* create_queue(size_t sz) {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->lo = q->hi = 0;
    q->ele = (void**)malloc(sizeof(void*) * sz);
}
void enqueue(struct Queue* q, void* ele) {
    q->ele[q->hi] = ele;
    q->hi += 1;
}
void* dequeue(struct Queue* q) {
    assert(q->lo < q->hi);
    void* ele = q->ele[q->lo];
    q->lo += 1;
    return ele;
}

size_t queue_size(struct Queue* q) {
    return q->hi - q->lo;
}
#endif