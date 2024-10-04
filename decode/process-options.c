#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <getopt.h>
#include "options.h"
#include "process-options.h"
#include "decode-algs.h"
#include "globals.h"
#include "levenshtein.h"
#include "utils.h"
#include "filter.h"

static const bool dbg = false;

int default_max_zeros_in_packet(int bits_per_packet, 
    int mpdus_per_bit, int preamble_mpdus);

//-----------------------------------------------------------------------
void usage(char *pgm) 
{
  printf("Usage: %s [options] filename\n", pgm);
  printf("  -a # : filter packets if there are # zeros before or after the pre/postamble\n");
  printf("  -b # : Allow packets shorter than expected by # when filtering (can be negative)\n");
  printf("  -c # : Allow packets longer than expected by # when filtering\n");
  printf("  -e   : expected values are not included in the input file\n");
  printf("  -g # : gap threshold\n");
  printf("  -i   : find preamble index/location in the block ack\n");
  printf("  -j   : Extra ones added in levenshtein\n");
  printf("  -k   : Print bit error rate\n");
  printf("  -l # : number of overlapped packets to check to determine value sent\n");
  printf("  -n # : number of block acks to check to determine value sent\n");
  printf("  -p   : apply filter for packets first\n");
  printf("  -q   : quiet mode (reduce the amount of output\n");
  printf("  -r # : use the set of regular expressions # from the source code\n");
  printf("  -t   : Check for and filter packets that are too short - message length minus -b #\n");
  printf("  -v   : verbose (print detailed output)\n");
  printf("  -w # : window size for block ack stats\n");
  printf("  -x   : use exact Levenshtein string lengths only\n");
  printf("  -z # : filter packets that have fewer than # zeros\n");
  printf("  -A   : process all of the file to determine what the sent value was\n");
  printf("  -B # : number of bits in tag packet/message\n");
  printf("  -C # : create information about first Consecutive # of zeros\n");
  printf("  -D # : delay between two messages on the tag \n");
  printf("  -G # : when looking for first consecutive zeros only look at ampdus with estimated length >= #\n");
  printf("  -E # : the expected value being sent in all packets is #\n");
  printf("  -F   : filter and print packets\n");
  printf("  -I   : path to input file\n");
  printf("  -L   : Use Levenshtein algorithm\n");
  printf("  -M # : number of MPDUs used to represent 1 bit\n");
  printf("  -N # : number of MPDUs in AMPDU is 64\n");
  printf("  -O   : path to output file for writing values to (used with -n option)\n");
  printf("  -U   : number of output files (uses -O path with suffixes for each file (e.g., -U 3: file1, file2, file3\n");
  printf("  -P # : number of 0 bits in the premable\n");
  printf("  -R # : print A-MPDU rate after every # block acks\n");
  printf("  -X   : not clear this does anything :-) \n");
  printf("  -Z # : filter packets that have more than # zeros\n");
  printf("\n");
  printf("These options are for use in demos with sensors\n");
  printf("  -f # : factor used to convert sensor values\n");
  printf("  -s # : set sensor type (used for converting readings to values\n");
  printf("       : 0 is no conversion\n");
  printf("       : 1 is temperature\n");
  printf("       : 2 is light\n");
  printf("  -o # : offset used to convert sensor values\n");
  printf("  -d # : delay for # usecs between processing each line,\n");
  printf("         to more easily see changes in web page\n");
  printf("  -S   : safe mode: don't update output file if not enough packets\n");
  printf("  -m # : minimum packet count to update output file (used with safe_mode)\n");
  printf("\n");
  printf("  -h   : print usage\n");
  printf("  -?   : print usage\n");
}

