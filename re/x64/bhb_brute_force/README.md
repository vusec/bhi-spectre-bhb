# bhb\_brute\_force

## About

This experiment will perform BHB brute forcing to achieve a twofold objective:

* Discover if BHB collision can happen only in-place or also out-of-place
* Measure the average trials required before finding such collision

## How to run

Before compiling, set the the MACRO `IN_PLACE` to `1` or `0` to respectively run the experiment in-place or out-of-place.
The `IN_PLACE` macro is present in the [src/main.c](src/main.c) file.
After this, the experiment can be compiled and run as follow:

```
make TARGET=INTEL_10_GEN #or INTEL_11_GEN
./run.sh
```

## Expected output

With `IN_PLACE == 1` (in-place):

```
[Round        21955]hits:      100/100  round:    21955 ind_branches={0x7f472e1f391c,0x7f472e1f391c}
[Round        22560]hits:      100/100  round:    22560 ind_branches={0x7f472e1f391c,0x7f472e1f391c}
[Round         9385]hits:       99/100  round:     9385 ind_branches={0x7f472e1f391c,0x7f472e1f391c}
[Round         1589]hits:      100/100  round:     1589 ind_branches={0x7f472e1f391c,0x7f472e1f391c}
[Round         4921]hits:       99/100  round:     4921 ind_branches={0x7f472e1f391c,0x7f472e1f391c}
```
This means that, on average, after ~16k rounds the brute force managed to find in-place collisions.

With `IN_PLACE == 0` (out-of-place): 

```
[Round         8075]hits:      100/100  round:     8075 ind_branches={0x7fbb1378d96d,0x7fbb16b5b871}
[Round         1070]hits:      100/100  round:     1070 ind_branches={0x7fbb1378d96d,0x7fbb26c8417d}
[Round        10262]hits:      100/100  round:    10262 ind_branches={0x7fbb1378d96d,0x7fbadf711fd0}
[Round         4892]hits:      100/100  round:     4892 ind_branches={0x7fbb1378d96d,0x7fbb22fc9c2d}
[Round        10733]hits:      100/100  round:    10733 ind_branches={0x7fbb1378d96d,0x7fbac888e5c1}
```
This means that, on average, after ~16k rounds the brute force managed to find out-of-place collisions.

For both in-place and out-of-place experiments, the expected average iterations before a collision are:

* 16384 (2^14) for Intel 10th gen
* 131072 (2^17) for Intel 11th gen

## Troubleshooting

* Ensure that that the governor is set to `performance`
* Ensure that the `THR` is correctly selected
