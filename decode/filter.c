#include <stdbool.h>
#include <stdio.h>
#include <regex.h>
#include <assert.h>
#include <string.h>
#include "regexes.h"
#include "options.h"
#include "globals.h"
#include "ack-stats.h"
#include "utils.h"
#include "filter.h"
#include "interval-stats.h"
// ----------------------------------------------------------------------

#define MIN_BITMAP_LEN          (4)

#define INDEX_INVALID           (-1)

static const bool dbg = false;
static const bool dbg_pkt = false;
static const bool dbg_pre_post = false;
static const bool dbg_location = false;
static const bool dbg_possible_len = false;
static const bool where_are_lone_preambles = true;


// Filter stats (why are block acks not considered a packet
static int filter_len = 0;
static int filter_excess_zeros = 0;
static int filter_no_zeros = 0;
static int filter_no_ones = 0;
static int filter_insufficient_zeros = 0;
static int filter_amble = 0;
static int filter_packet_len = 0;
static int filter_regexes = 0;

void count_bits(char *str, int *zeros, int *ones);
bool is_packet_len_valid(int plen);
bool too_many_leading_trailing_zeros(char *bitmap, int amble_filter_thold);
char *preamble_start(char *bitmap);
char *postamble_end(char *bitmap);

static int preamble_locations[AMPDU_LEN+1] = {0};
static int possible_msg_lens[AMPDU_LEN+1] = {0};
static char preamble_zeros[AMPDU_LEN] = {'\0'};
static char postamble_zeros[AMPDU_LEN] = {'\0'};
static int possible_count = 0;

static int zeros_locations[AMPDU_LEN+1] = {0};

static int zeros_histo[AMPDU_LEN+1] = {0};
static int ones_histo[AMPDU_LEN+1] = {0};
static double zeros_total = 0;
static double ones_total = 0;

int preamble_location(int preamble_mpdus, char *bitmap) {
  int index = INDEX_INVALID;

  if (preamble_zeros[0] == '\0') {
    int i = 0;
    for (i=0; i<preamble_mpdus; i++) {
      preamble_zeros[i] = '0';
    }
    preamble_zeros[i] = '\0';
    if (dbg_location) printf("preamble_zeros = [%s]\n", preamble_zeros);
  }

  char *p = strstr(bitmap, preamble_zeros);
  if (dbg_location) printf("bitmap = %p [%s]\n", (void *) bitmap, bitmap);
  if (dbg_location) printf("p = %p\n", (void *) p);

  if (p == NULL) {
    index = AMPDU_LEN;
  } else {
    index = p - bitmap;
  }

  if (dbg_location) printf("index = %d\n", index);
  return index;
}

int postamble_location(int preamble_mpdus, char *bitmap) {
  int index = INDEX_INVALID;

  if (postamble_zeros[0] == '\0') {
    int i = 0;
    for (i=0; i<preamble_mpdus; i++) {
      postamble_zeros[i] = '0';
    }
    postamble_zeros[i] = '\0';
    if (dbg_location) printf("postamble_zeros = [%s]\n", postamble_zeros);
  }

  int cur = strlen(bitmap);
  int zero_count = 0;

  if (dbg_location) printf("A: cur = %d zero_count = %d\n", cur, zero_count);
  while (cur >= 0) {
     if (bitmap[cur] == '0') {
       zero_count++;
       if (dbg_location) printf("B: cur = %d zero_count = %d\n", cur, zero_count);
     } else {
       zero_count = 0;
     }

     if (zero_count == preamble_mpdus) {
       break;
     }

     cur--;
     if (dbg_location) printf("C: cur = %d zero_count = %d\n", cur, zero_count);
  }

  if (dbg_location) printf("D: cur = %d zero_count = %d\n", cur, zero_count);

  if (zero_count == preamble_mpdus) {
    index = cur + strlen(postamble_zeros) - 1;
    if (dbg_location) printf("cur = %d strlen = %d bitmap = [%s]\n", cur, (int) strlen(postamble_zeros), bitmap);
  } else {
    index = AMPDU_LEN;
  }

  if (dbg_location) printf("bitmap = [%s]\n", bitmap);
  if (dbg_location) printf("index = %d\n", index);
  return index;
}

