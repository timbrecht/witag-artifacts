#ifndef __globals_h__
#define __globals_h__

#include "decode.h"
#include "window.h"

extern int line_count;
extern int len_count[AMPDU_LEN];
extern int correct_by_len[AMPDU_LEN];
extern int ack_stats_window;
extern int one_or_more_zeros;
extern char **expected_str;
extern int expected_len;
extern struct ack_stats *ack_stats;
extern int num_values;

extern int zero_locations_not_counted;

// Kamrans (dt = delta?)
extern int dt_per_packet_count;
extern double t_packet_rate;
extern double Exp_tag_pkt;

extern int error_counts[MAX_VALUES];
extern int match_counts[MAX_VALUES];
extern int total_matches;
extern int counts[MAX_VALUES];
extern int regex_index;
extern char **lev_strings;

// So we can close it outside of the function that opens it
extern FILE *outfp;

extern int estimated_ampdu_len;
extern int num_ties[MAX_VALUES];
extern int total_edit_dist[MAX_VALUES];
#endif
