# bhi\_test

## About

This test allow to verify if the system is affected by Spectre-BHB/Branch History Injection (BHI). 


## How to run

Before running this experiments, please verify that:

* The `vm` is up and running. See [vm/README.md](vm/README.md) how to setup the VM.
* The system under test supports eIBRS and IBRS as mitigation. You can verify this using [tools/ibrs_checker](../../../tools/ibrs_checker)

To execute this experiment start by compiling the experiment:

```
make TARGET=INTEL_10_GEN # or INTEL_11_GEN
```

Then by running the VM: 

```
cd vm
./update-img.sh	#to copy in the rootfs the experiment
./run_vm.sh
```

Then inside the VM, login using the `root` password and then execute:
```
cd bhi_test
./run.sh
```

## Expected output

```
         fr_buf user: 0x00007f92c967e000
         fr_buf kern: 0xffff9d4183acc000
hits = 875 / 1000
```

This means that by just controlling the user-space history we managed to redirect speculatively a kernel indirect branch 87% of times to an attack-controlled target.

## Troubleshooting

* Ensure that in the host the governor is set to `performance`.
* Ensure that the Flush+Reload `THR` is still valid inside the VM. To do so it is sufficient to run `fr_checker` inside the VM.