void analyze_preamble_distributions(int preamble_mpdus, char *bitmap) {

	int pre_index = preamble_location(preamble_mpdus, bitmap);
	assert(pre_index != INDEX_INVALID);
	assert(pre_index >= 0);
	assert(pre_index <= AMPDU_LEN);
	preamble_locations[pre_index]++;

	int post_index = postamble_location(preamble_mpdus, bitmap);
  if (dbg_location) printf("post_index = %d\n", post_index);
	assert(post_index != INDEX_INVALID);
	assert(post_index >= 0);
	assert(post_index <= AMPDU_LEN);

  if (pre_index != AMPDU_LEN) {
		// bitmap: 10001110001
		// index:  01234567890
		// pre_index = 1
		// post_index = 9
		// 9 - 1 + 1 + 
		int dist = post_index - pre_index + 1;
		// Could add 2 for the fence posts (but this stat doesn't count fenceposts)
		int possible_len = dist;
	  assert(possible_len >= 0);
	  assert(possible_len <= AMPDU_LEN);
		possible_msg_lens[possible_len]++;
    possible_count++;

    if (where_are_lone_preambles) {
#ifdef OLDWAY
       if (preamble_mpdus == possible_len ||
           preamble_mpdus + 1 == possible_len) {
          printf("PREAMBLE at %2d possible_len = %2d est_len = %2d in bitmap = [%s]\n",
                  pre_index, possible_len, estimated_ampdu_len, bitmap);
       }
#else
       // There are other cases we want to be able to look at.
       printf("PREAMBLE at %2d possible_len = %2d est_len = %2d in bitmap = [%s] on line = %d\n",
                  pre_index, possible_len, estimated_ampdu_len, bitmap, line_count);
#endif
    }

    if (possible_len == AMPDU_LEN) {
       if (dbg_possible_len) printf("PL: line_count = %d post_index = %2d pre_index = %2d bitmap = [%s]\n",
               line_count, post_index, pre_index, bitmap);
    }
  }

}

void find_first_set_of_zeros(int num_zeros, char *bitmap) {
	int pre_index = preamble_location(num_zeros, bitmap);
	assert(pre_index != INDEX_INVALID);
	assert(pre_index >= 0);
	assert(pre_index <= AMPDU_LEN);

  if (estimated_ampdu_len >= ampdu_greater_than) {
	  zeros_locations[pre_index]++;
  } else {
	  zero_locations_not_counted++;
  }
}


void test_preamble_locations(void) {
  assert(preamble_location(3, "00011111111") == 0);
  assert(preamble_location(2, "00011111111") == 0);
  assert(preamble_location(3, "100011111111") == 1);
  assert(preamble_location(3, "111111111") == AMPDU_LEN);
  assert(preamble_location(3, "11111111000") == 8);
  assert(preamble_location(2, "1111111100") == 8);
  assert(preamble_location(3, "111111110001111") == 8);
  assert(preamble_location(5, "111111110001111") == AMPDU_LEN);

  assert(postamble_location(3, "00011111111") == 2);
  assert(postamble_location(2, "00011111111") == 2);
  assert(postamble_location(3, "100011111111") == 3);
  assert(postamble_location(3, "111111111") == AMPDU_LEN);
  assert(postamble_location(3, "11111111000") == 10);
  assert(postamble_location(2, "1111111100") == 9);
  assert(postamble_location(3, "111111110001111") == 10);
  assert(postamble_location(5, "111111110001111") == AMPDU_LEN);
}

