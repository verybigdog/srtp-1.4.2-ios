/*
 * alloc.c
 *
 * memory allocation and deallocation 
 *
 * David A. McGrew
 * Cisco Systems, Inc.
 */
/*
 *	
 * Copyright (c) 2001-2005 Cisco Systems, Inc.
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

#include "alloc.h"
#include "crypto_kernel.h"

/* the debug module for memory allocation */

debug_module_t mod_alloc = {
  0,                  /* debugging is off by default */
  "alloc"             /* printable name for module   */
};

/*
 * Nota bene: the debugging statements for crypto_alloc() and
 * crypto_free() have identical prefixes, which include the addresses
 * of the memory locations on which they are operating.  This fact can
 * be used to locate memory leaks, by turning on memory debugging,
 * grepping for 'alloc', then matching alloc and free calls by
 * address.
 */

#ifdef SRTP_KERNEL_LINUX

#include <linux/interrupt.h>

void *
crypto_alloc(size_t size) {
  void *ptr;

  ptr = kmalloc(size, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

  if (ptr) {
    debug_print(mod_alloc, "(location: %p) allocated", ptr);
  } else
    debug_print(mod_alloc, "allocation failed (asked for %d bytes)\n", size);

  return ptr;
}

void 
crypto_free(void *ptr) {

  debug_print(mod_alloc, "(location: %p) freed", ptr);

  kfree(ptr);
}


#elif defined(HAVE_STDLIB_H)

void *
crypto_alloc(size_t size) {
  void *ptr;

  ptr = malloc(size);
    
  if (ptr) {
    debug_print(mod_alloc, "(location: %p) allocated", ptr);
  } else
    debug_print(mod_alloc, "allocation failed (asked for %d bytes)\n", size);
    
  return ptr;
}

void 
crypto_free(void *ptr) {

  debug_print(mod_alloc, "(location: %p) freed", ptr);

  free(ptr);
}

#else  /* we need to define our own memory allocation routines */

#error no memory allocation defined yet 

#endif
