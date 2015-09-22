/*
 * rand_gen.c
 *
 * a random source (random number generator)
 *
 * David A. McGrew
 * Cisco Systems, Inc.
 */
/*
 *	
 * Copyright(c) 2001-2005 Cisco Systems, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 * 
 *   Neither the name of the Cisco Systems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdio.h>           /* for printf() */
#include <unistd.h>          /* for getopt() */
#include "crypto_kernel.h"

/*
 * MAX_PRINT_STRING_LEN is defined in datatypes.h, and is the length
 * of the largest hexadecimal string that can be generated by the
 * function octet_string_hex_string().
 */

#define BUF_LEN (MAX_PRINT_STRING_LEN/2)

void
usage(char *prog_name) {
  printf("usage: %s -n <num_bytes> [-l][ -d debug_module ]*\n"
	 "   -n <num>   output <num> random bytes, where <num>"
	 " is between zero and %d\n"
	 "   -l         list the avaliable debug modules\n"
	 "   -d <mod>   turn on debugging module <mod>\n", 
	 prog_name, BUF_LEN);
  exit(255);
}

int
main (int argc, char *argv[]) {
  extern char *optarg;
  int q;
  int num_octets = 0;
  unsigned do_list_mods = 0;
  err_status_t status;

  if (argc == 1)
    usage(argv[0]);

  /* initialize kernel - we need to do this before anything else */ 
  status = crypto_kernel_init();
  if (status) {
    printf("error: crypto_kernel init failed\n");
    exit(1);
  }

  /* process input arguments */
  while (1) {
    q = getopt(argc, argv, "ld:n:");
    if (q == -1) 
      break;
    switch (q) {
    case 'd':
      status = crypto_kernel_set_debug_module(optarg, 1);
      if (status) {
	printf("error: set debug module (%s) failed\n", optarg);
	exit(1);
      }
      break;
    case 'l':
      do_list_mods = 1;
      break;
    case 'n':
      num_octets = atoi(optarg);
      if (num_octets < 0 || num_octets > BUF_LEN)
	usage(argv[0]);
      break;
    default:
      usage(argv[0]);
    }    
  }

  if (do_list_mods) {
    status = crypto_kernel_list_debug_modules();
    if (status) {
      printf("error: list of debug modules failed\n");
      exit(1);
    }
  }

  if (num_octets > 0) {
    uint8_t buffer[BUF_LEN];
    
    status = crypto_get_random(buffer, num_octets);
    if (status) {
      printf("error: failure in random source\n");
    } else {
      printf("%s\n", octet_string_hex_string(buffer, num_octets));
    }
  }

  status = crypto_kernel_shutdown();
  if (status) {
    printf("error: crypto_kernel shutdown failed\n");
    exit(1);
  }
  
  return 0;
}

