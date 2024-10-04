#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "queue.h"

// See the interface for details

struct llnode {
  int item;
  struct llnode *next;
};


struct queue {
  struct llnode *front;
  struct llnode *back;    // <--- NEW
};


struct queue *queue_create(void) {
  struct queue *q = malloc(sizeof(struct queue));
  q->front = NULL;
  q->back = NULL;
  return q;
}


struct llnode *new_node(int i, struct llnode *pnext) {
  struct llnode *node = malloc(sizeof(struct llnode));
  node->item = i;
  node->next = pnext;
  return node;
}


void queue_add_back(int i, struct queue *q) {
  assert(q);
  struct llnode *node = new_node(i, NULL);
  if (q->front == NULL) {
    q->front = node;
  } else {
    q->back->next = node;
  }
  q->back = node;
}


int queue_remove_front(struct queue *q) {
  assert(q);
  assert(q->front);
  int retval = q->front->item;
  struct llnode *old_front = q->front;
  q->front = q->front->next;
  free(old_front);
  if (q->front == NULL) {
    q->back = NULL;
  }
  return retval;
}


int queue_front(const struct queue *q) {
  assert(q);
  assert(q->front);
  return q->front->item;
}


bool queue_is_empty(const struct queue *q) {
  assert(q);
  return q->front == NULL;
}


void queue_destroy(struct queue *q) {
  assert(q);
  while (!queue_is_empty(q)) {
    queue_remove_front(q);
  }
  free(q);
}
