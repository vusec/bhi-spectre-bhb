# ibrs\_checker

## About

This tool verifies the hardware support for IBRS and eIBRS mitigation.
More info on these defenses:

* [Intel](https://www.intel.com/content/www/us/en/developer/articles/technical/software-security-guidance/technical-documentation/speculative-execution-side-channel-mitigations.html)
* [AMD](https://developer.amd.com/wp-content/resources/Architecture_Guidelines_Update_Indirect_Branch_Control.pdf)

## How to run

```
make TARGET=INTEL_10_GEN # or INTEL_11_GEN
./run.sh
```

## Expected output

On a system with both IBRS and eIBRS support:

```
cpu_support_ibrs:      1
cpu_support_eibrs:     1
cpu_(e)ibrs_enabled:   1
```

On a system with only IBRS support:

```
cpu_support_ibrs:      1
cpu_support_eibrs:     0
cpu_(e)ibrs_enabled:   0
```

## Troubleshooting

* `rdmsr: can't open /dev/cpu/0/msr`
	* Install dependencies: `sudo apt install msr-tools`.
	* The program must be run on the host and not in the VM. 
