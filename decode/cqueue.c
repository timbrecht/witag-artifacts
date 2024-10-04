#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cqueue.h"

// ----------------------------------------------------------------------
#define INVALID (-1)

static const bool dbg = false;

// ----------------------------------------------------------------------
void cqueue_destroy(struct cqueue *cq) 
{
	if (dbg) printf("destroy: front = %d rear = %d\n", cq->front, cq->rear);
   assert(cq);
   assert(cq->items);
   free(cq->items);
   free(cq);
   cq = NULL;
}

// ----------------------------------------------------------------------
struct cqueue *cqueue_create(int size) 
{
   struct cqueue *cq = malloc(sizeof(struct cqueue)); 
	 assert(cq);
   cq->items = malloc(sizeof(void *) * size);
	 assert(cq->items);
	 for (int i=0; i<size; i++) {
	   cq->items[i] = NULL;
	 }
	 cq->size = size;
	 cq->front = INVALID;
	 cq->rear = INVALID;
	 return cq;
}

// ----------------------------------------------------------------------
int cqueue_items(struct cqueue *cq) 
{
  assert(cq);
  int count = 0;

	if (cq->rear == INVALID) {
	   assert(cq->front == INVALID);
		 count = 0;
	} else if (cq->rear >= cq->front) {
    count = cq->rear - cq->front + 1;
	} else {
	  count = ((cq->size - cq->front) + cq->rear + 1);
	}

	if (dbg) printf("items: front = %d rear = %d count = %d\n", cq->front, cq->rear, count);

  return count;
}

// ----------------------------------------------------------------------
void cqueue_add(struct cqueue *cq, void *element) 
{
	 // void *tmp = 0;
   assert(cq);
	 assert(element);
   assert(cqueue_items(cq) <= cq->size);
	 bool first_item = false;

	 if (cqueue_items(cq) == 0) {
     first_item = true;
	 }

   if (cqueue_items(cq) < cq->size) {
	   cq->rear = (cq->rear + 1) % cq->size;
	   cq->items[cq->rear] = element;
	 } else {
	   cqueue_remove(cq);
     assert(cqueue_items(cq) < cq->size);
	   cq->rear = (cq->rear + 1) % cq->size;
	   cq->items[cq->rear] = element;
	 }

	 if (first_item) {
     cq->front++;
	 }

	if (dbg) printf("add: front = %d rear = %d element = %p\n", cq->front, cq->rear, element);
}

// ----------------------------------------------------------------------
// Returns a pointer to the entry at the front of the queue
void *cqueue_front(const struct cqueue *cq)
{
  assert(cq);
	assert(cq->items[cq->front]);
	return cq->items[cq->front];
}

// ----------------------------------------------------------------------
void *cqueue_remove(struct cqueue *cq)
{
  bool last_item = false;

  assert(cq);
  assert(cqueue_items(cq) != 0);

  if (cqueue_items(cq) == 1) {
	  last_item = true;
	}

	if (dbg) printf("remove: front = %d rear = %d last_item = %d\n", cq->front, cq->rear, last_item);
  void *tmp = cq->items[cq->front];
  cq->items[cq->front] = NULL;
  if (dbg) printf("remove: front = %d rear = %d tmp = %p\n", cq->front, cq->rear, tmp);

  if (last_item == true) {
    cq->rear = INVALID;
    cq->front = INVALID;
	} else {
    cq->front = (cq->front + 1) % cq->size;
	}
  return tmp;
}

// ----------------------------------------------------------------------
// TODO: Fill the rest of this in
void cqueue_count(struct cqueue *cq, int counts[], int len)
{
  for (int i=0; i<cq->size; i++) {
	  assert(cq->items[i]);
	  for (int j=0; j<len; j++) {
		  if (cq->items[i]) {
			  counts[j]++;
		  }
	  }
  }
}

