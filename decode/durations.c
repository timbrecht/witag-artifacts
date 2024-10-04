#include <math.h>
#include "decode.h"
#include "durations.h"
#include "globals.h"
#include "options.h"

// TODO: add parens to make the precidence clear
#define BIT(n,i) (n >> i & 1)

#define SIFS (16)        // 10 micro-seconds
#define slot_short (9)   // 9 micro-seconds

/* 
// For Ath9k devices?
#define RTS_DUR (320)
#define CTS_DUR (320)
*/

#define RTS_DUR (28)
#define CTS_DUR (28)

#define DIFS_DUR (34)
#define BLOCK_ACK_DUR (32)
#define PLCP_DUR (40)


int overhead = 15;
//----------------------- Valiable to change for different settings

// long num_burst = 0

double rates[6][2] = {
   {78,  86.7},   // MCS 12
   {104, 115.6},  // MCS 13
   {117, 130},    // MCS 14
   {130, 144.4},  // MCS 15
   {117, 130},    // MCS 20
   {156, 173.3}   // MCS 21
};

double nDBPS[7]= {
  312,  // MCS 12
  416,  // MCS 13
  468,  // MCS 14
  520,  // MCS 15
  468,  // MCS 20
  624,  // MCS 21
  702   // MCS 22
};

int numbits = 12;
// TODO: What is this comment below?
double mpdu_time = 16.0;  // 7.9778; //40/3; //11.324099723;
double m = 1.0;
//double bit_duration = mpdu_time * m;


// ----------------------------------------------------------------------
double mpdu_duration(int payload, int rate, int SGI) 
{
  //double PLCP_bits=0;
  int rate_idx;
  double duration, gi_length;
  int min_dept = 8, npad = 3;
  // int  MPDU_delimiter_dur =0;

  if (rate <= 15) {
    rate_idx = rate - 12;
  } else {
    rate_idx = rate - 16;
  }

  if (SGI ==1) {
    gi_length = 3.6;
  } else {
    gi_length  = 4;
  }
  // TODO: Kamran what is this about?
  // payload = payload;
  npad = payload % 4;
  payload = (payload + npad) * 8;
  duration= ceil(payload / nDBPS[rate_idx]) * gi_length;
  if (duration < min_dept) {
    duration = min_dept;
  }
  return (round(duration));
}

// ----------------------------------------------------------------------
double get_backoff(void) 
{
  return 7.5 * slot_short;
}

// ----------------------------------------------------------------------
double get_payload_duration(int rate_idx, int SGI, int length, int aggr_num) 
{
  int mpdu_time =  mpdu_duration(length, rate_idx, SGI);
  double payload = mpdu_time * aggr_num ;
  return payload;
}

// ----------------------------------------------------------------------
double get_PLCP_duration(int rate_idx, int SGI, int length, int aggr_num) 
{
  double overhead_duration = RTS_DUR + SIFS + CTS_DUR + SIFS + 
         DIFS_DUR + get_backoff() + PLCP_DUR;
  overhead_duration += SIFS + BLOCK_ACK_DUR;
  return get_payload_duration(rate_idx,SGI, length, aggr_num) + 
         overhead_duration;
}


// ----------------------------------------------------------------------
void duration_calcs(int preamble_mpdus, int mpdus_per_bit, int bits_per_packet) 
{
  int i = 0;
  int tag_packet_len = (mpdus_per_bit * bits_per_packet + 
                        2 * preamble_mpdus + 2);
  double tag_packet_dur = tag_packet_len  * mpdu_duration(DATA_LEN, DATA_RATE, DATA_SGI);
  Exp_tag_pkt =  1000000.0 / (tag_packet_dur + wait_time);

  for (i=tag_packet_len; i <= max_mpdus_p_ampdu; i++) {
    double AMPDU_dur = get_PLCP_duration(DATA_RATE, DATA_SGI, DATA_LEN, i);
    //printf(" %f  ", 1000000/AMPDU_dur);

    double payload = get_payload_duration(DATA_RATE, DATA_SGI, DATA_LEN, i);
    double x = payload - tag_packet_dur;
    if (x > 0) {
      t_p[i-1] = x / AMPDU_dur;
    } else {
      t_p[i-1] = 0;
    }
    //printf("(%d, %.4f)  ",i, t_p[i-1]);
    t_packet_rate = 1000000 / AMPDU_dur;
  }
}
