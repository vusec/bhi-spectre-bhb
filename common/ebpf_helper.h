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

#ifndef __HELPER__EBPF__
#define __HELPER__EBPF__

#include <linux/bpf.h>
#include <linux/filter.h>
#include <linux/ip.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h> 
#include <assert.h>
#include <time.h>
#include <assert.h>
#include <err.h>
#include <stdint.h>
#include <linux/bpf.h>
#include <linux/filter.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <asm/unistd_64.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <linux/bpf.h>
#include <linux/filter.h>
#include <linux/ip.h>

#define _GNU_SOURCE
#define GPLv2 "GPL v2"

#define ARRSIZE(x) (sizeof(x) / sizeof((x)[0]))

const char *str =
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

/* registers */
/* caller-saved: r0..r5 */
#define BPF_REG_ARG1    BPF_REG_1
#define BPF_REG_ARG2    BPF_REG_2
#define BPF_REG_ARG3    BPF_REG_3
#define BPF_REG_ARG4    BPF_REG_4
#define BPF_REG_ARG5    BPF_REG_5
#define BPF_REG_CTX     BPF_REG_6
#define BPF_REG_FP      BPF_REG_10

#define BPF_RAW_INSN(CODE, DST, SRC, OFF, IMM)          \
    ((struct bpf_insn) {                    \
     .code  = CODE,                  \
     .dst_reg = DST,                 \
     .src_reg = SRC,                 \
     .off   = OFF,                   \
     .imm   = IMM })

#define BPF_LD_IMM64_RAW(DST, SRC, IMM)         \
  ((struct bpf_insn) {                          \
    .code  = BPF_LD | BPF_DW | BPF_IMM,         \
    .dst_reg = DST,                             \
    .src_reg = SRC,                             \
    .off   = 0,                                 \
    .imm   = (__u32) (IMM) }),                  \
  ((struct bpf_insn) {                          \
    .code  = 0, /* zero is reserved opcode */   \
    .dst_reg = 0,                               \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = ((__u64) (IMM)) >> 32 })
#define BPF_LD_MAP_FD(DST, MAP_FD)              \
  BPF_LD_IMM64_RAW(DST, BPF_PSEUDO_MAP_FD, MAP_FD)
#define BPF_LDX_MEM(SIZE, DST, SRC, OFF)        \
  ((struct bpf_insn) {                          \
    .code  = BPF_LDX | BPF_SIZE(SIZE) | BPF_MEM,\
    .dst_reg = DST,                             \
    .src_reg = SRC,                             \
    .off   = OFF,                               \
    .imm   = 0 })
#define BPF_MOV64_REG(DST, SRC)                 \
  ((struct bpf_insn) {                          \
    .code  = BPF_ALU64 | BPF_MOV | BPF_X,       \
    .dst_reg = DST,                             \
    .src_reg = SRC,                             \
    .off   = 0,                                 \
    .imm   = 0 })
#define BPF_ALU64_REG(OP, DST, SRC)             \
    ((struct bpf_insn) {                        \
     .code  = BPF_ALU64 | BPF_OP(OP) | BPF_X,   \
     .dst_reg = DST,                            \
     .src_reg = SRC,                            \
     .off   = 0,                                \
     .imm   = 0 })
#define BPF_ALU64_IMM(OP, DST, IMM)             \
  ((struct bpf_insn) {                          \
    .code  = BPF_ALU64 | BPF_OP(OP) | BPF_K,    \
    .dst_reg = DST,                             \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = IMM })
#define BPF_ALU32_IMM(OP, DST, IMM)             \
  ((struct bpf_insn) {                          \
    .code  = BPF_ALU | BPF_OP(OP) | BPF_K,    \
    .dst_reg = DST,                             \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = IMM })
#define BPF_STX_MEM(SIZE, DST, SRC, OFF)        \
  ((struct bpf_insn) {                          \
    .code  = BPF_STX | BPF_SIZE(SIZE) | BPF_MEM,\
    .dst_reg = DST,                             \
    .src_reg = SRC,                             \
    .off   = OFF,                               \
    .imm   = 0 })
#define BPF_ST_MEM(SIZE, DST, OFF, IMM)         \
  ((struct bpf_insn) {                          \
    .code  = BPF_ST | BPF_SIZE(SIZE) | BPF_MEM, \
    .dst_reg = DST,                             \
    .src_reg = 0,                               \
    .off   = OFF,                               \
    .imm   = IMM })
#define BPF_EMIT_CALL(FUNC)                     \
  ((struct bpf_insn) {                          \
    .code  = BPF_JMP | BPF_CALL,                \
    .dst_reg = 0,                               \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = (FUNC) })
#define BPF_JMP_IMM(OP, DST, IMM, OFF)          \
  ((struct bpf_insn) {                          \
    .code  = BPF_JMP | BPF_OP(OP) | BPF_K,      \
    .dst_reg = DST,                             \
    .src_reg = 0,                               \
    .off   = OFF,                               \
    .imm   = IMM })
#define BPF_EXIT_INSN()                         \
  ((struct bpf_insn) {                          \
    .code  = BPF_JMP | BPF_EXIT,                \
    .dst_reg = 0,                               \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = 0 })
