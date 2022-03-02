# bhb\_size

## About

This experiment aim to discover  the BHB size, i.e., how many previous
branches are tracked

## How to run

```
make TARGET=INTEL_10_GEN #or INTEL_11_GEN
./run.sh
```

## Expected output

```
...
History bit flip at 219/255 [bhb_size= 37]    hits: 1000/1000
History bit flip at 220/255 [bhb_size= 36]    hits: 1000/1000
History bit flip at 221/255 [bhb_size= 35]    hits: 1000/1000
History bit flip at 222/255 [bhb_size= 34]    hits: 1000/1000
History bit flip at 223/255 [bhb_size= 33]    hits: 1000/1000
History bit flip at 224/255 [bhb_size= 32]    hits: 1000/1000
History bit flip at 225/255 [bhb_size= 31]    hits: 1000/1000
History bit flip at 226/255 [bhb_size= 30]    hits: 1000/1000
History bit flip at 227/255 [bhb_size= 29]    hits: 1/1000
History bit flip at 228/255 [bhb_size= 28]    hits: 3/1000
History bit flip at 229/255 [bhb_size= 27]    hits: 1/1000
History bit flip at 230/255 [bhb_size= 26]    hits: 1/1000
History bit flip at 231/255 [bhb_size= 25]    hits: 0/1000
History bit flip at 232/255 [bhb_size= 24]    hits: 0/1000
History bit flip at 233/255 [bhb_size= 23]    hits: 0/1000
History bit flip at 234/255 [bhb_size= 22]    hits: 0/1000
History bit flip at 235/255 [bhb_size= 21]    hits: 0/1000
History bit flip at 236/255 [bhb_size= 20]    hits: 0/1000
History bit flip at 237/255 [bhb_size= 19]    hits: 0/1000
History bit flip at 238/255 [bhb_size= 18]    hits: 0/1000
History bit flip at 239/255 [bhb_size= 17]    hits: 0/1000
History bit flip at 240/255 [bhb_size= 16]    hits: 0/1000
History bit flip at 241/255 [bhb_size= 15]    hits: 0/1000
History bit flip at 242/255 [bhb_size= 14]    hits: 0/1000
History bit flip at 243/255 [bhb_size= 13]    hits: 0/1000
History bit flip at 244/255 [bhb_size= 12]    hits: 0/1000
History bit flip at 245/255 [bhb_size= 11]    hits: 0/1000
History bit flip at 246/255 [bhb_size= 10]    hits: 1/1000
History bit flip at 247/255 [bhb_size=  9]    hits: 0/1000
History bit flip at 248/255 [bhb_size=  8]    hits: 0/1000
History bit flip at 249/255 [bhb_size=  7]    hits: 0/1000
History bit flip at 250/255 [bhb_size=  6]    hits: 0/1000
History bit flip at 251/255 [bhb_size=  5]    hits: 0/1000
History bit flip at 252/255 [bhb_size=  4]    hits: 0/1000
History bit flip at 253/255 [bhb_size=  3]    hits: 0/1000
History bit flip at 254/255 [bhb_size=  2]    hits: 0/1000
History bit flip at 255/255 [bhb_size=  1]    hits: 0/1000
```

From this example output we observe that when we flip the 29th branch before the indirect branch, the predictor stop mispredicting. This suggests that the BHB is capable to track only the latest 29 branches. 


## Troubleshooting

* Ensure that that the governor is set to `performance`
* Ensure that the `THR` is correctly selected