bool is_too_short() {
  if (estimated_ampdu_len == 0) {
    return false;
  }

  if (estimated_ampdu_len <  too_short_len) {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/* Returns the number of matches */
int is_packet(char *bitmap, regex_t r[], int count, int zero_thold,
       int amble_filter, int line_num, int expected_num, char *packet_str,
       char *packet_regexes[])
{
  int retval = 0;
  int match_count = 0;
  int zeros = 0;
  int ones = 0;
  int len = strlen(bitmap);
  // char packet_str[MAX_LEN] = {0};
  char *p = 0;
  regmatch_t  pmatch[1];
  regoff_t plen = 0;
  int j = 0;
  int bits_counted = 0;

  if (preamble_index) {
    analyze_preamble_distributions(preamble_mpdus, bitmap);
  }

  if (consecutive_zeros) {
    find_first_set_of_zeros(consecutive_zeros, bitmap);
  }

  if (ack_stats_window) {
    count_bits(bitmap, &zeros, &ones);
    bits_counted = 1;
    ack_stats_update(ack_stats, len, zeros);
  }

  if (dbg_pkt) printf("is_packet: line = %d bitmap = %s\n", line_num, bitmap);
  // if (dbg_pkt) printf("is_packet: len = %d MIN_BITMAP_LEN = %d\n", len, MIN_BITMAP_LEN);

  if (filter_using_estimates && is_too_short()) {
    filter_len++;
    return 0;
  }

  if (len < MIN_BITMAP_LEN) {
    if (dbg_pkt) printf("is_packet: len = %d < MIN_BITMAP_LEN = %d\n", len, MIN_BITMAP_LEN);
    filter_len++;
    return 0;
  }

  if (!bits_counted) {
    count_bits(bitmap, &zeros, &ones);
    bits_counted = 1;
  }

  assert(zeros >= 0);
  assert(zeros <= AMPDU_LEN);
  assert(ones >= 0);
  assert(ones <= AMPDU_LEN);

  zeros_histo[zeros]++;
  zeros_total += (double) zeros;
  ones_histo[ones]++;
  ones_total += (double) ones;


  // Just count these
  if (zeros > 0) {
    one_or_more_zeros++;
  }

  if (zeros == 0) {
     filter_no_zeros++;
     return 0;
  }

  if (ones == 0) {
     filter_no_ones++;
     return 0;
  }


  // Are there enough zeros to consider this a packet (should have some pre and post amble zeros)
  if (zeros < min_zeros) {
    if (dbg_pkt) printf("is_packet: zeros = %d < min_zeros = %d\n", zeros, min_zeros);
    filter_insufficient_zeros++;
    return 0;
  }

  // Extra new filter. Some packets have lots of 0's and that causes problems.
  if (zeros > zero_thold) {
    if (dbg_pkt) printf("is_packet: zeros = %d > zero_thold = %d\n", zeros, zero_thold);
    filter_excess_zeros++;
    return 0;
  }

  int no_match = 0;
  for (int i=0; i<count; i++) {
    if (dbg) printf("is_packet: calling regexec i = %d\n", i);
    fflush(stdout);
    retval = regexec(&r[i], bitmap, 1, pmatch, 0);

    if (retval == REG_MATCH) {
      // This can only be done after we determine the packet has pre and post ambles
      // If there are too many zeros before the preamble or after the preamble
      dt_per_packet_count++;
      if (amble_filter && too_many_leading_trailing_zeros(bitmap, amble_filter)) {
        if (dbg_pkt) printf("Amble filter: %s\n", bitmap);
        filter_amble++;
        continue;
      }

      if (dbg_pkt) printf("Match on: %s\n", packet_regexes[i]);

      p = &bitmap[pmatch[0].rm_so];
      plen = pmatch[0].rm_eo - pmatch[0].rm_so;

      for (j=0; j<plen; j++) {
        packet_str[j] = *p;
        p++;
      }
      packet_str[j] = '\0';

      if (!is_packet_len_valid(plen)) {
         if (dbg_pkt) printf("is_packet: filtered because of plen = %d\n", (int) plen);
         filter_packet_len++;
         continue;
      }

      // Count the first and last 1
      len_count[plen]++;
      assert(strlen(packet_str) == plen);

      if (filter_and_print_packets) {
        printf("--------------------------------------------\n");
        printf("is_packet: line %6d bitmap = %32s packet str = %20s "
#if 0
#ifdef __APPLE__
               "packet_len = %lld\npacket_match on regex %2d (%s): "
#else
               "packet_len = %d\npacket_match on regex %2d (%s): "
#endif
#endif
               "packet_len = %d\npacket_match on regex %2d (%s): "
               "attempted num = %d (%s)\n",
               line_num, bitmap, packet_str, (int) plen, i,
               packet_regexes[i],
               expected_num, expected_str[expected_num]);
      }

      match_count++;
    } else {
      no_match = 1;
    }
  }
  if (match_count) {
    if (filter_and_print_packets) {
      // printf("------------------------------------------------\n");
    }
    if (dbg_pkt) printf("is_packet: returning %d bitmap = %s packet = %s\n",
       match_count, bitmap, packet_str);
  } else {
    // if (dbg_pkt) printf("is_packet: no matches returning %d\n", match_count);
  }

  if (no_match) {
    if (dbg_pkt) printf("is_packet: filtered because no regex matches %s\n", bitmap);
    filter_regexes++;
  }
  return match_count;
}


// ----------------------------------------------------------------------
bool too_many_leading_trailing_zeros(char *bitmap, int amble_filter_thold)
{
  if (dbg_pre_post) printf("-------------------\n");
  char *start_estimate = preamble_start(bitmap);
  assert(start_estimate);
  // if (dbg_pre_post) printf("start = %p\n", bitmap - start_estimate);
  char *end_estimate = postamble_end(bitmap);
  // if (dbg_pre_post) printf("end = %p endstr = %s\n", bitmap - end_estimate, end_estimate);
  assert(end_estimate);
  // char *end_bitmap = &bitmap[strlen(bitmap)];
  int zeros_before_preamble = 0;
  int zeros_after_postamble = 0;

  char *c = bitmap;
  while (*c && c != start_estimate) {
    if (*c == '0') {
      zeros_before_preamble++;
    }
    c++;
  }
  if (dbg_pre_post) printf("bitmap = [%32s] zeros before = %d\n", bitmap, zeros_before_preamble);

  c = end_estimate;
  while (*c) {
    if (*c == '0') {
      zeros_after_postamble++;
    }
    c++;
  }
  if (dbg_pre_post) printf("bitmap = [%32s] zeros after = %d\n", bitmap, zeros_after_postamble);

  if ((zeros_before_preamble > amble_filter_thold) ||
      (zeros_after_postamble > amble_filter_thold)) {
    return true;
  }
  return false;
}


// ----------------------------------------------------------------------
void count_bits(char *str, int *zeros, int *ones)
{
  int len = strlen(str);
  for (int i=0; i<len; i++) {
    if (str[i] == '0') {
      (*zeros)++;
    } if (str[i] == '1') {
      (*ones)++;
    } else {
      if (!((str[i] == '0') || (str[i] == '1'))) {
        printf("str[%d] = %c (%d) at line = %d\n", i, str[i], str[i], line_count);
      }

      assert((str[i] == '0') || (str[i] == '1'));
    }
  }
  return;
}


// ----------------------------------------------------------------------
void print_filter_stats(void) 
{
  int filter_total = filter_len + filter_no_zeros + filter_no_ones + filter_excess_zeros +
    filter_insufficient_zeros + filter_amble + filter_packet_len +
    filter_regexes;
  printf("\n");
  printf("Filter stats NOTE SOME OF THESE WON'T ADD UP IF THERE ARE MULTIPLE REGEXES USED:\n");
  printf("Total ACKs = %d  Total Filtered = %d  (%.2lf%%) ACKs - Filtered = %d\n",
          line_count, filter_total, (double) filter_total * 100.0 / line_count,
          line_count - filter_total);
  printf("%-55s %8s %12s %8s\n", "Reason", "Count", "%ofFiltered", "%ofAll");
  printf("%-55s %8d %12.2lf %8.2lf\n", "By ACK len too short",
         filter_len, (double) filter_len * 100 / filter_total,
         (double) filter_len * 100 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By no zeros",
         filter_no_zeros, (double) filter_no_zeros * 100 / filter_total,
         (double) filter_no_zeros * 100 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By no ones",
         filter_no_ones, (double) filter_no_ones * 100 / filter_total,
         (double) filter_no_ones * 100 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By insufficient zeros",
         filter_insufficient_zeros, (double) filter_insufficient_zeros * 100 / filter_total,
         (double) filter_insufficient_zeros * 100 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By excess zeros",
         filter_excess_zeros, (double) filter_excess_zeros * 100 / filter_total,
         (double) filter_excess_zeros * 100 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By too many zeros before preamble or after postamble",
         filter_amble, (double) filter_amble * 100.0 / filter_total,
         (double) filter_amble * 100.0 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By due to wrong packet length",
         filter_packet_len, (double) filter_packet_len * 100.0 / filter_total,
         (double) filter_packet_len * 100.0 / line_count);
  printf("%-55s %8d %12.2lf %8.2lf\n", "By regexes",
         filter_regexes, (double) filter_regexes * 100.0 / filter_total,
         (double) filter_regexes * 100.0 / line_count);
}


// ----------------------------------------------------------------------
bool is_packet_len_valid(int plen) 
{
  if ((plen >= (expected_len - plen_short_by)) &&
      (plen <= (expected_len + plen_long_by))) {
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------
char *preamble_start(char *bitmap)
{
  char preamble_str[MAX_BITS] = {'\0'};
  get_preamble_str(preamble_str, preamble_mpdus);
  return(strstr(bitmap, preamble_str));
}

// ----------------------------------------------------------------------
char *postamble_end(char *bitmap)
{
  char *tmp = 0;
  char postamble_str[MAX_BITS] = {'\0'};
  get_postamble_str(postamble_str, preamble_mpdus);
  char *cur = &bitmap[strlen(bitmap)] - strlen(postamble_str);
  tmp = strstr(cur, postamble_str);
  while ((tmp == NULL) && (cur >= bitmap)) {
    if (dbg_pre_post) printf("bitmap = [%32s] cur = %s tmp = %s\n", bitmap, cur, tmp);
    cur--;
    tmp = strstr(cur, postamble_str);
  }
  assert(cur > 0);
  if (dbg_pre_post) printf("cur = %s\n", cur);
  // cur = cur + strlen(postamble_str) + 1;
  cur = cur + strlen(postamble_str);
  if (dbg_pre_post) printf("now cur = %s\n", cur);

  return cur;
}

// ----------------------------------------------------------------------
void print_overall_stats(void) 
{
  printf("\n");
  printf("Overall block ack stats\n");
  printf("%-55s %8d %7.2lf%%  rate = %.1lf/sec\n", "Block Acks with one or more 0's", 
           one_or_more_zeros, (double) one_or_more_zeros * 100.0 / line_count,
           one_or_more_zeros / cumm_rate_gap);
  printf("%-55s %8d %7.2lf%%  rate = %.1lf/sec\n", "Block Acks with no 0's", 
           filter_no_zeros, (double) filter_no_zeros * 100.0 / line_count, 
           (double) filter_no_zeros / cumm_rate_gap );
  printf("%-55s %8d %7.2lf%%  rate = %.1lf/sec\n", "Block Acks", 
           line_count, (double) line_count * 100.0 / line_count, 
           (double) line_count / cumm_rate_gap );
           
  printf("\n");
}

void print_preamble_locations(void) {

  // Exclude block acks that are all zeros and that don't have the preamble
  int adj_count = line_count - filter_no_ones - preamble_locations[AMPDU_LEN];
  double uniform_expectation = 1.0 / (AMPDU_LEN - preamble_mpdus) * 100;
  char *fits = "YES";
  // One extra entry for when there is no preamble found

  printf("\nLOCATIONS of preamble and frequency\n");
  printf("Message len = %d block acks without preamble = %d\n", 
    expected_len, preamble_locations[AMPDU_LEN]);
  printf("all_zeros (no_ones) = %d\n", filter_no_ones);
  printf("adjusted count = %d line_count - filter_no_ones - premable_locations[AMPDU_LEN]\n", adj_count);
  printf( "all_ones (no zeros) = %d\n", filter_no_zeros);
  printf("Uniform would be %6.2lf\n", uniform_expectation);
  printf("Uniform excludes MPDUs at the end where a full postamble won't fit\n");
  printf("1/AMPDU_LEN * 100 = %6.2lf %%\n", 1.0 / AMPDU_LEN * 100.0);
  printf("\n");
  printf("%-10s %5s %8s %6s %7s %6s %4s\n", "# Location", "Index", "Count", "Pct", "Adjust%", "Diff", "Fits");
  for (int i=0; i<AMPDU_LEN; i++) {
    if (i > (AMPDU_LEN - expected_len - 1)) {
       fits = "NO";
    } 

    int adj_value = preamble_locations[i];
    if (i == 0) {
      // TODO: Check if this is right
      adj_value = preamble_locations[i] - filter_no_ones;
    }

    printf("%-10s %5d %8d %6.2lf %7.2lf %6.2lf %4s\n", 
            "Location", i, preamble_locations[i], 
            (double) preamble_locations[i] * 100.0 / line_count,
            (double) adj_value * 100.0 / adj_count,
            ((double) adj_value * 100.0 / adj_count) - uniform_expectation,
            fits);
  }
  printf("\n");

  printf("\nPossible Message Lengths\n");
  printf("Experimental: This is trying count how many MPDUs are there\n");
  printf("              for use between the first set of zeros for the preamble\n");
  printf("              and the last first set of zeros for the postamble\n");
  printf("NOTE: because we are looking for zeros at the beginning and end,\n");
  printf("      lengths are probably off (we aren't including fenceposts)\n");
  printf("%-20s %5s %8s %6s\n", "# Possible Lengths", "Index", "Count", "Pct");
  for (int i=0; i<=AMPDU_LEN; i++) {
    printf("%-20s %5d %8d %6.2lf\n", 
            "Possible Lengths", i, possible_msg_lens[i], 
            (double) possible_msg_lens[i] * 100.0 / possible_count);
  }
  printf("\n");


  printf("\nBits (zeros and ones) histograms\n");
  // TODO: Fix this
  printf("Ratios and averages here are experimental - need to be checked\n");
  printf("Total zeros = %.0lf Total ones = %.0lf Zeros/Ones = %.2lf\n",
          zeros_total, ones_total, zeros_total / ones_total);
  printf("Avg zeros per block ack = %6.2lf avg ones = %6.2lf\n", 
          zeros_total / line_count,
          ones_total / line_count);
  printf("Avg zeros per block ack not counting blocks with all ones and all zeros = %6.2lf avg ones = %6.2lf\n", 
          (zeros_total - (AMPDU_LEN * filter_no_ones)) / (line_count - filter_no_zeros - filter_no_ones),
          (ones_total - (AMPDU_LEN * filter_no_zeros)) / (line_count - filter_no_zeros - filter_no_ones));
  printf("\n");

  printf("%-20s %5s %8s %6s %5s %8s %6s\n", "# Bits histogram",
          "Zeros", "0_count", "0-Pct", "Ones", "1_count", "1-Pct");
  for (int i=0; i<=AMPDU_LEN; i++) {
    int j = AMPDU_LEN - i;
    assert(zeros_histo[i] == ones_histo[j]);
    printf("%-20s %5d %8d %6.2lf %5d %8d %6.2lf\n", 
            "Bits histogram", 
             i, zeros_histo[i], (double) zeros_histo[i] * 100.0 / line_count,
             j, ones_histo[j], (double) ones_histo[j] *  100.0 / line_count);
  }
  printf("\n");

  if (consecutive_zeros) {
    int adj_count = line_count - filter_no_ones - zeros_locations[AMPDU_LEN] - zero_locations_not_counted;
    double uniform_expectation = 1.0 * 100 / (AMPDU_LEN - consecutive_zeros);
    int total_count = 0;
    printf("Zero locations not counted because they were shorter than %d = %d\n",
            ampdu_greater_than, zero_locations_not_counted);
    printf("%-10s %5s %8s %6s %7s %6s\n", "# Zeros", "Index", "Count", "Pct", "Adjust%", "Diff");
    for (int i=0; i<AMPDU_LEN; i++) {
      int adj_value = zeros_locations[i];
      total_count += zeros_locations[i];
      if (i == 0) {
        // TODO: Check if this is right
        adj_value = zeros_locations[i] - filter_no_ones;
      }
      printf("%-10s %5d %8d %6.2lf %7.2lf %6.2lf\n", 
            "Zeros", i, zeros_locations[i], 
            (double) zeros_locations[i] * 100.0 / line_count,
            (double) adj_value * 100.0 / adj_count,
            ((double) adj_value * 100.0 / adj_count) - uniform_expectation);
    }
    printf("\n");
    printf("Total count of packets recording location = %d\n", total_count);
    printf("\n");
  }
}