//-----------------------------------------------------------------------
void print_options(void) 
{
  /* Print all the options */
  printf("all_of_file (-A) = %d\n", all_of_file);
  printf("bits_per_packet (-B) = %d\n", bits_per_packet);
  printf("consecutive_zeros (-C) = %d\n", consecutive_zeros);
  printf("ampdu_greater_than (-G) = %d\n", ampdu_greater_than);
  printf("mpdus_per_bit (-M) = %d\n", mpdus_per_bit);
  printf("rate_gap_thold (-R) = %d\n", rate_gap_thold);
  printf("preamble_mpdus (-P) = %d\n", preamble_mpdus);
  printf("expected_value (-E) = %d\n", expected_value);
  printf("amble_filter (-a) = %d\n", amble_filter);
  printf("plen_short_by (-b) = %d\n", plen_short_by);
  printf("plen_long_by (-c) = %d\n", plen_long_by);
  printf("delay (-d) = %d\n", delay);
  printf("delay between two messages on the tag (-D) = %d\n", wait_time);
  printf("expected_in_file (-e) = %d\n", expected_in_file);
  printf("preamble_index (-i) = %d\n", preamble_index);
  printf("min_packet_count (-m) = %d\n", min_packet_count);
  printf("num_consecutive (-n) = %d\n", num_consecutive);
  printf("overlap_packet_count (-l) = %d\n", overlap_packet_count);
  printf("offset (-o) = %lf\n", offset);
  printf("regex_index (-r) = %d\n", regex_index);
  printf("filter_using_estimates (-t) = %d\n", filter_using_estimates);
  printf("filter_and_print_packets (-F) = %d\n", filter_and_print_packets);
  printf("packet_filter (-p) = %d\n", packet_filter);
  printf("gap_thold (-g) = %d\n", gap_thold);
  printf("Infile (-I) = %s\n", infile);
  printf("Outfile (-O) = %s\n", outfile1);
  printf("Output file count (-U) = %d\n", outfile_count);
  printf("verbose (-v) = %d\n", verbose);
  printf("ack_stats_window (-w) = %d\n", ack_stats_window);
  printf("min_zeros (-z) = %d\n", min_zeros);
  printf("zero_thold (-Z) = %d\n", zero_thold);
  printf("Algorithm is Levenshtein (-L) = %d\n", algorithm == ALG_LEV);
  printf("Input file = %s\n", infile);
  printf("factor (-f) = %lf\n", factor);
  printf("sensor_type (-s) = %d\n", sensor_type);
  printf("safe_mode (-S) = %d\n", safe_mode);
  printf("Extra ones added in levenshtein (-j) = %d\n", extra_ones_added);
  printf("bit error rate calculation (-k) = %d\n", print_bit_error_rate);
}

