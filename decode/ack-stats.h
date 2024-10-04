#ifndef __ack_stats_h__
#define __ack_stats_h__

struct ack_stats;

void ack_stats_update(struct ack_stats *a, int len, int zeros);
struct ack_stats *ack_stats_create(int max_bits, int window_size);
void ack_stats_destroy(struct ack_stats *a);
void ack_stats_print(struct ack_stats *a);
void ack_stats_lengths_print(struct ack_stats *a);

#endif
