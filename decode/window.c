#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "options.h"
#include "match_info.h"
#include "globals.h"
#include "window.h"
#include "expected.h"
#include "sensors.h"
#include "cqueue.h"

// ----------------------------------------------------------------------
const bool dbg = false;
const bool dbg_freq = false;



// ----------------------------------------------------------------------
struct match_info window[MAX_WINDOW_SIZE] = {0};
// TODO: Not sure if this needs to be global or not (it may need to be)
int windex = 0;

// struct match_info packet_window[MAX_PACKET_WINDOW_SIZE] = {0};
struct match_info *packet_window = 0;
int packet_windex = 0;

static int num_overlap_window_checks = 0;
static int most_freq_correct_count = 0;

// ----------------------------------------------------------------------
int find_window_slot(struct match_info *w)
{
  static int current = -1;

  current = (current + 1) % MAX_WINDOW_SIZE;
  return current;
}

// ----------------------------------------------------------------------
// Add info to window and return the location
void window_add(struct match_info *w, int index, int value,
  int status, bool is_packet, int ack_num) 
{
  // initialize all of the info / entries for the window
  // because these get reused/recycled.
  struct match_info *m = &w[index];
  m->status = STATUS_INVALID;
  m->is_packet = false;
  m->ack_number = -1;
  for (int i=0; i<num_values; i++) {
    m->match[i] = false;
  }

  w[index].ack_number = ack_num;

  switch (status) {
    case STATUS_MATCH:
      w[index].status = STATUS_MATCH;
      w[index].match[value] = true;
      w[index].is_packet = is_packet;
      break;

    case STATUS_NO_MATCH:
      if (w[index].status != STATUS_MATCH) {
        w[index].status = STATUS_NO_MATCH;
      }
      w[index].match[value] = false;
      w[index].is_packet = is_packet;
      break;

    case STATUS_NOT_PACKET:
      w[index].status = status;
      for (int i=0; i<num_values; i++) {
        w[index].match[i] = false;
      }
      w[index].is_packet = false;
      break;

    default:
      fprintf(stderr, "window_add: Unhandled status = %d\n", status);
      exit(1);
      break;
  }
} /* window_add */

// ----------------------------------------------------------------------
// This is designed specifically for the packet_cq
void process_saved_packets(struct cqueue *cq, char *expected_str, FILE *outfp)
{
  int max_freq = 0;
  int most_freq_value = VALUE_INVALID;
  int freq[MAX_VALUES] = {0};

  int overlap_ack_diff = 0;

  int min_ack_number = INT_MAX;
  int max_ack_number = INT_MIN;
  bool most_freq_matched = false;

  if (cqueue_items(cq) < overlap_packet_count) {
    if (!quiet_mode) {
      printf("Not enough items in packet_cq = %d need %d\n", cqueue_items(cq), overlap_packet_count);
    }
    return;
  }

  assert(cqueue_items(cq) == overlap_packet_count);

  for (int i=0; i<overlap_packet_count; i++) {
    struct match_info *m = (struct match_info *) cq->items[i];
    if (dbg) {
      printf("i = %d\n", i);
      printf("m->is_packet = %d\n", m->is_packet);
      printf("m->status = %d\n", m->status);
      printf("m->ack_number = %d\n", m->ack_number);
    }
    assert(m);
    if (!m->is_packet) {
      printf("m->is_packet = %d\n", m->is_packet);
      printf("m->status = %d\n", m->status);
      printf("m->ack_number = %d\n", m->ack_number);
    }
    assert(m->is_packet);

    assert(m->ack_number != 0);
    // num_window_checks++;

    if (m->status == STATUS_MATCH) {

      if (m->ack_number < min_ack_number) {
         min_ack_number = m->ack_number;
      }

      if (m->ack_number > max_ack_number) {
         max_ack_number = m->ack_number;
      }

      for (int j=0; j<num_values; j++) {
        if (dbg) {
          printf("j = %d m->match = %d\n", j, m->match[j]);
        }
        if (m->match[j] == true) {
          freq[j]++;
          if (freq[j] > max_freq) {
           max_freq = freq[j];
           most_freq_value = j;
          }
        } /* if */
      } /* for */
    } /* if */
  } /* for */

  num_overlap_window_checks++;

  overlap_ack_diff = max_ack_number - min_ack_number;

  if ((most_freq_value != VALUE_INVALID) && 
      is_expected(most_freq_value, expected_str)) {
    most_freq_correct_count++;
    most_freq_matched = true;
  } /* if */

  if (verbose) {
    printf("%d: overlap packet max - min = %d\n", line_count, overlap_ack_diff);
    printf("%d: overlap packet most freq = %d found %d times in %d packets:(%d %%) of frames %s\n",
        line_count, most_freq_value, max_freq, overlap_packet_count,
        (max_freq * 100 / overlap_packet_count),
        (most_freq_matched ? "Correct" : "Incorrect"));
    if (!most_freq_matched) {
       if (dbg_freq) {
         for (int k=0; k<num_values; k++) {
           printf("%d:%d, ", k, freq[k]);
         }
         printf("\n");
       }
    }
  } /* if */

  if (safe_mode && (most_freq_value == VALUE_INVALID)) {
    // Do nothing
  } else {
    if (outfile1[0] != '\0') {
     write_value(outfps, most_freq_value);
    }
  }

  if (verbose) {
    printf("%8d: Most freq correct = %8d out of %8d = %6.6lf%% ",
       line_count, most_freq_correct_count, num_overlap_window_checks,
       most_freq_correct_count * 100.0 / num_overlap_window_checks);

    double avg_acks_between_packets = ((double) overlap_ack_diff / overlap_packet_count);
    printf("avg acks between packets = %5.1lf diff = %d\n", avg_acks_between_packets, overlap_ack_diff);
#if 0
    if (avg_acks_between_packets < min_packets_per_ack_diff) {
      min_packets_per_ack_diff = avg_acks_between_packets;
    } /* if */
#endif
  } /* if */
}
