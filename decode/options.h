#ifndef __options_h__
#define __options_h__

#include <stdio.h>
#include <limits.h>

#define MIN_ZEROS_DEFAULT         (4) // TODO: FIXME this needs to change with P
#define DEFAULT_MPDUS_PER_AMPDU  (32)

// ----------------------------------------------------------------------
extern int bits_per_packet;
extern int mpdus_per_bit;
extern int preamble_mpdus;
extern int expected_value;
extern int min_zeros;
extern int gap_thold;
extern int rate_gap_thold;
extern int filter_and_print_packets;
extern int quiet_mode;
extern int safe_mode;

extern int consecutive_zeros;
extern int ampdu_greater_than;

extern double factor;
extern double offset;

extern int too_short_len;
extern int filter_using_estimates;

extern int algorithm;
extern int expected_in_file;
extern int exact_lev_len;

extern int sensor_type;
extern int sensor0_type;
extern int sensor1_type;

extern char outfile1[PATH_MAX];
extern char infile[PATH_MAX];
extern char **outfiles;
extern FILE **outfps;
extern int outfile_count;

extern int zero_thold;
extern int amble_filter;

extern int delay;
extern int all_of_file;
extern int packet_filter;
extern int num_consecutive;

extern int min_packet_count;

extern int verbose;
extern int expected_num;

extern int regex_index;

extern FILE *infp;

extern int overlap_packet_count;
extern int max_mpdus_p_ampdu;

extern int plen_short_by;
extern int plen_long_by;

extern int wait_time;

extern int preamble_index;
extern int extra_ones_added;
extern int print_bit_error_rate;
#endif
