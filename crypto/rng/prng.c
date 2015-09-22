/*
 * prng.c 
 *
 * pseudorandom source
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


#include "prng.h"

/* single, global prng structure */

x917_prng_t x917_prng;

#ifdef WINCE
static time_t time(time_t *timer);
#endif

err_status_t
x917_prng_init(rand_source_func_t random_source) {
  v128_t tmp_key;
  err_status_t status;

  /* initialize output count to zero */
  x917_prng.octet_count = 0;

  /* set random source */
  x917_prng.rand = random_source;
  
  /* initialize secret key from random source */
  status = random_source((uint8_t *)&tmp_key, 16);
  if (status) 
    return status;

  /* expand aes key */
  aes_expand_encryption_key(&tmp_key, x917_prng.key);

  /* initialize prng state from random source */
  status = x917_prng.rand((uint8_t *)&x917_prng.state, 16);
  if (status) 
    return status;

  return err_status_ok;
}

err_status_t
x917_prng_get_octet_string(uint8_t *dest, uint32_t len) {
  uint32_t t;
  v128_t buffer;
  int i, tail_len;
  err_status_t status;

  /* 
   * if we need to re-initialize the prng, do so now 
   *
   * avoid overflows by subtracting instead of adding
   */
  if (x917_prng.octet_count > MAX_PRNG_OUT_LEN - len) {
    status = x917_prng_init(x917_prng.rand);    
    if (status)
      return status;
  }
  x917_prng.octet_count += len;
  
  /* find out the time */
  t = time(NULL);
  
  /* loop until we have output enough data */
  for (i=0; i < len/16; i++) {
    
    /* exor time into state */
    x917_prng.state.v32[0] ^= t; 
 
    /* copy state into buffer */
    v128_copy(&buffer, &x917_prng.state);

    /* apply aes to buffer */
    aes_encrypt(&buffer, x917_prng.key);
    
    /* write data to output */
    *dest++ = buffer.v8[0];
    *dest++ = buffer.v8[1];
    *dest++ = buffer.v8[2];
    *dest++ = buffer.v8[3];
    *dest++ = buffer.v8[4];
    *dest++ = buffer.v8[5];
    *dest++ = buffer.v8[6];
    *dest++ = buffer.v8[7];
    *dest++ = buffer.v8[8];
    *dest++ = buffer.v8[9];
    *dest++ = buffer.v8[10];
    *dest++ = buffer.v8[11];
    *dest++ = buffer.v8[12];
    *dest++ = buffer.v8[13];
    *dest++ = buffer.v8[14];
    *dest++ = buffer.v8[15];

    /* exor time into buffer */
    buffer.v32[0] ^= t;

    /* encrypt buffer */
    aes_encrypt(&buffer, x917_prng.key);

    /* copy buffer into state */
    v128_copy(&x917_prng.state, &buffer);
    
  }
  
  /* if we need to output any more octets, we'll do so now */
  tail_len = len % 16;
  if (tail_len) {
    
    /* exor time into state */
    x917_prng.state.v32[0] ^= t; 
 
    /* copy value into buffer */
    v128_copy(&buffer, &x917_prng.state);

    /* apply aes to buffer */
    aes_encrypt(&buffer, x917_prng.key);

    /* write data to output */
    for (i=0; i < tail_len; i++) {
      *dest++ = buffer.v8[i];
    }

    /* now update the state one more time */

    /* exor time into buffer */
    buffer.v32[0] ^= t;

    /* encrypt buffer */
    aes_encrypt(&buffer, x917_prng.key);

    /* copy buffer into state */
    v128_copy(&x917_prng.state, &buffer);

  }
  
  return err_status_ok;
}

err_status_t
x917_prng_deinit(void) {
  
  return err_status_ok;  
}

#ifdef WINCE
/* Epoch base year */
#define EPOCH_YEAR 1970

/* tm struct members conversion units */
#define TM_YEAR_BASE    1900    /* tm_year base year */
#define TM_MONTH_MIN    0       /* tm_mon = 0 - January */ 
#define TM_MONTH_MAX    11      /* tm_mon = 11 - December */ 

#define MIN_SEC         60                  /* seconds in a minute */
#define HOUR_SEC        3600                /* seconds in an hour */
#define DAY_SEC         86400               /* seconds in a day */
#define YEAR_SEC        (365 * DAY_SEC)     /* seconds in a year */
#define FOUR_YEAR_SEC   (4 * YEAR_SEC + 1)  /* seconds in a 4-year period */

/* Checks if given year is a leap year. */
#define IS_LEAP_YEAR(year) \
    (((year) % 4) == 0 && (((year) % 100) != 0 || ((year) % 400) == 0))

// Get unix timestamp for a GMT date
static time_t gmmktime(struct tm *tmbuff)
{
    static const int monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    time_t tres = 0;
    int doy = 0;
    int i = 0;
    time_t _loctime_offset = 0;

    /* We do allow some ill-formed dates, but we don't do anything special
    with them and our callers really shouldn't pass them to us.  Do
    explicitly disallow the ones that would cause invalid array accesses
    or other algorithm problems. */
    if (tmbuff->tm_mon < 0 || tmbuff->tm_mon > 11 || tmbuff->tm_year < (EPOCH_YEAR - TM_YEAR_BASE))
    {
        return (time_t) -1;
    }

    /* Convert calender time to a time_t value. */
    tres = 0;

    /* Sum total amount of days from the Epoch with respect to leap years. */
    for (i = EPOCH_YEAR; i < tmbuff->tm_year + TM_YEAR_BASE; i++)
    {
        tres += 365 + IS_LEAP_YEAR(i);
    }

    /* Add days of months before current month. */
    doy = 0;
    for (i = 0; i < tmbuff->tm_mon; i++)
    {
        doy += monthDays[i];
    }
    tres += doy;
    
    /* Day of year */
    tmbuff->tm_yday = doy + tmbuff->tm_mday;

    if (tmbuff->tm_mon > 1 && IS_LEAP_YEAR(tmbuff->tm_year + TM_YEAR_BASE))
    {
        tres++;
    }
    
    /* Add days of current month and convert to total to hours. */
    tres = 24 * (tres + tmbuff->tm_mday - 1) + tmbuff->tm_hour;

    /* Add minutes part and convert total to minutes. */
    tres = 60 * tres + tmbuff->tm_min;

    /* Add seconds part and convert total to seconds. */
    tres = 60 * tres + tmbuff->tm_sec;
    
    /* For offset > 0 adjust time value for timezone
    given as local to UTC time difference in seconds). */
    tres += _loctime_offset;
    
    return tres;
}

// Return the value of time in seconds since the Epoch
static time_t time(time_t *timer)
{
    time_t t;
    struct tm tmbuff;
    SYSTEMTIME st;

    // Retrive current system date time as UTC
    GetSystemTime(&st);

    // Build tm struct based on SYSTEMTIME values
    tmbuff.tm_year = st.wYear - TM_YEAR_BASE;
    tmbuff.tm_mon = st.wMonth - 1;      // wMonth value 1-12
    tmbuff.tm_mday = st.wDay;
    tmbuff.tm_hour = st.wHour;
    tmbuff.tm_min = st.wMinute;
    tmbuff.tm_sec = st.wSecond;
    tmbuff.tm_isdst = 0;    // always 0 for UTC time
    tmbuff.tm_wday = st.wDayOfWeek;
    tmbuff.tm_yday = 0;

    // Convert tm struct to time_t
    t = gmmktime(&tmbuff);

    // Assign time value
    if (timer != NULL) {
        *timer = t;
    }

    return t;
}
#endif

