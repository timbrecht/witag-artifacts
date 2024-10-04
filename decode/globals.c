#include "match_info.h"
#include "window.h"
#include "decode.h"

// ----------------------------------------------------------------------

int line_count = 0;
int len_count[AMPDU_LEN] = {0};
int correct_by_len[AMPDU_LEN] = {0};
int ack_stats_window = 0;
int one_or_more_zeros = 0;
char **expected_str = 0;
int expected_len = 0;
int num_values = 0;

int zero_locations_not_counted = 0;

int error_counts[MAX_VALUES] = {0};
int match_counts[MAX_VALUES] = {0};
int total_matches = 0;
int counts[MAX_VALUES] = {0};
char **lev_strings;

// So we can close it outside of the function that opens it
FILE *outfp = NULL;

// Trying to get this from Kamran's code that is trying to get
// A-MPDU len from the tcpdump data and sequence numbers
int estimated_ampdu_len = 0;
int num_ties[MAX_VALUES] = {0};
int total_edit_dist[MAX_VALUES] = {0};
