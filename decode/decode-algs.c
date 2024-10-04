#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <regex.h>
#include <limits.h>
#include "decode-algs.h"
#include "levenshtein.h"
#include "globals.h"
#include "window.h"
#include "utils.h"
#include "options.h"
#include "interval-stats.h"
#include "expected.h"
#include "filter.h"

// ----------------------------------------------------------------------
static const bool dbg = false;
static const bool dbg_lev = false;
static int last_packet_line = 0;
static int last_packet_time = 0;

char **value_regexes = 0;

// ----------------------------------------------------------------------
// Function added to add levenshtein distance ties in an array 
void break_ties(int *result, int *min, int *num, int *min_index, int *ties_count, int *ties_index){
        if (*result < *min) {
          *ties_count = 0;
          ties_index[0] = *num;
          *min = *result;
          *min_index = *num;
          
        }
        else{
            if( *result == *min) {
                (*ties_count)++;
                ties_index[*ties_count] = *num ;
            }
          } 
      
}

#ifndef OLDWAY
// See below
static int ties_index[MAX_VALUES];
#endif

void detect_with_lev(char *packet, int line_count,
     int expected_num, char *expected, int verbose,  double delta_sec, char *bitmap)
{
   int min = INT_MAX;
   int min_index = -1;
   // got_match = 0;
   int num = 0;
   int result = -1;
   int status = STATUS_UNUSED;
   int got_match = 0;
   char target[MAX_BITS] = {0};
   int plen = 0;
   double sum_t_p = 0.0;
   double sum_t_f = 0.0;
   int ties_count = 0;
#ifdef OLDWAY
   // Suspect that large stacks were causing problems on the OpenWRT 
   // device we are using so moving this off of the stack.
   int ties_index[MAX_VALUES] = {0};
#else
   memset(ties_index, 0, MAX_VALUES * sizeof(int));
#endif
   int rand_num = 0;
   /* Check for a match in any/all of the strings */
   for (num=0; num<num_values; num++) {
      if (dbg) printf("detect_with_lev: packet = %s num = %d lev_string = %s\n",
                       packet, num, lev_strings[num]);

      plen = strlen(packet);
      // TODO: Check on this and next bit of code
      if (extra_ones_added == 1) { // extra_ones_added
            if ((plen == expected_len) ||  exact_lev_len) {
              fflush(stdout);
              result = levenshtein(packet, lev_strings[num]);
              if (dbg_lev) printf("packet = [%s] target = [%27s] distance from %2d = %2d regular string\n",
                      packet, lev_strings[num], num, result);
              if (print_bit_error_rate) { // 0 extra_ones_added and print_bit_error
                  break_ties(&result, &min, &num, &min_index, &ties_count, ties_index); 
                 }  //  0 extra_ones_added and print_bit_error
                 else { // extra_one_added
                  if (result < min) {
                    min = result;
                    min_index = num;
                  }
                 }
            } // if plen == expected_len 
            else {
              /* This is to handle packets that are 1 longer than the "ideal" */
              /* With an extra 1 at the beginning */
             
                  fflush(stdout);
                  strcpy(target, "1");
                  strcat(target, lev_strings[num]);
                  result = levenshtein(packet, target);
                  if (dbg_lev) printf("packet = [%s] target = [%27s] distance from %2d = %2d with 1 prepended\n",
                          packet, target, num, result);
                  if (print_bit_error_rate) { // 1-start extra_ones_added and print_bit_error
                   break_ties(&result, &min, &num, &min_index, &ties_count, ties_index); 
                  }   //  1-start extra_ones_added and print_bit_error 
                  else {
                   if (result < min) {
                    min = result;
                    min_index = num;
                   }
                 }

                  /* This is to handle packets that are 1 longer than the "ideal" */
                  /* With an extra 1 at the end */
                  fflush(stdout);
                  strcpy(target, lev_strings[num]);
                  strcat(target, "1");
                  result = levenshtein(packet, target);
                  if (dbg_lev) printf("packet = [%s] target = [%27s] distance from %2d = %2d with 1 appended\n",
                          packet, target, num, result);
                  if (print_bit_error_rate) { // 1-end extra_ones_added and print_bit_error
                     break_ties(&result, &min, &num, &min_index, &ties_count, ties_index); 
                 }  //  1-end extra_ones_added and print_bit_error 
                 else {
                  if (result < min) {
                    min = result;
                    min_index = num;
                  }
                 }
               // --?
               
            } // else plen == expected_len
        }  // extra_ones_added 
        else { //else NO extra_ones_added
              
              result = levenshtein(packet, lev_strings[num]);
              if (dbg_lev) printf("packet = [%s] target = [%27s] distance from %2d = %2d regular string without adding extra ones\n",
                      packet, lev_strings[num], num, result);
              if (print_bit_error_rate) {
                 break_ties(&result, &min, &num, &min_index, &ties_count, ties_index); 
              }  // if print_bit_error_rate and No extra Ones
              else { // NO extra one and NO print BER
                  if (result < min) {
                    min = result;
                    min_index = num;
                  }
                 } // else print_bit_error_rate

        }  // else No extra_ones_added
   }  // for loop
   // Did we get a match
   if (min < INT_MAX) {
       num = min_index;
       status = STATUS_MATCH;
       got_match++;
       // no_matches = 0;
       
       dt_packet_count++;
    if (print_bit_error_rate){
          if(ties_count > 0){
             rand_num = rand() % ties_count;
          }
          else {
             rand_num = 0;

          }
          /*if(ties_count > 0){
            for (int i=0; i <= ties_count; i++) {
              printf(" %d, ",ties_index[i]);
            }
            printf(" r:%d \n ",rand_num);
          }*/
          num = ties_index[rand_num];
          min_index = num;
          num_ties[num] += ties_count;
          total_edit_dist[num] += min;
    } //if 

        
#ifdef OLDWAY
       rate_gap += delta_sec - last_packet_time;
#endif
       int gap = line_count - last_packet_line;
       double time_gap = delta_sec - last_packet_time;

       if (rate_gap >= rate_gap_thold) {
           //---- TODO-------
          sum_t_p = 0.0;
          sum_t_f = 0.0;
  //        tag_packet_len = (mpdus_per_bit * bits_per_packet + 2 *preamble_mpdus + 2);
          for (int i=1; i <= AMPDU_LEN; i++) {
            sum_t_p += ((double) t_f[i-1]) * t_p[i-1];
            //printf("(%d, %.2d)  ",i,t_f[i-1]);
            sum_t_f += ((double) t_f[i-1]);
            cumm_t_f[i-1] +=  t_f[i-1];
            t_f[i-1] = 0;

          }

          printf("During last %d sec AMPDU/s = %.2lf  and Thr pkt/s = %.2lf ",
                  rate_gap_thold, (double) dt_blockack_count / rate_gap,
                  t_p[max_mpdus_p_ampdu-1] * Exp_tag_pkt);

          printf("Exp pkt/s = %.2lf actual pkt/s = %.2lf ",
            sum_t_p / (double) sum_t_f * Exp_tag_pkt, 
            (double) dt_packet_count / rate_gap); 

          printf("RE pkt/s = %.2lf avg-len = %d  %d-Len %.2lf%%\n", 
            (double) dt_per_packet_count / rate_gap,
            delta_ampdu_len / dt_blockack_count, max_mpdus_p_ampdu,
            (double) delta_len_one / (double) dt_blockack_count * 100.0);

          // printf("delta_len_one = %d dt_blockack_count = %d\n", delta_len_one, dt_blockack_count);

          // update the cummulative variables before reseting delta variables 
          cumm_per_packet_count += dt_per_packet_count;
          cumm_packet_count += dt_packet_count;
          cumm_blockack_count += dt_blockack_count;
          cumm_rate_gap += rate_gap;
          cumm_ampdu_len += delta_ampdu_len;
          cumm_len_one += delta_len_one;

          // reset delta variables after rate_gap_thold seconds
          dt_per_packet_count = 0;
          dt_packet_count = 0;
          dt_blockack_count = 0;
          rate_gap = 0;
          delta_ampdu_len = 0;
          delta_len_one = 0;
       }

       if (gap >= gap_thold) {
         printf("BIG gap found at line_count = %d last packet was on line %d gap = %d\n",
                 line_count, last_packet_line, gap);
       }
       if (verbose) {
         printf("T from start %.2lf sec T_last_match %.2lf match on line %8d: "
                "num = %4d [%32s] len = %3d guessed %4d (%s) gap = %5d (%5.1lf%%)",
               delta_sec, time_gap, line_count, num, packet, plen, num, expected_str[num],
               gap, 100.0 / (double) gap);


         last_packet_line = line_count;
         last_packet_time = delta_sec  ;
         if (expected_in_file || expected_value != -1) {
           printf(" expected = %s", expected);
         } else {
           printf("\n");
         }
       }

       if ((expected_in_file || (expected_value != -1)) &&
            is_expected(num, expected)) {
         if (verbose) {
           printf(" is correct\n");
         }
         counts[num]++;
         correct_by_len[strlen(packet)]++;

       } else {
         if ((expected_in_file || (expected_value != -1))  && verbose) {
           printf(" is NOT correct\n");
           printf("Bitmap was %s\n", bitmap);
         }
         error_counts[num]++;
       }

       if (filter_and_print_packets) {
         // printf("------------------------------------------------\n");
       }

     } else {
       if (verbose) {
          // printf("No match on line %d: num = %d [%s]\n", line_count, num, bitmap);
       }
       status = STATUS_NO_MATCH;
     }

     if (got_match) {
       match_counts[num]++;
       total_matches++;

       // If we were able to decode the packet add it to the packet window
       if (overlap_packet_count > 0) {
         packet_windex = (packet_windex + 1) % overlap_packet_count;
         window_add(packet_window, packet_windex, num, status, true, line_count);
       }
     }

     window_add(window, windex, num, status, true, line_count);

}



