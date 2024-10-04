#include <stdbool.h>

// this is a queue module
// the documentation is conspicuously absent
// see the notes for more details

struct queue;

struct queue *queue_create(void);

void queue_add_back(int i, struct queue *q);

int queue_remove_front(struct queue *q);

int queue_front(const struct queue *q);

bool queue_is_empty(const struct queue *q);
  
void queue_destroy(struct queue *q);
