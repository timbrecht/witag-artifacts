#ifndef __window_h__
#define __window_h__

#include <stdio.h>
#include "match_info.h"
#include "cqueue.h"

// ----------------------------------------------------------------------
#define MAX_WINDOW_SIZE    (100000)
// #define MAX_PACKET_WINDOW_SIZE    (100)
#define STATUS_INVALID       (0)
#define STATUS_MATCH        (-1)
#define STATUS_NO_MATCH     (-2)
#define STATUS_UNUSED       (-3)
#define STATUS_NOT_PACKET   (-4)

// ----------------------------------------------------------------------
extern struct ack_stats *ack_stats;
extern struct match_info window[MAX_WINDOW_SIZE];
// Not sure if this needs to be global or now (it may need to be)
extern int windex;

extern struct match_info *packet_window;
extern int packet_windex;

int find_window_slot(struct match_info *w);

// Add info to window and return the location
void window_add(struct match_info *w, int index, int value,
  int status, bool is_packet, int ack_num);

void process_saved_packets(struct cqueue *cq, char *expected_str, FILE *outfp);

#endif
