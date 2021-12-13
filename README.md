# StOS

This is the StOS project - a current result of me mediocrely trying to write some OS-like thing


# Disclaimer

This file is only a temporary draft of what an actual readme will look like

The information on building, running, and etc may be not relevant or incorrect (yet)

# Building prerequisites

 - Nasm installed
 - GCC cross compilers installed for targets i686-elf-pc and x86_64-elf-pc and named respectively i686-elf-gcc and x86_64-elf-gcc which support both C and C++

If you have compliant build tools installed or these exact ones but just named differently, you can force make to use them as follows:
```bash
make CC32=my_cc_32bit_compiler CXX16=my_cpp_16bit_compiler ....
```
Analogously, you should specify compilers that produce 16, 32 and 64 bits object files from C and C++ sources
(Sorry, I don't [yet] know how to make configure scripts)

By default, the CC16 and CXX16 are obtained by adding the -m16 switch to CC32 and CXX32, so you don't have to specify them if such conditions suit you

# Running prerequisites

In order to run the OS without running it on a real machine, you should have at least one of the following installed:
 - qemu-system-x86_64
 - qemu-system-x86_64-headless
 - VirtualBox

By default, the maximum memory available to QEMU is set to be 6 GB

Note that it is allocated dynamically, not statically. That is, memory will be allocated as the guest OS will need more. But still you are free to change it by passing the appropriate parameter to the run command:
```bash
make run64 .... QEMU64_MEMORY_MB=N ....
```
Where N is a maximum amount of memory in MiB

Note also that there is indeed a run32 make target and QEMU32_MEMORY_MB switch as well, but they are present solely for debugging purposes. The OS won't run on 32 bit CPU because it is designed solely to target x86_64 architecture

# Building and running

When you first clone the repo, you may want to proceed with the following steps:

To build everything, run
```bash
make
```

Then create a small raw hard drive image in ./system by running
```bash
make system_wipe
```

Then you will need to create an MBR partition table, you can do this by running
```bash
make system_reset_mbr
```

Then, to actually write everything you've built to the disk, run
```bash
make system_burn_mbr
```

Finally, we are ready to run the result. To do this (in QEMU), run
```bash
make run64
```

Note that in case you wish to run the OS on a headless version of QEMU, you can do it with VNC:
```bash
make run64v
```
or
```bash
make run64v VNC_DISPLAY=1
```

After which, connect to the guest system with your preferable VNC viewer by connecting to localhost:5901 (for display 1, the default one is display 0)

# Cleaning

To clean the build results as well as temporary files, run
```bash
make clean
```

To wipe everithing out and bring the directory into a state just as when you first clone the repository, you can run
```bash
make clean_wipe
```

# Licensing

This repository contains source code used to build and debug the software. All the code within this repository is distributed under the GNU General Public Licence v3. This however does not apply to the XML files in stuff/gdb folder as they are freely available on the internet

See LICENSE for the full text of license and more information

