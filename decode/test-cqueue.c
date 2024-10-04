#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "cqueue.h"

static const bool dbg = false;

// very simple assertion-based demo of a cqueue

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) 
{
	int data[] = {100, 200, 300, 400, 500};
	int *value = 0;
	int size = 5;

  struct cqueue *cq = cqueue_create(size);
	assert(cqueue_items(cq) == 0);

	for (int i=0; i<size; i++) {
	  if (dbg) printf("&data[i] = %p\n", &data[i]);
	}
	printf("\n");

  cqueue_add(cq, &data[0]);
	assert(cqueue_items(cq) == 1);
  value = cqueue_remove(cq);
	if (dbg) printf("value = %p\n", value);
	assert(value == &data[0]);
	assert(cqueue_items(cq) == 0);
	printf("value = %d\n", *value);
	assert(*value == 100);

	for (int i=0; i<size; i++) {
    cqueue_add(cq, &data[i]);
	}
	assert(cqueue_items(cq) == 5);

	for (int i=0; i<size; i++) {
    cqueue_add(cq, &data[i]);
	}
	assert(cqueue_items(cq) == 5);

	for (int i=0; i<size; i++) {
    value = cqueue_remove(cq);
	  assert(value == &data[i]);
	  assert(*value == data[i]);
	}
	assert(cqueue_items(cq) == 0);

  cqueue_destroy(cq);
}
