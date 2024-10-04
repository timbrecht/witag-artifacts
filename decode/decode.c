//-----------------------------------------------------------------------
// Tim Brecht
// Sun Oct 16 15:37:45 EDT 2022

#define VERSION "Version-0.46"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>
// #include "getopt/getopt.h"
#include "cqueue.h"
#include "levenshtein.h"
#include "sensors.h"
#include "regexes.h"
#include "ack-stats.h"
#include "git-version.h"
#include "decode.h"
#include "options.h"
#include "globals.h"
#include "match_info.h"
#include "window.h"
#include "utils.h"
#include "decode-algs.h"
#include "filter.h"
#include "process-options.h"
#include "durations.h"
#include "interval-stats.h"
#include "expected.h"
//-----------------------------------------------------------------------


static const bool dbg = false;
static const bool dbg_fields = false;

// #define MAX_LEN             (160)
#define MAX_INPUT_FIELD_LEN (AMPDU_LEN)
#define MAX_INPUT_LINE_LEN  (160)

#define MIN_PACKETS_IN_WINDOW  (1)
#define MAX_PACKET_REGEXES  (80)

static char **packet_regexes = 0;
static double min_packets_per_window = 99999999999.0;

#ifdef DEMO_MODE_UNUSED
// output value = (value * factor) + offset
// This allows for large values and adapting different granularities
static double factor = 31.0;   /* Multiple the value read by this amount for light sensor factor = (1023/31) */
static double offset = 0;  /* Add this to the reading (value obtained) */
#endif

void compute_most_freq(struct cqueue *cq, int line_count);
int num_fields(char *str);

