FROM gcc:9.4.0-buster

# Step 1: create a RISCV cross compiler.
RUN apt-get update
RUN apt-get install -y \
    autoconf	       \
    automake	       \
    autotools-dev      \
    curl	       \
    python3	       \
    libmpc-dev	       \
    libmpfr-dev	       \
    libgmp-dev 	       \
    gawk	       \
    build-essential    \
    bison	       \
    flex	       \
    texinfo	       \
    gperf	       \
    libtool	       \
    patchutils	       \
    bc		       \
    zlib1g-dev	       \
    libexpat-dev       \
    python3	       \
    python3-dev

# We need this to build u-boot.
RUN git clone https://github.com/riscv/riscv-gnu-toolchain
WORKDIR riscv-gnu-toolchain
RUN git checkout 3c02b426c15d20c3b8dd677c2edbec228bc0717f
RUN ./configure --prefix=/opt/riscv &&	\
    make linux

# We need this to build our kernel.
RUN make clean &&				\
    ./configure --prefix=/opt/riscv-unknown &&	\
    make

ENV PATH=/opt/riscv-unknown/bin/:"$PATH"
