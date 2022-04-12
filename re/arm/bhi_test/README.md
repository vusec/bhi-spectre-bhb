# bhi\_test

## About

This test allow to verify if the system is affected by Spectre-BHB/Branch History Injection (BHI). 

To run `bhi_test` it is necessary to use a custom Android kernel with these changes:

* A modified syscall handler with a `clflush` to enlarge the transient window of the indirect branch present in `invoke_syscall`. This allows to observe more reliably the mistraining.
* A custom syscall `sys_bhi_hit` to load a provided pointer. This is necessary to leave a Flush+Reload trace to accurately observe if the misprediction happened.

For more details about these changes, please refer to the diff file `custom_syscalls.patch`

This experiment is meant to be run on a Pixel 6. For this reason ensure to have this device running with the **prebuilt** custom kernel by running:

```
sudo adb reboot bootloader
sudo fastboot boot boot.img
```

## How to run

```
make TARGET=PIXEL_6
adb push main.o /data/local/tmp
adb shell
su
cd /data/local/tmp
chmod +x main.o
taskset 10 ./main.o	#taskset ensures that the program is run on the Corte-X1 core
```



## Expected output

```
         fr_buf user: 0x00000071f32de000
         fr_buf kern: 0xffffff89778d8000
Fast access: avg: 101.500000 min: 97 max: 107
Slow access: avg: 562.309998 min: 534 max: 570
Threshold: 100

hits 906/1000
```

This means that by just controlling the user-space history we managed to redirect speculatively a kernel indirect branch
90% of times to an attack-controlled target.

## Troubleshooting

* Ensure that the Flush+Reload `THR` is correctly selected.

## How to compile the custom kernel

Get a clean kernel repository following [https://source.android.com/setup/build/building-kernels](https://source.android.com/setup/build/building-kernels):

```
mkdir android-kernel && cd android-kernel
repo init -u https://android.googlesource.com/kernel/manifest -b android-gs-raviole-5.10-android12-qpr1-d
repo sync
```

Then apply the patch to add the custom `bhi_hit` syscall:

```
cd aosp
git apply ../../custom_syscall.patch
```

Finally start the compilation:

```
cd ..
BUILD_KERNEL=1 KERNEL_CMDLINE="nokaslr nohlt" ./build_slider.sh
```

Use [Magisk](https://github.com/topjohnwu/Magisk) to root the compiled kernel `out/mixed/dist/boot.img` and then boot using

```
sudo adb reboot bootloader
sudo fastboot boot boot.img
```