#define BPF_LD_ABS(SIZE, IMM)                   \
  ((struct bpf_insn) {                          \
    .code  = BPF_LD | BPF_SIZE(SIZE) | BPF_ABS, \
    .dst_reg = 0,                               \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = IMM })
#define BPF_ALU64_REG(OP, DST, SRC)             \
  ((struct bpf_insn) {                          \
    .code  = BPF_ALU64 | BPF_OP(OP) | BPF_X,    \
    .dst_reg = DST,                             \
    .src_reg = SRC,                             \
    .off   = 0,                                 \
    .imm   = 0 })
#define BPF_MOV64_IMM(DST, IMM)                 \
  ((struct bpf_insn) {                          \
    .code  = BPF_ALU64 | BPF_MOV | BPF_K,       \
    .dst_reg = DST,                             \
    .src_reg = 0,                               \
    .off   = 0,                                 \
    .imm   = IMM })
#define BPF_LD_IMM64_RAW_FULL(DST, SRC, OFF1, OFF2, IMM1, IMM2) \
    ((struct bpf_insn) {                    \
        .code = BPF_LD | BPF_DW | BPF_IMM,      \
        .dst_reg = DST,                 \
        .src_reg = SRC,                 \
        .off   = OFF1,                  \
        .imm   = IMM1 }),           \
    ((struct bpf_insn) {                    \
        .code  = 0,                 \
        .dst_reg = 0,                   \
        .src_reg = 0,                   \
        .off   = OFF2,                  \
        .imm   = IMM2 })

//helpers
int bpf_(int cmd, union bpf_attr *attrs) {
    return syscall(__NR_bpf, cmd, attrs, sizeof(*attrs));
}

int prog_load(struct bpf_insn *insns, size_t insns_count) {
    char verifier_log[100000];
    union bpf_attr create_prog_attrs = {
        .prog_type = BPF_PROG_TYPE_SOCKET_FILTER,
        .insn_cnt = insns_count,
        .insns = (uint64_t)insns,
        .license = (uint64_t)GPLv2,
        .log_level = 1,
        .log_size = sizeof(verifier_log),
        .log_buf = (uint64_t)verifier_log
    };
    int progfd = bpf_(BPF_PROG_LOAD, &create_prog_attrs);
    int errno_ = errno;
    errno = errno_;
    if (progfd == -1) {
        printf("==========================\n%s==========================\n", verifier_log);
        err(1, "prog load");
    }
    return progfd;
}

int create_filtered_socket_fd(int progfd) {
    int socks[2];
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, socks))
        err(1, "socketpair");
    if (setsockopt(socks[0], SOL_SOCKET, SO_ATTACH_BPF, &progfd, sizeof(int)))
        err(1, "setsockopt");
    return socks[1];
}

void trigger_ebpf(int sockfd, int len) {
    if (sockfd == 0) err(1, "wrong socket");
    if (len > 256) err(1, "length too big");
    if (write(sockfd, str, len) != len)
        err(1, "write to proc socket failed");
}

int map_array_create(int value_size, int num_entries) {
    union bpf_attr create_map_attrs = {
        .map_type = BPF_MAP_TYPE_ARRAY,
        .key_size = 4,
        .value_size = value_size,
        .max_entries = num_entries
    };
    int mapfd = bpf_(BPF_MAP_CREATE, &create_map_attrs);
    if (mapfd == -1)
        err(1, "map create");
    return mapfd;
}

int map_jmptable_create(int num_entries) {
	union bpf_attr create_map_attrs = {
		.map_type = BPF_MAP_TYPE_PROG_ARRAY,
		.key_size = 4,
		.value_size = 4,
		.max_entries = num_entries
	};
	int mapfd = bpf_(BPF_MAP_CREATE, &create_map_attrs);
	if (mapfd == -1)
		err(1, "jmptable create");
	return mapfd;
}

void map_set(int mapfd, uint32_t key, uint64_t value) {
    union bpf_attr attr = {
        .map_fd = mapfd,
        .key    = (uint64_t)&key,
        .value  = (uint64_t)&value,
        .flags  = BPF_ANY,
    };

    int res = bpf_(BPF_MAP_UPDATE_ELEM, &attr);
    if (res)
        err(1, "map update elem");
}

void map_set_ptr(int mapfd, uint32_t key, void *ptr) {
	union bpf_attr attr = {
		.map_fd = mapfd,
		.key    = (uint64_t)&key,
		.value  = (uint64_t)ptr,
		.flags  = BPF_ANY,
	};

	int res = bpf_(BPF_MAP_UPDATE_ELEM, &attr);
	if (res)
		err(1, "map update ptr elem");
}

uint32_t map_get(int mapfd, uint32_t key) {
    uint32_t value = 0;
    union bpf_attr attr = {
        .map_fd = mapfd,
        .key    = (uint64_t)&key,
        .value  = (uint64_t)&value,
        .flags  = BPF_ANY,
    };
    int res = bpf_(BPF_MAP_LOOKUP_ELEM, &attr);
    if (res)
        err(1, "map lookup elem");
    return value;
}
#endif
