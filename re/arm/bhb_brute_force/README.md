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
make TARGET=PIXEL_6
adb push main.o /data/local/tmp                                                                                                                                                                                                                               
adb shell
cd /data/local/tmp                                                                                                                                                                                                                                            
chmod +x main.o                                                                                                                                                                                                                                               
taskset 10 ./main.o     #taskset ensures that the program is run on the Corte-X1 core 
```

## Expected output

With `IN_PLACE == 1`:

```
rwx mem: 0x732c633000
[Round 105] hits: 99
[Round 193] hits: 67
[Round 378] hits: 66
[Round 383] hits: 99
[Round 663] hits: 66
[Round 734] hits: 96
```
This means that, on average, after ~400 rounds the brute force managed to find in-place collisions.

With `IN_PLACE == 0` (out-of-place) instead we cannot find any collision


## Troubleshooting

* Ensure that the `THR` is correctly selected
