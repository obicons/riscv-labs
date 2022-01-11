# Lab 0: Toolchain Setup

The goal of this lab is to setup your development toolchain for
RISCV. We run a cross compiler to build code for RISCV in a Docker
container. We use the QEMU virtual machine to run RISCV code. We use
Das U-Boot to boot code.

Before class try to:
* Install Docker
* Build the cross compiler
* Install QEMU

## Cross Compiler

*Prerequisites*
* Install [Docker](https://www.docker.com/get-started).

Your computer (probably an x86, or maybe an ARM) might already have a
C compiler installed. This compiler is **not suffcient** for RISCV
native development for a few reasons. First, it targets the wrong
architecture. We need a compiler that targets RISCV, not your
computer's architecture. Second, it's going to create binaries that
depend on your operating system's application execution environment
(AEE).

Building a cross compiler involves compiling a compiler that targets
the desired architecture. But this tends to be difficult. There are
many dependencies in the build process, and then you need to manage
the installation once it's complete. Rather than subject you to this
~torture~ painful endeavor, we provide a Dockerfile that does the work
for you.

A Dockerfile is a recipe to build a Docker container image. You can
think of a Docker container image as a VM image, but that's not really
true. Docker containers are kind of like VMs (but that's also not
really true.) We'll build a Docker container image that contains our
RISCV cross compiler. Then, we'll be able to build code for RISCV by
mounting the source code directory into a Docker container, and then
compiling using our cross compiler.

Make sure the Docker daemon is running. Then, change directories to
this lab. Now, run the command: `docker image build -t riscv-cc
./`. This will start the build process for your cross
compiler. Heads up: this will take a *long* time!

## Das U-Boot

We need a way to load our applications onto a RISCV board. We're going
to use [Das U-Boot](https://www.denx.de/wiki/U-Boot). Das U-Boot is a
popular boot loader for Linux environments, but it can also boot other
things.

The U-Boot source tree is included as a git submodule in this lab's
source directory. Make sure that it's present by running the command
`git submodule update --init --recursive`.

Change directories to Das U-Boot. Then, Start a Docker container
running the image you made in the Cross Compiler section. You can use
this command: `docker container run -ti -v "$(pwd)":/src/ riscv-cc
bash`. This command will start a new Docker container. Then, you can
build Das U-Boot by running these commands:
```sh
$ cd /src/
$ CROSS_COMPILE=/opt/riscv/bin/riscv64-unknown-linux-gnu- make qemu-riscv64_defconfig
$ CROSS_COMPILE=/opt/riscv/bin/riscv64-unknown-linux-gnu- make
```

This compiles Das U-Boot for QEMU's RISCV virtual board. You may now
exit the container (just type exit or Control-D.)

## QEMU

We use QEMU version 6.1.0 as our RISCV VM for testing. You need to
install QEMU. You can do this on a Mac using
[homebrew](https://brew.sh), and on Linux using your package manager. 

Once you've installed QEMU, you can run das u-boot by executing the
command `qemu-system-riscv64 -nographic -machine virt -bios u-boot` in
the das u-boot directory. You should see output that looks similar to this:
```sh
U-Boot 2022.01 (Jan 11 2022 - 13:26:15 +0000)

CPU:   rv64imafdcsu
Model: riscv-virtio,qemu
DRAM:  128 MiB
Core:  17 devices, 9 uclasses, devicetree: board
Flash: 32 MiB
Loading Environment from nowhere... OK
In:    uart@10000000
Out:   uart@10000000
Err:   uart@10000000
Net:   No ethernet found.
```

You can quit QEMU by typing Ctr+a x.

## Hello, world!

Let's build and run a simple application to make sure everything is
working. 

First, you need to build the hello world application. Change
directories to `hello`. Then, run the command `docker container run
-ti -v "$(pwd)":/src/ riscv-cc bash`. Inside the container, run:
```sh
$ cd /src/
$ riscv64-unknown-elf-gcc -mcmodel=medany -g -ffreestanding -O0 -Wl,--gc-sections  -nostartfiles -nostdlib -nodefaultlibs -Wl,-T,riscv64-virt.ld crt0.s hello.c -o hello
```
to compile. **Exit the container.**

Now, you can run this command in the `00_boot` directory to start
u-boot in QEMU: `qemu-system-riscv64 -nographic -machine virt -bios
u-boot-src/u-boot -drive file=fat:rw:hello,id=hd0 -device
virtio-blk-device,drive=hd0`

Run these uboot commands:
```
$ load virtio 0 ${kernel_addr_r} hello
$ bootelf ${kernel_addr_r}
```

You should see output like this:
```
U-Boot 2022.01 (Jan 11 2022 - 13:54:40 +0000)

CPU:   rv64imafdcsu
Model: riscv-virtio,qemu
DRAM:  128 MiB
Flash: 32 MiB
Loading Environment from nowhere... OK
In:    uart@10000000
Out:   uart@10000000
Err:   uart@10000000
Net:   No ethernet found.
Hit any key to stop autoboot:  0 
=> load virtio 0 ${kernel_addr_r} hello
7504 bytes read in 4 ms (1.8 MiB/s)
=> bootelf ${kernel_addr_r}
## Starting application at 0x80000000 ...
Hello world
## Application terminated, rc = 0x0
```
