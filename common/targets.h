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

#ifndef __TARGET_H__
#define __TARGET_H__

#define X64                 (0)
#define ARMV8               (1)
//An history size big enought that should work on any microarch
#define MAX_HISTORY_SIZE    (256)

#if   defined INTEL_10_GEN
#define ARCH            (X64)
#define HISTORY_SIZE    (29)
#define THR             (160)
#define STRIDE          (4096)
#define PAGE_OFFSET     (0xffff888000000000ULL)

#elif defined INTEL_11_GEN
#define ARCH            (X64)
#define HISTORY_SIZE    (66)
#define THR             (60)
#define STRIDE          (4096)
#define PAGE_OFFSET     (0xff11000000000000ULL)

#elif defined PIXEL_6
#define ARCH            (ARMV8)
#define THR             (100)
#define STRIDE          (4096)
#define PHYS_OFFSET     (0x80000000LL)
#define PAGE_OFFSET     (0xffffff8000000000ULL)

#elif defined CUSTOM_UARCH
// HISTORY_SIZE: use re/bhb_size to find the value
// #define HISTORY_SIZE    (TODO)
// THR: use tools/fr_checker to find a good value
// #define THR             (TODO)
// #define ARCH            (TODO)
// #define STRIDE          (TODO)

#else
#error "Must specify a target"
#endif

#endif  //__TARGET_H__
