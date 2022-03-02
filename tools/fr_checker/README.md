# fr\_checker

## About

This tool helps to verify that the timing threshold for the Flush+Reload side channel is correctly selected.

This threshold is stored inside the macro `THR` present in `common/targets.h`.

**IMPORTANT:** All `re` experiments are based on this side channel. So it is essential to verify that Flush+Reload works reliably on the tested machine before running other experiments.

## How to run

```
make TARGET=INTEL_10_GEN #or INTEL_11_GEN, PIXEL_6
./run.sh
```

## Expected output

```
fr_buf = 0x7f3462cda000
Fast access: avg: 25.059999 min: 24 max: 28
Slow access: avg: 254.979996 min: 110 max: 1500
Threshold: 160

(100   0) (100   0) (100   0) ( 99   0) (  0 100) (  0 100) (  0 100) (100   0) 
70 p

(  0 100) (100   0) (100   0) (  0 100) (100   0) (  0 100) (  0 100) (100   0) 
69 i

(100   0) (  0  99) (  0 100) (  0 100) (100   0) (  0 100) (  0 100) (100   0) 
6e n

(  0 100) (100   0) (  0 100) (100   0) (100   0) (  0 100) (  0 100) (100   0) 
65 e

(  0 100) (100   0) (100   0) (100   0) (100   0) (  0 100) (  0 100) (100   0) 
61 a

(100   0) (100   0) (100   0) (100   0) (  0 100) (  0 100) (  0 100) (100   0) 
70 p

(100   0) (100   0) (100   0) (100   0) (  0  99) (  0 100) (  0 100) (100   0) 
70 p

(100   0) (100   0) (  0 100) (  0 100) (100   0) (  0 100) (  0 100) (100   0) 
6c l

(  0 100) (100   0) (  0 100) (100   0) (100   0) (  0 100) (  0 100) (100   0) 
65 e

(  0 100) (100   0) (  0 100) (  0 100) (100   0) (  0 100) (100   0) (100   0) 
2d -

(100   0) (100   0) (100   0) (100   0) (  0 100) (  0 100) (  0 100) (100   0) 
70 p

(  0 100) (100   0) (100   0) (  0 100) (100   0) (  0 100) (  0 100) (100   0) 
69 i

(100   0) (  0 100) (100   0) (  0 100) (  0 100) (  0 100) (  0 100) (100   0) 
7a z

(100   0) (  0 100) (100   0) (  0 100) (  0 100) (  0 100) (  0 100) (100   0) 
7a z

(  0 100) (100   0) (100   0) (100   0) (100   0) (  0 100) (  0 100) (100   0) 
61 a

```

We use a 1-bit Flush+Reload, and for each bit we print how many times we leaked the bit `0` and `1`.

e.g.

```
bit0 = 0  bit1 = 0  bit2 = 0  bit3 = 0  bit4 = 1  bit5 = 1  bit6 = 1  bit7 = 0
(100   0) (100   0) (100   0) ( 99   0) (  0 100) (  0 100) (  0 100) (100   0) 
0b01110000 = 0x70 =  'p'
```

## Troubleshooting

* Ensure that that the governor is set to `performance`
* Ensure that the `THR` is a value between the fast and slow access time
