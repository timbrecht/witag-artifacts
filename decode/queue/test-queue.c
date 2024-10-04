#include <assert.h>
#include "queue.h"

// very simple assertion-based demo of a queue

int main() {
  struct queue *q = queue_create();
  queue_add_back(4, q);
  queue_add_back(20, q);
  queue_remove_front(q);
  assert(queue_front(q) == 20);
  assert(!queue_is_empty(q));
  queue_destroy(q);
}
