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
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>    
#include <unistd.h> 
#include <sys/mman.h>
#include <string.h>
#include "targets.h"
#include "flush_reload.h"

#define ITER 100

int main(int argc, char **argv)
{
    uint64_t t;
    uint64_t avg, min, max;
    size_t results[2];
    uint8_t *fr_buf = mmap(NULL, 2*STRIDE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
    printf("fr_buf = %p\n", fr_buf);
    memset(fr_buf, 0x41, 2*STRIDE);

    //Print avg, min and max access time when cached
    avg = 0; min = 999999; max = 0;
    for(int i=0; i<ITER; i++) {
        maccess(&fr_buf[0]);
        fence();

        t = load_time(fr_buf);
        avg += t;
        if(t > max) max = t;
        if(t < min) min = t;
    }
    printf("Fast access: avg: %f min: %ld max: %ld\n", (float)avg/ITER, min, max);
    
    //Print avg, min and max access time when not cached
    avg = 0; min = 999999; max = 0;
    for(int i=0; i<ITER; i++) {
        clflush(fr_buf);
        fence();

        t = load_time(fr_buf);
        avg += t;
        if(t > max) max = t;
        if(t < min) min = t;
    }
    printf("Slow access: avg: %f min: %ld max: %ld\n", (float)avg/ITER, min, max);
    printf("Threshold: %d\n\n", THR);


    //Try to leak "pineapple-pizza" with selected threshold
    char *secret = "pineapple-pizza";
    uint8_t byte;

    for(int i=0; i<strlen(secret); i++) {

        byte = 0;
        for(int j=0; j<8; j++) {
            memset(results, 0, sizeof(results));

            for(int k=0; k<ITER; k++) {
                flush(fr_buf);
                maccess(&fr_buf[((secret[i]>>j)&1)*STRIDE]);
                fence();
                reload(fr_buf, results);
            }
            printf("(%3ld %3ld) ", results[0], results[1]);

            add_bit_leak(&byte, j, results);
        }

        printf("\n%x %c\n\n", byte, byte);
    }

    return 0;
}
