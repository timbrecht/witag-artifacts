#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
//----------------Packet duration calculation
/* 802.11 general constants */


#define SIFS (16)        // 10 micro-seconds
#define slot_short (9)   // 9 micro-seconds

/*
#define RTS_DUR (320)
#define CTS_DUR (320)
*/
#define RTS_DUR (28)
#define CTS_DUR (28)


#define DIFS_DUR (34)

//#define AMPDU_len (32)

#define BlockACK_DUR (32)

#define PLCP_DUR (40)

int DATA_RATE = 15;
int DATA_SGI = 1;

double t_p = 0;
double t_packet_rate = 0;
// Hence, approx 754 AMPDUs/sec

int overhead = 15;
//----------------------- Valiable to change for different settings
int DATA_LEN = 32;  // 1560
int factor = 2;
int wait_time = 1000;
static int bits_per_packet = 5;
static int mpdus_per_bit = 3;
static int preamble_mpdus = 3;
//----------------------- Valiable to change for different settings

double Exp_tag_pkt = 0;
// static int dt_packet_count = 0;
// static int dt_per_packet_count = 0;
// static int dt_blockack_count = 0;
// static double rate_gap = 0.0;

long num_burst;
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
double mpdu_time = 16;  // 7.9778; //40/3; //11.324099723;
double m = 1;
//double bit_duration = mpdu_time * m;

// TODO: add parens to make the precidence clear
#define BIT(n,i) (n >> i & 1)

double mpdu_duration(int payload, int rate, int SGI) {
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

double get_backoff(void) {
  return 7.5 * slot_short;
}

double get_payload_duration(int rate_idx, int SGI, int length, int aggr_num) {
  int mpdu_time =  mpdu_duration(length, rate_idx, SGI);
  double payload = mpdu_time * aggr_num ;
  return payload;
}

double get_PLCP_duration(int rate_idx, int SGI, int length, int aggr_num) {
  double overhead_duration = RTS_DUR + SIFS + CTS_DUR + SIFS + 
         DIFS_DUR + get_backoff() + PLCP_DUR;
  overhead_duration += SIFS + BlockACK_DUR;
  return get_payload_duration(rate_idx,SGI, length, aggr_num) + 
         overhead_duration;
}

int  main(int argc, char *argv[]) {
    
    /*
    why going down for even number of packets
    Max payload 
    
    3 lines
    probabilty of overlap based on payload
    number of packets per sec
    multiple of the first two line 
    
    
    256, 512,1024, max
    */
    if(argc == 2  && strcmp(argv[1],"-h") == 0){
      printf("  [AMPDU_len] [mpdus_per_bit] [bits_per_packet] [preamble_mpdus] [Data_len]  [factor]\n");
      return 0;
    }

    if ( argc != 7) {    
      printf("  [AMPDU_len] [mpdus_per_bit] [bits_per_packet] [preamble_mpdus] [Data_len]  [factor]\n");
      printf("  -h   : print usage\n");
      return 0;
    }
    int AMPDU_len = 32;
    AMPDU_len = atoi(argv[1]);
    mpdus_per_bit = atoi(argv[2]);
    bits_per_packet = atoi(argv[3]);
    preamble_mpdus = atoi(argv[4]);
    DATA_LEN = atoi(argv[5]);
    factor = atoi(argv[6]);
    double overhead = RTS_DUR + SIFS + CTS_DUR + SIFS + DIFS_DUR + get_backoff() + PLCP_DUR;

    printf("# %12s %15s %15s %2.1f ms) %12s %12s %12s %12s\n", "MPDU_Size", "AMPDU_Dur(ms)","AMPDU_Overhead%(", overhead/1000, "OverlapProb",  "MaxWiFiPkts",  "MaxTagPkts", "OverlapRate");
    for (int i=DATA_LEN; i <= 1560; i*=2) {
        int tag_packet_len = (mpdus_per_bit * bits_per_packet + 2 * preamble_mpdus + 2);
        double tag_packet_dur = tag_packet_len  * mpdu_duration(i, 15, 0);
        
        double AMPDU_dur = get_PLCP_duration(DATA_RATE, DATA_SGI, i,AMPDU_len );
      //printf(" %f  ", 1000000/AMPDU_dur);
    
        double payload = get_payload_duration(DATA_RATE, DATA_SGI, i, AMPDU_len);
      //
        wait_time = factor * AMPDU_dur;
        Exp_tag_pkt =  1000000.0/(tag_packet_dur + wait_time);
        double x = payload - tag_packet_dur;
        if (x > 0)
          t_p = x / AMPDU_dur;
    
        else
          t_p = 0;
         //
        t_packet_rate = 1000000 / AMPDU_dur;
        printf("  %12d %12.2f %2.1f%% %12.6f %12.1f %12.1f %12.1f\n", i, AMPDU_dur/1000.0, ((AMPDU_dur-payload)/AMPDU_dur)*100 , t_p, t_packet_rate, Exp_tag_pkt, t_p * Exp_tag_pkt);
    
    
      }
}
