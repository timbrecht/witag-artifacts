#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ack-stats.h"
#include "cqueue.h"
// ----------------------------------------------------------------------
#define MIN(a, b) ((a < b) ? a : b)

struct ack_stats *ack_stats = 0;

struct ack_info {
  int bits;
	int zeros;
};

struct ack_stats {
  struct cqueue *ack_stats_cq;
  struct ack_info *window; 
  int window_size;
  int window_index;

  int *lengths;

	int cur_bits_total;
	int cur_zeros_total;

	int bits_total;
  int zeros_total;
  int ack_count;

  int max_bits;
};

// ----------------------------------------------------------------------
struct ack_stats *ack_stats_create(int max_bits, int window_size) 
{
	struct ack_stats *a = malloc(sizeof(struct ack_stats));
	assert(a);

  a->ack_stats_cq = cqueue_create(window_size);
  assert(a->ack_stats_cq);

	a->window_size = window_size;
  a->window = malloc(window_size * (sizeof(struct ack_info)));
  assert(a->window);

	a->window_index = 0;
  
	for (int i=0; i < window_size; i++) {
	  a->window[i].bits = -1;
	  a->window[i].zeros = -1;
	}
   
	a->lengths = malloc((max_bits + 1)  * sizeof(int));
	for (int i=0; i<max_bits; i++) {
	  a->lengths[i] = 0;
  }

	a->cur_bits_total = 0;
	a->cur_zeros_total = 0;

	a->bits_total = 0;
	a->zeros_total = 0;
	a->ack_count = 0;

	a->max_bits = max_bits;
  
	return a;
}


// ----------------------------------------------------------------------
void ack_stats_destroy(struct ack_stats *a) 
{
  assert(a);

  assert(a->window);
  free(a->window);

  assert(a->ack_stats_cq);
  cqueue_destroy(a->ack_stats_cq);

  assert(a->lengths);
  free(a->lengths);

  free(a);
}


// ----------------------------------------------------------------------
void ack_stats_update(struct ack_stats *a, int bits, int zeros) 
{
  assert(a);
	assert(bits > 0);
	assert(zeros >= 0);
	assert(bits <= a->max_bits);
	assert(zeros <= a->max_bits);
	assert(zeros <= bits);

  a->ack_count++;
	a->lengths[bits]++;

	// If adding to the queue will remove an item, adjust totals
  if (cqueue_items(a->ack_stats_cq) >= a->window_size) {
    struct ack_info *tmp_info = cqueue_front(a->ack_stats_cq);
		a->cur_bits_total -= tmp_info->bits;
		a->cur_zeros_total -= tmp_info->zeros;
	}

  assert(a->window_index >= 0);
  assert(a->window_index < a->window_size);

	a->window[a->window_index].bits = bits;
	a->window[a->window_index].zeros = zeros;
  a->cur_bits_total += bits;
	a->cur_zeros_total += zeros;
	a->bits_total += bits;
	a->zeros_total += zeros;
	cqueue_add(a->ack_stats_cq, &a->window[a->window_index]);

	a->window_index = (a->window_index + 1) % a->window_size;
}


// ----------------------------------------------------------------------
void ack_stats_print(struct ack_stats *a) 
{
  int count = MIN(cqueue_items(a->ack_stats_cq), a->window_size);
  printf("Ack Stats: Current avg ack len = %d  percent 0's %d%% | ", 
	    a->cur_bits_total / count, a->cur_zeros_total * 100 / a->cur_bits_total);
  printf("Overall avg ack len = %d  percent 0's %d%%\n", 
	    a->bits_total / a->ack_count, a->zeros_total * 100 / a->bits_total);
}

// ----------------------------------------------------------------------
void ack_stats_lengths_print(struct ack_stats *a) 
{
  printf("Ack length distributions:\n");
	for (int i=0; i <= a->max_bits; i++) {
	  if (a->lengths[i] != 0) {
		  printf("bits = %2d  count = %8d  percent = %.1lf\n", 
			        i, a->lengths[i], a->lengths[i] * 100.0 / a->ack_count);

		}
	}
}