// ----------------------------------------------------------------------
// These are specific to FIVE_BITS
/* These are the regular expression we are trying for numbers */
static char *value_regexes_0[] =
{
  /* 0 */ "10{3,4}0{2,4}0{2,4}0{2,4}0{3,4}1",
  /* 1 */ "10{3,4}0{2,4}0{2,4}1{2,4}0{3,4}1",
  /* 2 */ "10{3,4}0{2,4}1{2,4}0{2,4}0{3,4}1",
  /* 3 */ "10{3,4}0{2,4}1{2,4}1{2,4}0{3,4}1",
  /* 4 */ "10{3,4}1{2,4}0{2,4}0{2,4}0{3,4}1",
  /* 5 */ "10{3,4}1{2,4}0{2,4}1{2,4}0{3,4}1",
  /* 6 */ "10{3,4}1{2,4}1{2,4}0{2,4}0{3,4}1",
  /* 7 */ "10{3,4}1{2,4}1{2,4}1{2,4}0{3,4}1",
};

static char *value_regexes_1[] =
{
  /* 0 */ "10{3,4}0{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 1 */ "10{3,4}0{2,3}0{2,3}1{2,3}0{3,4}1",
  /* 2 */ "10{3,4}0{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 3 */ "10{3,4}0{2,3}1{2,3}1{2,3}0{3,4}1",
  /* 4 */ "10{3,4}1{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 5 */ "10{3,4}1{2,3}0{2,3}1{2,3}0{3,4}1",
  /* 6 */ "10{3,4}1{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 7 */ "10{3,4}1{2,3}1{2,3}1{2,3}0{3,4}1",
};