//-----------------------------------------------------------------------
void
process_options(int argc, char *argv[]) 
{
   // char infile[PATH_MAX] = {0};
   // int num_regexes = 0;
   // FILE *fp = NULL;
   // int num_values = 0;

   int i = 0;
   int ch = 0;
   while ((ch = getopt(argc, argv, "0:1:a:b:c:d:ef:g:ijkl:m:n:o:qpr:s:tvw:xz:AB:C:D:G:XR:N:E:FI:LM:O:U:P:SZ:h?")) != -1) {
     switch (ch) {

       // See sensor.h for differnt numbers to use for these types
       case '0':
         sensor0_type = atoi(optarg);
         break;

       // See sensor.h for differnt numbers to use for these types
       case '1':
         sensor1_type = atoi(optarg);
         break;

       case 'A':
         all_of_file = 1;
         break;

       case 'a':
         // Pre and post amble filter
         amble_filter = atoi(optarg);
         break;

       case 'b':
         plen_short_by = atoi(optarg);
         if (!quiet_mode) {
           printf("Allowing packets up to %d MPDUs shorter\n", plen_short_by);
         }
         break;

       case 'c':
         plen_long_by = atoi(optarg);
         if (!quiet_mode) {
           printf("Allowing packets up to %d MPDUs longer\n", plen_long_by);
         }
         break;

       case 'd':
         delay = atoi(optarg);
         if (!quiet_mode) {
           printf("Adding delay = %d usecs before processing each line\n", delay);
         }
         break;

       case 'e':
         if (!quiet_mode) {
           printf("Expected value is not part of the input file\n");
         }
         expected_in_file = 0;
         break;

       case 'f':
         factor = atof(optarg);
         if (!quiet_mode) {
           printf("Using multiplicative factor = %lf\n", factor);
         }
         break;

       case 'g':
         gap_thold = atoi(optarg);
         if (!quiet_mode) {
           printf("Using gap threshold = %d\n", gap_thold);
         }
         break;

       case 'i':
         preamble_index = 1;
         if (!quiet_mode) {
           printf("Finding preamble index/location in the block ack\n");
         }
         // For development / debugging
         // test_preamble_locations();
         break;

         case 'j':
         extra_ones_added = 0;
         if (!quiet_mode) {
           printf("Extra ones added in levenshtein = %d\n", extra_ones_added);
         }
         break;

          case 'k':
         print_bit_error_rate = 1;
         if (!quiet_mode) {
           printf("Print Bit Error Rate = %d\n", print_bit_error_rate);
         }
         break;
         // For development / debugging
         // test_preamble_locations();

       case 'l':
         overlap_packet_count = atoi(optarg);
         if (!quiet_mode) {
           printf("Using overlap packet count = %d\n", overlap_packet_count);
         }
         break;

       case 'm':
         min_packet_count = atoi(optarg);
         if (!quiet_mode) {
           printf("Requiring minimum of %d packets to update output\n", min_packet_count);
         }
         break;

       case 'n':
         num_consecutive = atoi(optarg);
         if (!quiet_mode) {
           printf("Checking %d consecutive packets\n", num_consecutive);
         }
         break;

       case 'o':
         offset = atof(optarg);
         if (!quiet_mode) {
           printf("Using offset = %lf\n", offset);
         }
         break;

       case 'p':
         if (!quiet_mode) {
           printf("Applying packet filters first\n");
         }
         packet_filter = 1;
         break;

       case 's':
         sensor_type = atoi(optarg);
         if (!quiet_mode) {
           printf("Using sensor type = %d\n", sensor_type);
         }
         printf("sensor_type probably does not work anymore\n");
         printf("Use -0 # for one sensor\n");
         printf("Use -1 # for a second sensor\n");
         exit(1);
         break;

       case 't':
         filter_using_estimates = 1;
         if (!quiet_mode) {
           printf("Filtering using estimated_ampdu_lens\n");
         }
         break;

       case 'q':
         quiet_mode = 1;
         if (!quiet_mode) {
           printf("Quiet mode turned on\n");
         }
         break;

       case 'I':
         strcpy(infile, optarg);
         if (!quiet_mode) {
           printf("Using input from file %s\n", infile);
         }
         break;

       case 'L':
         if (!quiet_mode) {
           printf("Using Levenshtein algorithm\n");
         }
         algorithm = ALG_LEV;
         break;

       case 'r':
         regex_index = atoi(optarg);
         assert(regex_index >= 0);
         assert(regex_index <= (sizeof(value_regex_options) / sizeof(char **)));
         if (!quiet_mode) {
           printf("Using regex_option = %d\n", regex_index);
         }
         value_regexes = value_regex_options[regex_index];
         break;

       case 'U':
         outfile_count = atoi(optarg);
         if (!quiet_mode) {
           printf("Number of output files is %d\n", outfile_count);
         }
         break;

       case 'O':
         strcpy(outfile1, optarg);
         if (!quiet_mode) {
           printf("outfile1 = %s\n", outfile1);
         }
         if (outfile_count == 0) {
           outfile_count = 1;
           if (!quiet_mode) {
             printf("Outfile count = %d\n", outfile_count);
           }
         }
         break;

       case 'v':
         verbose = 1;
         break;

       case 'w':
         ack_stats_window = atoi(optarg);
         if (!quiet_mode) {
           printf("Using window size of %d for stats\n", ack_stats_window);
         }
         // Changed from this
         // static struct match_info window[MAX_WINDOW_SIZE] = {0};
         // window = malloc(sizeof(struct match_info) * ack_stats_window;
         break;

       case 'x':
         exact_lev_len = 1;
         if (!quiet_mode) {
           printf("Using exact Levenshtein string lengths only\n");
         }
         break;

       case 'z':
         min_zeros = atoi(optarg);
         if (!quiet_mode) {
           printf("Packets with fewers than %d zeros will not be considered packets\n",
                 min_zeros);
         }
         break;

       case 'Z':
         zero_thold = atoi(optarg);
         if (!quiet_mode) {
           printf("Packets with more than %d zeros will not be considered packets\n",
                 zero_thold);
         }
         break;

       case 'X':
         // runtime = 1;
         printf("The -X options doesn't seem to do anything / is broken\n");
         exit(1);
         break;

       case 'B':
         bits_per_packet = atoi(optarg);
         if (!quiet_mode) {
           printf("Packets should contain %d bits\n", bits_per_packet);
         }

         if (mypow(2, bits_per_packet) > MAX_VALUES) {
           printf("bits_per_packet = %d is larger than expected\n", bits_per_packet);
           printf("Need to increase MAX_VALUES = %d to %d\n", MAX_VALUES, mypow(2, bits_per_packet));
           exit(1);
         }
         break;

       case 'M':
         mpdus_per_bit = atoi(optarg);
         if (!quiet_mode) {
           printf("Packets use %d MPDUs per bit\n", mpdus_per_bit);
         }
         break;

       case 'P':
         preamble_mpdus = atoi(optarg);
         if (!quiet_mode) {
           printf("Packets use %d MPDUs for pre and postamble\n", preamble_mpdus);
         }
         break;

       case 'E':
         expected_value = atoi(optarg);
         assert(expected_value >= 0);
         if (!quiet_mode) {
           printf("Expected value in packets is %d\n", expected_value);
         }
         break;

       case 'F':
         filter_and_print_packets = 1;
         packet_filter = 1;
         break;

       case 'S':
         safe_mode = 1;
         if (!quiet_mode) {
           printf("Safe mode is turned on. Will try to avoid writing unsafe values to output\n");
         }
         break;

       case 'R':
         rate_gap_thold = atoi(optarg);
         // TODO: THIS DOES NOT MAKE SENSE
         // rate_gap_thold = rate_gap_thold;
         if (!quiet_mode) {
           printf("AMPDU rate printed after %d AMPDUs\n", rate_gap_thold);
         }
         break;
       
       case 'N':
         max_mpdus_p_ampdu = atoi(optarg);
         // TODO: THIS DOES NOT MAKE SENSE
         // rate_gap_thold = rate_gap_thold;
         if (!quiet_mode) {
           printf("Max MPDUs in a AMPDU %d \n", max_mpdus_p_ampdu);
         }
         break;

       case 'C':
         consecutive_zeros = atoi(optarg);
         if (!quiet_mode) {
           printf("Checking for %d consecutive zeros\n", consecutive_zeros);
         }

       case 'G':
         ampdu_greater_than = atoi(optarg);
         if (!quiet_mode) {
           printf("For consective zeros only consider ampdus with estimated length >= %d\n", 
                   ampdu_greater_than);
         }

       case 'D':
         wait_time = atoi(optarg);
         if (!quiet_mode) {
           printf("Delay = %d usecs between to tag messages\n", wait_time);
         }
         break;

       case '?':
       case 'h':
       default:
         usage(argv[0]);
         exit(1);
     }
   }


   if (min_zeros == -1) {
     if (preamble_mpdus > 0) {
       min_zeros = 2 * (preamble_mpdus - 1);
     } else {
       min_zeros = MIN_ZEROS_DEFAULT;
     }
   }

   if (!quiet_mode) {
     printf("optind = %d argc = %d\n", optind, argc);
   }
   // Should be one more index left which is the filename

   if (optind == argc - 1) {
      if (infile[0] != 0) {
        printf("Input file may be specified twice or options are not used properly\n");
        usage(argv[0]);
        exit(1);
      }
      strcpy(infile, argv[argc-1]);
      printf("Using input from file %s\n", infile);
   } else if (optind != argc) {
     usage(argv[0]);
     exit(1);
   }

   if (expected_in_file && (expected_value != -1)) {
     printf("Specified an explicit expected_value = %d\n", expected_value);
     printf("  and that expected values can be found in the input expected_in_file = %d\n", expected_in_file);
     printf("These are currently incompatible\n");
     exit(1);
   }

   // If one of these is specified they must all be specified
   if ((bits_per_packet != 0) ||
       (mpdus_per_bit != 0) ||
       (preamble_mpdus != 0)) {

       // printf("Options P, M and B not yet fully implemented\n");

       if (bits_per_packet == -1) {
         printf("Must specify bits_per_packet\n");
         exit(1);
       }
       if (mpdus_per_bit == -1) {
         printf("Must specify mpdus_per_bit\n");
         exit(1);
       }
       if (preamble_mpdus == -1) {
         printf("Must specify preamble_mpdus\n");
         exit(1);
       }

       if (algorithm != ALG_LEV) {
         printf("Must specify preamble_mpdus\n");
         exit(1);
       }

       printf("Creating lev_strings\n");
       fflush(stdout);
       lev_strings = create_lev_strings(bits_per_packet, mpdus_per_bit,
                     preamble_mpdus, &num_values);
       assert(lev_strings);
       // NUM_VALUES = num_values;
       printf("Using %d different possible values 0 -- %d\n",
               num_values, num_values-1);
   } else {
       if (algorithm == ALG_LEV) {
         printf("Must specify P, B and M when using Levenshtein algorithm\n");
         exit(1);
       }
   }

   expected_str = create_expected_strs(bits_per_packet);
   assert(expected_str);
   if (expected_value != -1) {
     printf("Expected value in packets is %d : expected string is %s\n",
             expected_value, expected_str[expected_value]);
     printf("bits_per_packet = %d\n", bits_per_packet);
     assert(expected_value < mypow(2, bits_per_packet));
   }

   // TODO: Check this over. For now if we don't specify a max we compute one
   if (zero_thold == INT_MAX) {
     zero_thold = default_max_zeros_in_packet(bits_per_packet, mpdus_per_bit, preamble_mpdus);
   }

  if (!quiet_mode){
    print_options();
  }

  if (overlap_packet_count && num_consecutive) {
    printf("-l # and -n # can not be used together. Use one or the other.\n");
    exit(1);
  }

  if (all_of_file && num_consecutive) {
    printf("-A and -n # can not be used together. Use one or the other.\n");
    exit(1);
  }

  if (all_of_file && overlap_packet_count) {
    printf("-A and -l # can not be used together. Use one or the other.\n");
    exit(1);
  }

  if (dbg) printf("Opening input file %s\n", infile);
  if (infile[0] == 0) {
    infp = stdin;
    printf("Reading input from stdin\n");
  } else {
    infp = fopen(infile, "r");
  }

  if (infp == NULL) {
    printf("Unable to open input file = %s\n", infile);
    exit(1);
  }


  // printf("Output file is %s\n", outfile);
  // for (int i=0; i<10; i++) {
  //   printf("outfile[%2d] = %c (0x%x)\n", i, outfile[i], outfile[i]);
  // }

  if (outfile1[0] != 0) {
    if (dbg) printf("Output file 1 is %s\n", outfile1);
    if ((num_consecutive <= 0) && (overlap_packet_count <= 0) && !all_of_file) {
       printf("Error: specifying output file -O %s requires -l #, -n # or -A\n", outfile1);
       exit(1);
    }
  }

  if (outfile_count != 0) {
    if (outfile1[0] == '\0') {
       printf("Must specify base path for all output files using -O\n");
       exit(1);
    }

    outfiles = malloc(sizeof(char *) * outfile_count);
    assert(outfiles);
    for (i=0; i<outfile_count; i++) {
       outfiles[i] = malloc(sizeof(char) * PATH_MAX);
       assert(outfiles[i]);
    }

    // Trying to maintain some backwards compatability with existing -O option
    if (outfile_count == 1) {
      strcpy(outfiles[0], outfile1);
    } else {
      for (i=0; i<outfile_count; i++) {
        sprintf(outfiles[i], "%s%d", outfile1, i);
        printf("outfiles[%d] = %s\n", i, outfiles[i]);
      }
    }

    if (!quiet_mode) {
      for (i=0; i<outfile_count; i++) {
        printf("Output readings/numbers for sensor %d to file %s\n", i, outfiles[i]);
      }
    }
  }

  if (safe_mode) {
    if (outfile1[0] == 0) {
      printf("Safe mode can only be used when an output file is specified\n");
      exit(1);
    }
  }

  expected_len = expected_packet_len();
  too_short_len = expected_len - plen_short_by;
  printf("Filtering based on estimated_ampdu_lens that are shorter than %d\n", too_short_len);
}


//-----------------------------------------------------------------------
// If there are too many zeros the block ack probably isn't a packet
// What is the maximum number of zeros we'll consider before we think it's a packet.
//   anything above this value will get filtered as (is not a packet)
#define SPURIOUS_ALLOWED (5)
int default_max_zeros_in_packet(int bits_per_packet, int mpdus_per_bit, int preamble_mpdus) 
{
   return(2 * preamble_mpdus + (bits_per_packet * mpdus_per_bit) + SPURIOUS_ALLOWED);
}