//-----------------------------------------------------------------------
int main(int argc, char *argv[])
{
   char line[MAX_INPUT_LINE_LEN] = {0};
   char bitmap[AMPDU_LEN+1] = {0};
   char expected[AMPDU_LEN+1] = {0};
#ifdef OLDWAY
   int expected_counts[MAX_VALUES] = {0};
   int packet_counts[MAX_VALUES] = {0};
#else
   int *expected_counts = calloc(MAX_VALUES, sizeof(int));
   int *packet_counts = calloc(MAX_VALUES, sizeof(int));
   int *freq = calloc(MAX_VALUES, sizeof(int));
   assert(expected_counts);
   assert(packet_counts);
   assert(freq);
#endif

   // values recv/read from dump  "msec dN sent unAck retry rate SGI"
   char r_delta_sec[MAX_INPUT_FIELD_LEN] = {0};
   char r_dN[MAX_INPUT_FIELD_LEN] = {0};
   char r_sent[MAX_INPUT_FIELD_LEN] = {0};
   char r_unack[MAX_INPUT_FIELD_LEN] = {0};
   char r_retry[MAX_INPUT_FIELD_LEN] = {0};
   char r_rate[MAX_INPUT_FIELD_LEN] = {0};
   char r_SGI[MAX_INPUT_FIELD_LEN] = {0};
   char ignore[MAX_INPUT_FIELD_LEN] = {0};
   double delta_sec = 0.0;
   bool all_expected_the_same = true;

   int prev_expected = -1;
   // int num_values = 0;

#ifdef OLDWAY
   regex_t value_regex[MAX_VALUES] = {0};
#else
   regex_t *value_regex = calloc(MAX_VALUES, sizeof(regex_t));
   assert(value_regex);
#endif
   regex_t packet_regex[MAX_PACKET_REGEXES] = {0};


   int retval = 0;
   int i = 0;
   int j = 0;
   struct cqueue *cq = 0;
   struct cqueue *packet_cq = 0;
   int most_freq_matched = false;
   static int most_freq_correct_count = 0;
   static int num_window_checks = 0;
   static int window_packet_count = 0;
   int num_valid_window_checks = 0;
   int packet_count = 0;
   int prev_packet_count = 0;

   int is_a_packet = 0;
   int num_packet_regexes = 0;

   char packet_str[AMPDU_LEN] = {'\0'};

   char **value_regexes = value_regex_options[regex_index];

   struct timeval tv_start = {0};
   struct timeval tv_end = {0};
   struct timeval tv_diff = {0};
   double seconds = 0.0;
   double usecs = 0.0;
   char temp_timestp[MAX_INPUT_FIELD_LEN] = {0};  
   char temp_ssn[MAX_INPUT_FIELD_LEN] = {0};
   char temp_avglen[MAX_INPUT_FIELD_LEN] = {0};
   int total_error_bits = 0;
   int error_in_bits = 0;
   
   // Initialize random number generator
   srand(time(NULL));

   for (i=0; i<argc; i++) {
     printf("%s ", argv[i]);
   }
   printf("\n");
   printf("%s\n", VERSION);
   printf("Git version = %s\n", GIT_VERSION);

   process_options(argc, argv);

#ifdef OLDWAY
   printf("outfile = %s\n", outfile);
   if (outfile[0] != '\0') {
      if (outfp == NULL) {
        outfp = fopen(outfile, "w");
        if (outfp == NULL) {
           fprintf(stderr, "Error: Unable to open output file %s\n", outfile);
           printf("Error: Unable to open output file %s\n", outfile);
           exit(1);
        }
      }
   }
#else
  if (outfile_count > 0) {
    outfps = malloc(sizeof(FILE *) * outfile_count);
    assert(outfps);

    for (int i=0; i<outfile_count; i++) {
      outfps[i] = fopen(outfiles[i], "w");
      if (outfps[i] == NULL) {
         fprintf(stderr, "Error: Unable to open output file %s\n", outfiles[i]);
         printf("Error: Unable to open output file %s\n", outfiles[i]);
         exit(1);
      }
    }
  }
#endif

   if (!quiet_mode) {
     printf("Expected (ideal) packet length = %d including leading and trailing 1\n",
             expected_len);
   }

   packet_regexes = packet_regexes_create(bits_per_packet, mpdus_per_bit, preamble_mpdus);

   // Initialization

   // num_packet_regexes = sizeof(packet_regexes) / sizeof(char *);
   num_packet_regexes = 0;
   while (packet_regexes[num_packet_regexes] != NULL) {
     if (dbg) printf("packet_regexes[%2d] = %s\n",
               num_packet_regexes, packet_regexes[num_packet_regexes]);
     num_packet_regexes++;
   }

   assert(num_packet_regexes < MAX_PACKET_REGEXES);
   if (dbg) printf("Using %d packet regular expresions\n", num_packet_regexes);

   for (i=0; i<num_values; i++) {
     // printf("%d = %s\n", i, expected_str[i]);
     counts[i] = 0;
     error_counts[i] = 0;
     expected_counts[i] = 0;
     match_counts[i] = 0;
   }

   for (i=0; i<MAX_WINDOW_SIZE; i++) {
     window[i].status = STATUS_INVALID;
     window[i].ack_number = -1;
     window[i].is_packet = false;
     for (j=0; j<num_values; j++) {
       window[i].match[j] = false;
     }
   }


   if (num_consecutive > 0) {
     cq = cqueue_create(num_consecutive);
     assert(cq);
   } else if (all_of_file) {
     cq = cqueue_create(MAX_WINDOW_SIZE);
     assert(cq);
   }

   if (overlap_packet_count > 0) {
     packet_cq = cqueue_create(overlap_packet_count);
     assert(packet_cq);

     packet_window = malloc(overlap_packet_count * sizeof(struct match_info));
     assert(packet_window);
     for (i=0; i<overlap_packet_count; i++) {
       packet_window[i].status = STATUS_UNUSED;
       packet_window[i].ack_number = 0;
       packet_window[i].is_packet = false;
       for (j=0; j<MAX_VALUES; j++) {
         packet_window[i].match[j] = false;
       }
     }
   }

   if (ack_stats_window > 0) {
     ack_stats = ack_stats_create(MAX_BITS, ack_stats_window);
     assert(ack_stats);
   }


   /* Compile the number regular expression */
   if (algorithm == ALG_REGEX) {
     for (i=0; i<num_values; i++) {
       retval = regcomp(&value_regex[i], value_regexes[i], REG_EXTENDED);

       if (retval) {
           fprintf(stderr, "Could not compile value_regex %d [%s]\n", i, value_regexes[i]);
           exit(1);
       }
     }
   }

   /* Compile the packet regular expression */
   for (i=0; i<num_packet_regexes; i++) {
     if (dbg) printf("calling regcomp i = %d\n", i);
     retval = regcomp(&packet_regex[i], packet_regexes[i], REG_EXTENDED);

     if (retval) {
         fprintf(stderr, "Could not compile packet_regex %d [%s]\n", i, packet_regexes[i]);
         exit(1);
     }
   }

   duration_calcs(preamble_mpdus, mpdus_per_bit, bits_per_packet);

   gettimeofday(&tv_start, 0);

   /* Main loop: over all of the input */
   while (fgets(line, MAX_INPUT_LINE_LEN, infp) != NULL) {

     if (delay) {
       usleep(delay);
     }

     most_freq_matched = false;

     /* If the file contains the block ack bitmap and the expected string */
     if (expected_in_file) {
       retval = sscanf(line, "%s %s", bitmap, expected);
       if (retval != 2) {
         fprintf(stderr, "Error: trying to read bitmap and expected value failed\n");
         fprintf(stderr, "sscanf returned %d was expecting 2\n", retval);
         fprintf(stderr, "Error on line %d = %s\n", line_count, line);
         exit(1);
       }

       expected_num = expected_str_to_num(expected);
       if (prev_expected == -1) {
          prev_expected = expected_num;
       } else {
          if (prev_expected != expected_num) {
             all_expected_the_same = false;
          }
       }
       prev_expected = expected_num;

     } else {
       /* We don't know what to expected but 0 will keep the rest of the code
          working. We'll just get really bad results in terms of testing
          for accuracy.
        */
       expected_num = 0;
       assert(expected_str);
       strcpy(expected, expected_str[expected_num]);

       if (max_mpdus_p_ampdu == 32) {
         int fields = num_fields(line);
         // printf("line = %d input fields = %d\n", line_count, fields);
         switch (fields) {
           case 0:
              continue;
              break;

           case 1:
             retval = sscanf(line, "%s", bitmap);
             if (retval != 1) {
							 fprintf(stderr, "Error: trying to read bitmap failed\n");
							 fprintf(stderr, "sscanf returned %d was expecting 1\n", retval);
							 fprintf(stderr, "Error on line %d = %s\n", line_count, line);
							 exit(1);
             }
             break;

           case 11:
             // msec dN SSN BM_h BM_l sent unAck retry rate SGI Bitmap
             retval = sscanf(line, "%s %s %s %s %s %s %s %s %s %s %s",
                r_delta_sec, r_dN,
                ignore, ignore, ignore,
                r_sent, r_unack, r_retry, r_rate, r_SGI, bitmap);
             if (retval < 11) {
                if (retval == 10) {
                  continue; 
                }
                fprintf(stderr, "Error: trying to read bitmap failed file = %s\n", infile);
                fprintf(stderr, "sscanf returned %d was expecting 11\n", retval);
                fprintf(stderr, "Error on line %d = %s\n", line_count, line);
                exit(1);
             }
             break;

           case 8:
             retval = sscanf(line, "%s %s %s %s %s %s %s %s",
                      r_delta_sec, r_dN, r_sent, r_unack, r_retry, r_rate, r_SGI, bitmap);

						 if (retval != 8) {
							 fprintf(stderr, "Error: trying to read bitmap failed\n");
							 fprintf(stderr, "sscanf returned %d was expecting 8\n", retval);
							 fprintf(stderr, "Error on line %d = %s\n", line_count, line);
							 exit(1);
						 }
             break;

           default:
             printf("Unknown input format\n");
             exit(1);
             break;
         } /* switch */        
       } else {
         assert(max_mpdus_p_ampdu == 64);
				 retval = sscanf(line, "%s %s %s %s %s %s",
									temp_timestp, r_delta_sec, temp_ssn, r_sent, temp_avglen, bitmap);
				 if (retval != 6) {
					 fprintf(stderr, "Error: trying to read bitmap failed\n");
					 fprintf(stderr, "sscanf returned %d was expecting 8\n", retval);
					 fprintf(stderr, "Error on line %d = %s\n", line_count, line);
					 exit(1);
				 }
       } /* if */
     } /* if */

     double temp_sec = ((double) atoi(r_delta_sec))/1000000.0;
     int temp_len = atoi(r_sent);
     estimated_ampdu_len = temp_len;
     t_f[temp_len-1]++;
#ifdef OLDWAY
     //rate_gap += temp_sec;
#else
     rate_gap += temp_sec;
#endif
     delta_sec += temp_sec;
     delta_ampdu_len += temp_len;
     if (temp_len == max_mpdus_p_ampdu) {
        delta_len_one++;
     }

     dt_blockack_count++;
     // TODO: WHAT IS THIS. It looks wrong. This should probably be expected_str or something
     if (retval == 7) {
         strcpy(bitmap,"0");
     }
     // if (dbg) printf("%s [%s]\n", bitmap, expected);
     line_count++;

     if (all_of_file) {
       if (line_count >= MAX_WINDOW_SIZE) {
         printf("MAX_WINDOW_SIZE = %d is too small for lines in the file\n", MAX_WINDOW_SIZE);
         exit(1);
       }
     }
     windex = line_count % MAX_WINDOW_SIZE;

     if (expected_value != -1) {
       expected_num = expected_value;
       strcpy(expected, expected_str[expected_num]);
     } else {
       expected_num = expected_str_to_num(expected);
     }

     assert(expected_num >= 0);
     assert(expected_num <= num_values);
     expected_counts[expected_num]++;
     // printf("expected_counts[%d] = %d\n", expected_num, expected_counts[expected_num]);

     fflush(stdout);
     is_a_packet = 1;
     packet_str[0] = '\0';
     if (packet_filter) {
        is_a_packet = is_packet(bitmap, packet_regex, num_packet_regexes, 
          zero_thold, amble_filter, line_count, expected_num, packet_str, packet_regexes);
     }

     // if (dbg) printf("is_packet returned %d\n", is_a_packet);
     fflush(stdout);

     if (is_a_packet) {
       packet_counts[expected_num]++;
       packet_count++;

       // no_matches = 1;
       switch (algorithm) {
         case ALG_REGEX:

          if (bits_per_packet != 3) {
            printf("Regexes for packet recognition only done for 3 bits\n");
            exit(1);
          }

          detect_with_regex(packet_str, value_regex, line_count, expected_num, expected, verbose);
          break;

         case ALG_LEV:
          if (dbg) printf("Calling detect_with_lev\n");
          fflush(stdout);
          detect_with_lev(packet_str, line_count, expected_num, expected, verbose, delta_sec, bitmap);
          break;

         default:
          printf("Detection algorithm = %d is unknown\n", algorithm);
          exit(1);
          break;
       } /* switch */

       if (overlap_packet_count > 0) {
         cqueue_add(packet_cq, &packet_window[packet_windex]);
       }

       // window_add() is done inside of algorithm because it knows num and status

     } else {
       window_add(window, windex, VALUE_INVALID, STATUS_NOT_PACKET, false, line_count);
     } /* if */



     if (num_consecutive > 0) {
       cqueue_add(cq, &window[windex]);
     } else if (all_of_file) {
       cqueue_add(cq, &window[windex]);
     }

     if (window[windex].status == STATUS_NO_MATCH) {
       // printf("No match on line %d: num = %d [%s]\n", line_count, num, bitmap);
     }

     // printf("cqueue_items = %d\n", cqueue_items(cq));
     int max_freq = 0;
     int most_freq_value = VALUE_INVALID;

     if (overlap_packet_count > 0 && packet_count != prev_packet_count) {
       process_saved_packets(packet_cq, expected, outfp);
       prev_packet_count = packet_count;
     }

     if (num_consecutive > 0) {
       if (line_count > num_consecutive) {
         // TODO: update_stats
         max_freq = 0;
         most_freq_value = VALUE_INVALID;
#ifdef OLDWAY
         int freq[MAX_VALUES] = {0};
#else
         // TODO: check this carefully
         // man page says last parameter is the number of bytes
         memset(freq, 0, MAX_VALUES * sizeof(int));
#endif

         window_packet_count = 0;

         for (int i=0; i<num_consecutive; i++) {
            struct match_info *m = (struct match_info *) cq->items[i];
            assert(m);
            if (m->is_packet) {
              window_packet_count++;
            }
            num_window_checks++;

            if (m->status == STATUS_MATCH) {
              for (int j=0; j<num_values; j++) {
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

         if (window_packet_count >= MIN_PACKETS_IN_WINDOW) {
            num_valid_window_checks++;

           if ((most_freq_value != VALUE_INVALID) && 
                is_expected(most_freq_value, expected)) {
             most_freq_correct_count++;
             most_freq_matched = true;
           }

           if (verbose) {
             printf("%d: most freq = %d found %d times in %d packets:(%d %%) of frames %s\n",
                   line_count, most_freq_value, max_freq, window_packet_count,
                   (max_freq * 100 / num_consecutive),
                   (most_freq_matched ? "Correct" : "Incorrect"));
           } /* if */
         } /* if */
       } /* if */


#ifdef OLDWAY
       if (most_freq_value != VALUE_INVALID && outfile[0] != '\0') {
          if (safe_mode && (window_packet_count >= min_packet_count)) {
            write_value(outfp, most_freq_value);
          }
       }
#else
       if (safe_mode && ((most_freq_value == VALUE_INVALID) || (window_packet_count < min_packet_count))) {
         // Do nothing
       } else {
         if (outfile_count > 0) {
           write_value(outfps, most_freq_value);
         }
       }
#endif // OLDWAY

       if (line_count > num_consecutive) {
         if (verbose) {
           printf("%8d: Most freq correct = %8d out of %8d = %6.6lf%% ",
               line_count, most_freq_correct_count, num_valid_window_checks,
               most_freq_correct_count * 100.0 / num_valid_window_checks);

           double avg_packets_per_window = 
               (((double) packet_count / line_count) * num_consecutive);
           printf("packets per window = %5.1lf\n", avg_packets_per_window);
           if (avg_packets_per_window < min_packets_per_window) {
             min_packets_per_window = avg_packets_per_window;
           } /* if */
         } /* if */
       } /* if */
     } /* if */


     if (ack_stats && line_count && (line_count % ack_stats_window == 0)) {
       ack_stats_print(ack_stats);
     } /* if */

   } /* while */

   if (all_of_file) {
     compute_most_freq(cq, line_count);
   }

   gettimeofday(&tv_end, 0);
   timersub(&tv_end, &tv_start, &tv_diff);
   usecs = tv_diff.tv_sec * 1000000.0 + tv_diff.tv_usec;
   seconds = usecs / 1000000.0;

   if (num_consecutive > 0) {
     printf("Freq based stats:\n");
     printf("Most freq correct = %8d out of %8d = %6.6lf%% ",
           most_freq_correct_count, num_valid_window_checks,
           most_freq_correct_count * 100.0 / num_valid_window_checks);



     printf("packets per window = %5.1lf\n",
            (((double) packet_count / (double) line_count) *
             (double) num_consecutive));
     printf("minimum packets per window = %5.1lf\n", min_packets_per_window);
   } /* if */

   /* Print the statistics. Errors are the number of times the
    * number was chosen/matched but it did not match the expected value
    */
   if (!quiet_mode) {
    if (rate_gap_thold < INT_MAX) {
       cumm_per_packet_count += dt_per_packet_count;
       cumm_packet_count += dt_packet_count;
       cumm_blockack_count += dt_blockack_count;
       // TODO: check into why rate_gap is added here
       cumm_rate_gap += rate_gap;
       cumm_ampdu_len += delta_ampdu_len;
       cumm_len_one += delta_len_one;
       for (int i=1; i <= AMPDU_LEN; i++) {
            
            
            cumm_t_f[i-1] +=  t_f[i-1];
            t_f[i-1] = 0;

          }

     }
       printf("\nLINE count = %d pkt count = %d pkt = %6.2lf%% "
                "A-MPDUs/s = %.2lf pkts/s = %.2lf  predicted (new) = %.2lf%% \n", 
                line_count, packet_count, (100.0 * packet_count / line_count),
                (double) line_count / (double) cumm_rate_gap, 
                (double) packet_count / (double) cumm_rate_gap,  (100.0 * (t_p[max_mpdus_p_ampdu-1] * Exp_tag_pkt)/t_packet_rate));

       printf("Total matches = %d total matches / packet_count = %.8lf\n",
               total_matches, (double) total_matches / (double) packet_count);

       print_filter_stats();
       print_overall_stats();

       if ((expected_in_file || expected_value != -1) && !quiet_mode) {
         if(print_bit_error_rate) {
               printf("%5s %8s %7s %9s %9s %7s %7s %7s %11s %8s %7s %7s %8s %8s %8s %8s\n",
                 "Num", "Count", "Pkts", "Matches", "%Matches", "Correct", "Errors",
                 "C+E", "Match%Corr", "Pkt%Corr", "%Count", "%Lines", "Binary", "BitError", "AvgDis", "AvgTies");
         }
         else {
              printf("%5s %8s %7s %9s %9s %7s %7s %7s %11s %8s %7s %7s %8s\n",
                "Num", "Count", "Pkts", "Matches", "%Matches", "Correct", "Errors",
                "C+E", "Match%Corr", "Pkt%Corr", "%Count", "%Lines", "Binary");

         }
         for (i=0; i<num_values; i++) {
           int total = 0;
           double percent = 0.0;
           double expect_percent = 0.0;
           total = counts[i] +  error_counts[i];

           if (counts[i] + error_counts[i] > 0) {
             percent = (100.0 * counts[i] / total);
           }

           if (expected_counts[i] != 0) {
             expect_percent = 100.0 * counts[i] / expected_counts[i];
           } else {
             // TODO: FIXME
           }
           if(print_bit_error_rate) {
            printf("%5d %8d %7d %9d %9.2lf %7d %7d %7d %11.1lf %8.1lf %7.1lf %7.1lf %8s %8d %8d %8d\n",
                   i,
                   expected_counts[i],
                   packet_counts[i],
                   match_counts[i],
                   100.0 * match_counts[i] / total_matches,
                   counts[i], error_counts[i],
                   total,
                   percent,
                   100.0 * counts[i] / packet_counts[i],
                   expect_percent,
                   100.0 * counts[i] / line_count,
                   expected_str[i],
                   levenshtein(expected_str[i], expected_str[expected_value]),
                   total_edit_dist[i]/(counts[i]+1),
                   num_ties[i]/(counts[i]+1)
                   );
            error_in_bits = levenshtein( expected_str[i],expected_str[expected_value]);
            total_error_bits += error_counts[i] * error_in_bits;
           }
           else{
            printf("%5d %8d %7d %9d %9.2lf %7d %7d %7d %11.1lf %8.1lf %7.1lf %7.1lf %8s\n",
                   i,
                   expected_counts[i],
                   packet_counts[i],
                   match_counts[i],
                   100.0 * match_counts[i] / total_matches,
                   counts[i], error_counts[i],
                   total,
                   percent,
                   100.0 * counts[i] / packet_counts[i],
                   expect_percent,
                   100.0 * counts[i] / line_count,
                   expected_str[i]
                   );
           }
           
         } /* for */

         if (expected_value != -1) {
           printf("\nEXPECTED value = %4d (%s) correct = %9.6lf\n",
                 expected_value, expected_str[expected_value], 
                 100.0 * match_counts[expected_value] / total_matches);
          if(print_bit_error_rate) {
              printf("\nBit Error Rate = %9.6lf \n",
                     100.0 * total_error_bits / (bits_per_packet * total_matches));
             }
         } 
         if (all_expected_the_same && expected_value == -1) {
           printf("\nEXPECTED value = %4d (%s) correct = %9.6lf\n",
                 expected_num, expected_str[expected_num], 
                 100.0 * match_counts[expected_num] / total_matches);
         }

         printf("\n");
         printf("Counts of packet lengths (includes the 1's in the pre and post amble):\n");
         for (int j=0; j<AMPDU_LEN; j++) {
           if (len_count[j]) {
             printf("len = %2d count = %8d %3d%% correct_by_len = %8d %3d%%\n", 
                     j, len_count[j], 100 * len_count[j] / packet_count, correct_by_len[j],
                     100 * correct_by_len[j] / len_count[j]);
           } /* if */
         } /* for */
      } else if (verbose) {
         printf("%5s %9s %9s %8s\n", "Num", "Matches", "%Matches", "Binary");
         for (i=0; i<num_values; i++) {
           // int total = 0;
           // total = counts[i] +  error_counts[i];
           printf("%5d %9d %9.2lf %8s\n",
                   i,
                   match_counts[i],
                   100.0 * match_counts[i] / total_matches,
                   expected_str[i]
                   );
         }

         printf("Counts of packet lengths (includes the 1's in the pre and post amble):\n");

         for (int j=0; j<AMPDU_LEN; j++) {
           if (len_count[j]) {
             printf("len = %2d count = %8d %.lf%%\n",
                    j, len_count[j], 100.0 * len_count[j] / packet_count);
           }
         } /* for */
      } /* if */
  }

  if (!quiet_mode) {

    if (ack_stats) {
      printf("\n");
      ack_stats_lengths_print(ack_stats);
    }

    if (rate_gap_thold < INT_MAX) {
       double sum_t_p = 0.0;
       // TODO: remove not used.
       double sum_t_f = 0.0;
       double sum_all_f = 0.0;
       double sum_all_duration = 0.0;
       double cummulative_percent = 0.0;
       int tag_packet_len = expected_len;
       // To try to get rid of a warning that seems wrong
       // Both are already 0.0
       sum_all_f = sum_t_f;
       printf("tag_packet_len = %d AMPDU_LEN = %d\n", tag_packet_len, max_mpdus_p_ampdu);
       printf("\nCummulative stats (len, freq, percentage, prob, cummulative):\n");
       for (int i = 1; i <= max_mpdus_p_ampdu; i++) {
         double tmp_percent = (((double) cumm_t_f[i-1]) / ((double) line_count)) * 100.0;
         cummulative_percent += tmp_percent;
         sum_t_p += ((double) cumm_t_f[i-1]) * t_p[i-1];
         printf("(%4d, %8d, %7.3lf, %7.3lf, %6.2lf)\n", 
                i, cumm_t_f[i-1], tmp_percent, t_p[i-1], cummulative_percent);
         sum_t_f += ((double) cumm_t_f[i-1]);
         sum_all_f += ((double) cumm_t_f[i-1]);  // NEW Calculation
         sum_all_duration += ((double) cumm_t_f[i-1]) * 
             (get_PLCP_duration(DATA_RATE, DATA_SGI, DATA_LEN, i));
       } /* for */

       // printf("*** sum_all_f = %.2lf\n", sum_all_f);
       //printf("*** sum_all_duration = %.2lf\n", );
       //printf("*** cumm_rate_gap = %.2lf\n", cumm_rate_gap);

       printf("\n");
       printf("Thr wifi pkts/s (max) = %.2lf and Thr witag pkts/s (max) = %.2lf\n",
         t_packet_rate, t_p[max_mpdus_p_ampdu-1] * Exp_tag_pkt);
       printf("Tag pkts/s = %.4lf\n", Exp_tag_pkt);
       printf("Dump of %.2lf secs AMPDU/s = %.2lf \n", 
               cumm_rate_gap, (double) cumm_blockack_count / cumm_rate_gap);
       printf("Duration of all the packets %.2lf \n",
               sum_all_duration / 1000000.0); 
       printf("Remaining Time (Access overhead and other transmissions) %.2lf \n",
               (cumm_rate_gap - (sum_all_duration / 1000000.0)));
       printf("Expected pkts/s (distribution based) = %.2lf\n", 
             (( (double) sum_t_p / (sum_all_f + 1)) * Exp_tag_pkt));

       
#ifdef OLDWAY
       //printf("BUG HERE BECAUSE sum_t_f = 0.0\n");
       printf("Actual pkts/s (decode) = %.2lf \n",
         (double) cumm_per_packet_count / cumm_rate_gap);
       printf("RE pkts/s(before filters) = %.2lf average_len = %d  %d Len %.2lf%%\n",
         (sum_t_p / (double) sum_t_f) * Exp_tag_pkt,
         cumm_ampdu_len / cumm_blockack_count, max_mpdus_p_ampdu,
         (double) cumm_len_one / (double) cumm_blockack_count * 100.0);
#else
       //printf("BUG HERE BECAUSE sum_t_f = 0.0\n");
       printf("Actual pkts/s (decode) = %.2lf \n",
         (double) cumm_packet_count / cumm_rate_gap);
       printf("RE pkts/s(before filters) = %.2lf average_len = %d  %d Len %.2lf%%\n",
         (double) cumm_per_packet_count / cumm_rate_gap,
         cumm_ampdu_len / cumm_blockack_count, max_mpdus_p_ampdu,
         (double) cumm_len_one / (double) cumm_blockack_count * 100.0);
#endif

       

     } /* if */
  } /* if */
 
  if (preamble_index) {
    print_preamble_locations();
  }

  printf("\nProcessing runtime stats\n");
  printf("Runtime = %lf seconds\n", seconds);
  printf("Block ACKs processed per second = %12.2lf\n", (double) line_count / seconds);
  printf("Packets    processed per second = %12.2lf\n", (double) packet_count / seconds);



  if (cq) {
    cqueue_destroy(cq);
  }

  if (packet_cq) {
    cqueue_destroy(packet_cq);
  }

  if (overlap_packet_count > 0) {
     assert(packet_window);
     free(packet_window);
     packet_window = NULL;
   }

  if (expected_str) {
    destroy_expected_strs(expected_str, num_values);
    expected_str = NULL;
  }

  if (lev_strings) {
    destroy_lev_strings(lev_strings, num_values);
    lev_strings = NULL;
  }

  for (i=0; i<num_packet_regexes; i++) {
     regfree(&packet_regex[i]);
  }

  for (i=0; i<num_values; i++) {
     regfree(&value_regex[i]);
  }

  if (packet_regexes) {
    packet_regexes_destroy(packet_regexes);
    packet_regexes = NULL;
  }

  if (ack_stats) {
    ack_stats_destroy(ack_stats);
    ack_stats = NULL;
  }

  if (infp) {
    fclose(infp);
    infp = NULL;
  }

  if (outfp) {
    fclose(outfp);
    outfp = NULL;
  }
}



//-----------------------------------------------------------------------
// p = premable and postamble, number of MPDUs used
// m = MPDUs per bit
// b = bits per packet

int expected_packet_len(void) {
  return (2 + (2 * preamble_mpdus) + (bits_per_packet * mpdus_per_bit));
}

//-----------------------------------------------------------------------
#ifndef OLDWAY
// See below.
static int freq_all[MAX_VALUES];
#endif

void
compute_most_freq(struct cqueue *cq, int line_count)
{
  int max_freq = 0;
  int most_freq_value = VALUE_INVALID;

  max_freq = 0;
  most_freq_value = VALUE_INVALID;
#ifdef OLDWAY
  // Moved this to get it off of the stack as that
  // might be causing problems on the OpenWRT device we are using
  int freq_all[MAX_VALUES] = {0};
#else
  memset(freq_all, 0, MAX_VALUES * sizeof(int));
#endif

  int window_packet_count = 0;

  for (int i=0; i<line_count; i++) {
    struct match_info *m = (struct match_info *) cq->items[i];
    assert(m);
    if(m->is_packet) {
      window_packet_count++;
    }

    if(m->status == STATUS_MATCH) {
      for(int j=0; j<num_values; j++) {
        if(m->match[j] == true) {
          freq_all[j]++;
          if(freq_all[j] > max_freq) {
            max_freq= freq_all[j];
            most_freq_value= j;
          }
        }
      }
    }
  }

  printf("ALL FILE: most freq = %d found %d times in %d "
         " packets of %d frames = (%.4lf %%) of frames ",
      most_freq_value, max_freq,
      window_packet_count,
      line_count,
      (max_freq * 100.0 / line_count));
  if (window_packet_count == 0) {
    printf("WARNING: window_packet_count is 0\n");
  } else if (window_packet_count < min_packet_count) {
    printf("WARNING: window_packet_count %d is less than min_packet_count %d\n",
            window_packet_count, min_packet_count);
    printf("%.4lf %% of packets\n", max_freq * 100.0 / window_packet_count);
  } else if (window_packet_count < num_consecutive) {
    printf("WARNING: window_packet_count %d is less than num_consecutive %d\n",
            window_packet_count, min_packet_count);
    printf("%.4lf %% of packets\n", max_freq * 100.0 / window_packet_count);
  } else {
    printf("%.4lf %% of packets\n", max_freq * 100.0 / window_packet_count);
  }

  if (safe_mode && ((most_freq_value == VALUE_INVALID) || 
     (window_packet_count < min_packet_count))) {
    if (!quiet_mode) {
      printf("WARNING: Not updating output file\n");
    }
  } else {
    if (outfile_count > 0) {
      write_value(outfps, most_freq_value);
    }
  }
}

// ----------------------------------------------------------------------
int num_fields(char *str) 
{
  int count = 0;
  char *ptr = str;
  // Progress until we get to a non space character
  while (*ptr != '\0' && isspace(*ptr)) {
    ptr++;
  }
  if (dbg_fields) printf("found first non whitespace = %c\n", *ptr);

  while (*ptr != '\0' && !isspace(*ptr)) {
      count++;
      if (dbg_fields) printf("found non whitespace = %c\n", *ptr);
      // Move to next white space location
      while (*ptr != '\0' && !isspace(*ptr)) {
        ptr++;
      }
      if (dbg_fields) printf("found whitespace = %c\n", *ptr);

      // Skip over spaces
      while (*ptr != '\0' && isspace(*ptr)) {
        ptr++;
      }
      if (dbg_fields) if (*ptr) printf("skipped over whitespace = %c\n", *ptr);
  }
  if (dbg_fields) printf("num_fields = %d\n", count);
  return count;
}
