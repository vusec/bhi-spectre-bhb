# VM setup

## About

To run the `bhi_test` it is necessary to use a custom kernel with these changes:

* A modified syscall handler with a `clflush` to enlarge the transient window of the indirect branch present in `do_syscall_64`. This allows to observe more reliably the mistraining.
* A custom syscall `sys_bhi_hit` to load a provided pointer. This is necessary to leave a Flush+Reload trace to accurately observe if the misprediction happened.

For simplicity we use qemu-kvm to run such custom kernel.

For more details about these changes, please refer to the diff file `custom_syscalls.patch`

## Requirements
* User must be in  the `kvm` group and `/dev/kvm` should be present
* `sudo apt-get install build-essential libncurses-dev bison flex libssl-dev libelf-dev zstd qemu-system-x86 debootstrap wget`

## 1. Create a rootfs

We reuse [syzkaller](https://github.com/google/syzkaller) scripts to easily create a rootfs using the following commands:

```
wget https://raw.githubusercontent.com/google/syzkaller/master/tools/create-image.sh -O create-image.sh
chmod +x create-image.sh
./create-image.sh -a x86_64 -d stretch -s 4096
```

After this, an empty rootfs will be created. To copy inside the experiments, simply run

```
./update-img.sh
```

## 2. Compile the kernel

We provide a pre-compiled kernel (`bzImage`) to speed-up the process. Please refer to the section *"How to compile and patch the kernel"* for all the details on how this image was created.

## 3. Test the VM is up and running

### Run the VM

Simply use the `./run_vm.sh` script
The default password is `root`

### How to exit the VM

1. `CTRL+A` followed by `x`
2. `q` + `enter`


## How to compile and patch the kernel

To recreate the provided `bzImage`, please follow these steps:

First get the Linux kernel used in the evaluation:
``` 
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.14.10.tar.gz
tar xvf linux-5.14.10.tar.gz
```

Copy the provided kernel `.config`

```
cp .config linux-5.14.10/.config
```

This `.config` was obtained by changing `CONFIG_BPF_UNPRIV_DEFAULT_OFF` to `n`. Please notice that this was the default at the time of our research.
The initial `.config` was taken from the default one in Ubuntu using the following commands:
```
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.14.10/amd64/linux-headers-5.14.10-051410-generic_5.14.10-051410.202110070632_amd64.deb
dpkg-deb --fsys-tarfile linux-headers-5.14.10-051410-generic_5.14.10-051410.202110070632_amd64.deb | tar Ox --wildcards './usr/src/*/.config' > .config_ubuntu
```

Apply the patches

```
cd  linux-5.14.10
git apply ../custom_syscalls.patch
```

Finally compile the kernel

```
make -j16
```
