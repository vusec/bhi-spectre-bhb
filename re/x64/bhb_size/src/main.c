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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h> 
#include <assert.h>
#include <time.h>
#include <errno.h>
#include "targets.h"
#include "helper.h"
#include "flush_reload.h"

#define ITERS               1000

extern void fill_bhb(uint8_t *history, void *target, uint8_t *arg);
extern void ret_gadget(void);
extern void hit_gadget(uint8_t *arg);

int main(int argc, char **argv)
{
    uint8_t hit_history[MAX_HISTORY_SIZE];
    uint8_t ret_history[MAX_HISTORY_SIZE];
    uint8_t *fr_buf;
    int hits = 0;

    fr_buf = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    memset(fr_buf, 0x41, 0x1000);

    //Define once the target history
    for(int j=0; j<MAX_HISTORY_SIZE; j++) hit_history[j] = 1;
    //for(int j=0; j<MAX_HISTORY_SIZE; j++) ret_history[j] = 0;
    for(int j=0; j<MAX_HISTORY_SIZE; j++) ret_history[j] = 1;

    for(int h=0; h<MAX_HISTORY_SIZE; h++) {

        printf("History bit flip at %3d/%3d [bhb_size=%3d]    ", h, MAX_HISTORY_SIZE-1, MAX_HISTORY_SIZE-h);
        //ret_history[MAX_HISTORY_SIZE-1-h] = 1;
        ret_history[h] = 0;

        for(int i=0; i<ITERS; i++) {
            //Train hit_history -> hit_gadget
            fill_bhb(hit_history, &hit_gadget, fr_buf);

            //Flush
            clflush(fr_buf);

            //See if ret_history still collides
            fill_bhb(ret_history, &ret_gadget, fr_buf);

            //Reload
            fence();   //Ensure all memory operations are done
            if (load_time(fr_buf) < THR) {
                hits++;
            }
        }
        
        printf("hits: %d/%d\n", hits, ITERS);
        hits = 0;
    }


    return 0;
}