static char *value_regexes_2[] =
{
  /* 0 */ "10{3,4}0{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 1 */ "10{3,4}0{2,3}0{2,3}1{2,3}0{3,4}1",
  /* 2 */ "10{3,4}0{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 3 */ "10{3,4}0{2,3}1{2,3}1{2,3}0{3,4}1",
  /* 4 */ "10{3,4}1{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 5 */ "10{3,4}1{2,3}0{2,4}1{2,3}0{3,4}1",  // NOTE this is different
  /* 6 */ "10{3,4}1{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 7 */ "10{3,4}1{2,3}1{2,3}1{2,3}0{3,4}1",
};

static char *value_regexes_3[] =
{
  // Trying to allow for the possibility of a 1 here and there for 000
  /* 0 */ "10{3,4}1{0,1}0{2,4}1{0,1}0{2,4}1{0,1}0{2,4}1{0,1}0{3,4}1",
  /* 1 */ "10{3,4}0{2,3}0{2,3}1{2,3}0{3,4}1",
  /* 2 */ "10{3,4}0{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 3 */ "10{3,4}0{2,3}1{2,3}1{2,3}0{3,4}1",
  /* 4 */ "10{3,4}1{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 5 */ "10{3,4}1{2,3}0{2,4}1{2,3}0{3,4}1",  // NOTE this is different
  /* 6 */ "10{3,4}1{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 7 */ "10{3,4}1{2,3}1{2,3}1{2,3}0{3,4}1",
};

