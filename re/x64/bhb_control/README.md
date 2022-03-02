# bhb\_control

## About

This experiment is a slightly modified version`bhb_brute_force` with the goal to find the minimum number of branches an attacker needs to control to to generate arbitrary BTB tags.

## How to run

```
make TARGET=INTEL_10_GEN #or INTEL_11_GEN
./run.sh
```

## Expected output

```
hits:      100/100      round:     9477 control: 29
hits:      100/100      round:      771 control: 28
hits:      100/100      round:    10351 control: 27
hits:      100/100      round:    33949 control: 26
hits:      100/100      round:    42307 control: 25
hits:      100/100      round:     4031 control: 24
hits:      100/100      round:     4723 control: 23
hits:      100/100      round:     3730 control: 22
hits:      100/100      round:      499 control: 21
hits:      100/100      round:     5973 control: 20
hits:      100/100      round:    24806 control: 19
hits:      100/100      round:     3440 control: 18
hits:      100/100      round:     6795 control: 17
hits:      100/100      round:     4724 control: 16
hits:      100/100      round:    25461 control: 15
hits:      100/100      round:     1817 control: 14
hits:      100/100      round:    37669 control: 13
hits:      100/100      round:    43534 control: 12
hits:      100/100      round:     9328 control: 11
hits:      100/100      round:    16252 control: 10
hits:      100/100      round:      273 control: 9
hits:      100/100      round:      756 control: 8
Unable to find collision with control=7
```

From this example output we can observe that by controlling only 8 branches source and destination addresses we successfully managed to find a BHB collision, while with only 7 branches this was not possible.


## Troubleshooting

* Ensure that that the governor is set to `performance`
* Ensure that the `THR` is correctly selected
