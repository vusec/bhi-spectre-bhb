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
#include "targets.h"
#include "helper.h"

uint64_t cpu_support_ibrs(void);
uint64_t cpu_support_eibrs(void);
uint64_t rdmsr(uint32_t reg, int cpu);
uint64_t cpu_ibrs_enabled(void);

int main(int argc, char **argv)
{
    printf("cpu_support_ibrs:      %lx\n", cpu_support_ibrs());
    printf("cpu_support_eibrs:     %lx\n", cpu_support_eibrs());
    printf("cpu_(e)ibrs_enabled:   %lx\n", cpu_ibrs_enabled());
    return 0;
}

//CPUID leaf=7, subleaf=0, edx[26]
//Enumerates support for indirect branch restricted speculation (IBRS) and the indirect branch pre-
//dictor barrier (IBPB). Processors that set this bit support the IA32_SPEC_CTRL MSR and the
//IA32_PRED_CMD MSR. They allow software to set IA32_SPEC_CTRL[0] (IBRS) and IA32_PRED_CMD[0]
//(IBPB).
uint64_t cpu_support_ibrs(void)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(&eax, &ebx, &ecx, &edx, 0x07, 0x0);
    return (edx>>26)&1;
}

//CPUID leaf=7, subleaf=0, edx[29]
//Enumerates support for the IA32_ARCH_CAPABILITIES MSR.
//
//MSR 10AH: IA32_ARCH_CAPABILITIES
//MSR[10AH][1] = IBRS_ALL: The processor supports enhanced IBRS.
uint64_t cpu_support_eibrs(void)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(&eax, &ebx, &ecx, &edx, 0x07, 0x0);
    if ( ((edx>>29)&1) != 1)
        return 0;
    return (rdmsr(0x10A, 0)>>1)&1;
}

//MSR 48H: IA32_SPEC_CTRL
//MSR[48H][0] = Indirect Branch Restricted Speculation (IBRS).
//              Restricts speculation of indirect branch.
uint64_t cpu_ibrs_enabled(void)
{
    if(cpu_support_ibrs())
        return (rdmsr(0x48, 0)>>0)&1;
    return 0;
}