static char *value_regexes_4[] =
{
  /* 0 */ "10{2,4}0{2,3}0{2,3}0{2,3}0{2,4}1",
  /* 1 */ "10{2,4}0{2,3}0{2,3}1{2,3}0{2,4}1",
  /* 2 */ "10{2,4}0{2,3}1{2,3}0{2,3}0{2,4}1",
  /* 3 */ "10{2,4}0{2,3}1{2,3}1{2,3}0{2,4}1",
  /* 4 */ "10{2,4}1{2,3}0{2,3}0{2,3}0{2,4}1",
  /* 5 */ "10{2,4}1{2,3}0{2,4}1{2,3}0{2,4}1",  // NOTE this is different
  /* 6 */ "10{2,4}1{2,3}1{2,3}0{2,3}0{2,4}1",
  /* 7 */ "10{2,4}1{2,3}1{2,3}1{2,3}0{2,4}1",
};

static char *value_regexes_5[] =
{
  /* 0 */ "10{3,4}0{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 1 */ "10{3,4}0{2,3}0{2,3}1{2,3}0{3,4}1",
  /* 2 */ "10{3,4}0{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 3 */ "10{3,4}0{2,3}1{2,3}1{2,3}0{3,4}1",
  /* 4 */ "10{3,4}1{2,3}0{2,3}0{2,3}0{3,4}1",
  /* 5 */ "10{3,4}1{2,3}0{2,4}1{2,3}0{3,4}1",  // NOTE this is different
  /* 6 */ "10{3,4}1{2,3}1{2,3}0{2,3}0{3,4}1",
  /* 7 */ "10{3,4}1{2,3}1{2,3}1{2,3}0{3,4}1",
};


// NOTE: CHANGE NUM_REGEXES IF CHANGING THIS
char **value_regex_options[NUM_REGEXES] = {
  value_regexes_0,
  value_regexes_1,
  value_regexes_2,
  value_regexes_3,
  value_regexes_4,
  value_regexes_5,
};


// ----------------------------------------------------------------------
void detect_with_regex(char *packet, regex_t value_regex[],
    int line_count, int expected_num, char *expected, int verbose)
{
   int got_match = 0;
   int no_matches = 1;
   int status = STATUS_UNUSED;
   int num = 0;
   int retval = 0;
   char msgbuf[256] = {0};

   /* Check for a match in any/all of the regular expressions */
   for (num=0; num<num_values; num++) {

     got_match = 0;

     /* Execute regular expression */
     if (dbg) printf("detect_with_regex: calling regexec num = %d\n", num);
     retval = regexec(&value_regex[num], packet, 0, NULL, 0);
     if (retval == REG_MATCH) {
       status = STATUS_MATCH;
       got_match++;
       no_matches = 0;

       if (verbose) {
         printf("Match on line %8d: num = %d [%32s] guessed %d (%s), expected = %s",
               line_count, num, packet, num, expected_str[num], expected);
       }
       if (is_expected(num, expected)) {
         if (verbose) {
           printf(" is correct\n");
         }
         counts[num]++;
         correct_by_len[strlen(packet)]++;

       } else {
         if (verbose) {
           printf(" is NOT correct\n");
         }
         error_counts[num]++;
       }

       if (filter_and_print_packets) {
         // printf("------------------------------------------------\n");
       }

     } else if (retval == REG_NOMATCH) {
       if (verbose) {
          // printf("No match on line %d: num = %d [%s]\n", line_count, num, packet);
       }
       status = STATUS_NO_MATCH;
     } else {
       regerror(retval, &value_regex[num], msgbuf, sizeof(msgbuf));
       fprintf(stderr, "Regex match failed: %s\n", msgbuf);
       exit(1);
     }

     if (got_match) {
       match_counts[num]++;
       total_matches++;
     }

     window_add(window, windex, num, status, true, line_count);
     window_add(packet_window, packet_windex, num, status, true, line_count);
   }

   if (no_matches && verbose) {
     printf("No match on line %d: [%s] expected = %s\n", line_count, packet, expected);
   }
}

