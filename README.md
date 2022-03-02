# Branch History Injection / Spectre-BHB

This repository contains reverse-engineering and exploits source code regarding the Spectre-BHB/Branch History Injection vulnerability.

For more information about our work:

* Project page: [https://vusec.net/projects/bhi-spectre-bhb](https://vusec.net/projects/bhi-spectre-bhb)
* [Intel security advisory](https://www.intel.com/content/www/us/en/developer/articles/technical/software-security-guidance/advisory-guidance/branch-history-injection.html)
* [ARM Security advisory](https://developer.arm.com/support/arm-security-updates/speculative-processor-vulnerability/spectre-bhb)

## Setup

Ensure that the required dependencies are installed depending on your system under test:

#### x86-64
 `sudo apt install gcc nasm msr-tools qemu-system-x86 debootstrap wget make`

#### Arm-Android
1. Download `android-ndk-r23b-linux.zip` from [https://developer.android.com/ndk/downloads/index.html](https://developer.android.com/ndk/downloads/index.html)
2.  `unzip android-ndk-r23b-linux.zip` in the top directory
3. Finally execute `source env.sh` to create the `CC_ANDROID` environment variable.

## How to compile

For every experiment/poc a corresponding `Makefile` is provided.
Depending on your target use:

```
make TARGET=INTEL_10_GEN      #e.g. for the Intel i7-10700K
make TARGET=INTEL_11_GEN      #e.g. for the Intel Xeon Silver 4310
make TARGET=PIXEL_6           #For the Pixel 6 Cortex-X1
```

In case the system under test is not supported, it is simply sufficient to edit [common/targets.h](common/targets.h) to add the required micro-architectural parameters.

## Repository organization

* [tools](tools): 
	* [ibrs_checker](tools/ibrs_checker): utility to test the presence of IBRS and eIBRS mitigations
	* [fr_checker](tools/fr_checker): tool to verify the the Flush+Reload timing threshold `THR` present in [tune.h](common/tune.h) is correctly selected (needed for the reverse engineering experiments).
	
* [pocs](pocs): end-to-end exploits demonstrating how Spectre-BHB primitive can be used to mount an attack to leak data from the Linux kernel.

	* [inter_mode](pocs/inter_mode): exploit based on Spectre-BHB/Branch History Injection demonstrates how the unprivileged history can be used to mount spectre-v2 attacks even in the presence of the ad-hoc hardware defense `eIBRS`.
	* [intra_mode](pocs/intra_mode): exploit that shows how same privilege mode exploitation (kernel to kernel) is not only possible but practical.

* [re](re): reverse engineering experiments to test the presence of *Branch History Injection* and to extrapolate the main parameter of the indirect branch predictor.
	* [x64](re/x64)
		* [bhi_test](re/x64/bhi_test): Experiment to verify the presence of the Spectre-BHB/Branch History Injection primitive
		* [bhb_brute_force](re/x64/bhb_brute_force): Experiment to observe how often with a brute-force approach we obtain a BHB-collision
		* [bhb_size](re/x64/bhb_size): Experiment to retrieve the size of the Branch History Buffer (BHB)
		* [bhb_control](re/x64/bhb_control): Experiment to discover what is the minimum required control of the BHB to obtain reliable exploitation
		
	* [arm](re/arm)
		* [bhi_test](re/arm/bhi_test): Experiment to verify the presence of the Spectre-BHB/Branch History Injection primitive
		* [bhb_brute_force](re/arm/bhb_brute_force): Experiment to observe how often with a brute-force approach we obtain a BHB-collision

