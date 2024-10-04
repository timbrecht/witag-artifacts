#ifndef __cqueue_h__
#define __cqueue_h__
#include <stdbool.h>

// this is a circular queue module

struct cqueue {
  void **items;
  int front;
  int rear;
  int size;
};


struct cqueue *cqueue_create(int size);

// Add element to the back of the queue
void cqueue_add(struct cqueue *cq, void *element);

// Returns the number of items in the queue
int cqueue_items(struct cqueue *cq);

// Removes the entry at the front of the queue
void *cqueue_remove(struct cqueue *cq);

// Returns a pointer to the entry at the front of the queue
void *cqueue_front(const struct cqueue *cq);

bool cqueue_is_empty(const struct cqueue *cq);
  
void cqueue_destroy(struct cqueue *cq);

#endif
