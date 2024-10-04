#include "interval-stats.h"
// number of tag pkts in a second for current B, M, P and wait_time
double Exp_tag_pkt = 0;
// variables for delta interval delta is rate_gap_thold
int dt_packet_count = 0;
int dt_per_packet_count = 0;
int dt_blockack_count = 0;
double rate_gap = 0.0;
int delta_ampdu_len = 0;
int delta_len_one = 0;

// Variables cummulative stats computed over time for the dump
int  cumm_t_f[AMPDU_LEN] = {0};
int cumm_per_packet_count = 0;
int cumm_packet_count = 0;
int cumm_blockack_count = 0;
double cumm_rate_gap = 0;
int cumm_ampdu_len = 0;
int cumm_len_one = 0;

//variables for Theoretical value calculation 
int t_f[AMPDU_LEN] = {0};
double t_p[AMPDU_LEN] = {0};
double t_packet_rate = 0;

