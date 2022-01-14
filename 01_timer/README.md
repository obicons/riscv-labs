# Lab 01: Timer and Interrupts [WIP]

In this lab, we'll look at how interrupts work on RISCV. We'll
implement a simple interrupt handler, and learn how to use that to
create a multitasking system. 

## Background

CPUs trigger interrupts when events need attention. There are
essentially two categories:
* *Synchronous* interrupts (i.e. exceptions) that occur as a result of
  the software doing something. For example, when the CPU divides by
  zero.
* *Asynchronous* interrupts that occur when an external event occurs.
The CPU executes a user-defined function when interrupts occur. We'll
look more at the specifics later

Timers are an important source of asynchronous interrupts. Timers are
commonly used to implement multitasking. The OS maintains several
tasks. When a timer interrupt occurs, the OS saves the task's
*register file* and switches to another task. The *register file* is
the collection of registers the hardware exposes to users.
	
## Implementation

How to handle interrupts on RISCV?

RISCV exposes control and status registers (CSR) to configure the
CPU. These include the `mstatus` (for "machine status") that
contains a number of flags. Please look at [the official
documentation](https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf)
to better understand this register in particular. It's important!

We must use assembly language to control CSRs. To read a CSR, we use
the `CSRRC` instruction (for "control and status register, read and
clear bits".) For example, we can read the `mstatus` register into the
register `a0` with the instruction `CSRRC a0, mstatus,
zero`. `CSRRC`'s last operand specifies the bits to clear. We provide
the value of zero so as to not modify the register. We write to CSRs
with the `CSRRW` instruction (for control and status register, read
and write). For example, we can write the `a0` register to the
`mstatus` register with the instruction `CSRRW zero, mstatus,
a0`. `CSRRW`'s first operand is the output register to store the
previous CSR's value, the middle operand is the CSR, and the last
operand is the register that contains the new value of the CSR.

For convenience, we can interact with CSRs using these macros:
```
#define READ_CSR(name) ({                                       \
	    unsigned long result = 0;                               \
	    asm volatile("csrrc %0," #name ",zero" : "=r"(result)); \
	    result;                                                 \
})

#define WRITE_CSR(name, val) ({asm volatile("csrrw zero," #name ",%0" : :"r"(val));})
```
The `READ_CSR` uses [the GCC statement expression
extension](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html) to
evaluate to the register that we read. The `#` operator transforms the
argument to the macro into a string. For instance, we can read the
`mstatus` register with `READ_CSR(mstatus)`.

The first thing we need to do to make interrupts work is enable
them. Again, the [RISCV privileged instruction
set](https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf)
is an invaluable resource to help learn how to do this. There are two
steps:
1. Set the MIE (machine interrupt enabled) bit in `mstatus` register.
2. Set the MTIE (machine timer interrupt enabled) bit in the `mie`
   (machine interrupt enabled) register.
To disable interrupts, you'll clear the same bits.

### Activity
Write two functions `enable_interrupts` and `disable_interrupts` that
enable and disable interrupts. Inside of `main`, call
`enable_interrupts`.

RISCV provides two memory mapped registers for timer-based
interrupts. The addresses of these registers are defined by the
specific board. The registers are named `mtime` and `mtimecmp`,
respectively. `mtime` provides a real-time clock. `mtimecmp` stores a
value. When `mtime >= mtimecmp` a timer interrupt is triggered.

Qemu emulates the SiFive board's interrupt system by default. You can
read about SiFive's implementation [here](https://www.manualslib.com/manual/1477612/Sifive-E31.html?page=24).

### Activity
Write two functions `uint64_t get_time()` and `void
set_timer(uint64_t offset)` to get the current value of the `mtime`
register and store an **offset** into the `mtimecmp` register.

Interrupts are handled by the trap handler. On RISCV, the address of
the trap handler is stored in the `mtvec` register. Again, you
can read more about this in the [privileged instruction set
documentation](https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf). **Important:**
you need to 4-byte align the address of the interrupt handler.

In GCC, you can write a simple interrupt handler like this:
```
__attribute__((interrupt,aligned(4))) void interrupt_handler() {
    puts("Hello world!\n");
}
```
The `interrupt` attribute tells GCC that the function is an interrupt
handler. This causes GCC to emit code to save the CPU's register file
before calling `interrupt_handler`. GCC also creates code to restore
the CPU's register file when `interrupt_handler` returns.

### Activity
Use `interrupt_handler` to print a message to the screen every second.

## Build a simple task system
Let's use our timer interrupts to build a simple task system. We'll
create a task struct that stores the state of each task. Then, we'll
set a timer and let each task run for a fixed amount of time. When the
timer goes off, we'll save the task's state and switch to the next
task. This allows our operating system to multitask.

We define our task struct like this:
```
struct RegisterFile {
    unsigned long registers[32];
    unsigned long fregisters[32];
    unsigned long satp;
    void *trap_stack;
    unsigned long hart_id;
};

struct Task {
    unsigned long pc;
    struct RegisterFile register_file;
    void (*start)();
    enum State state;
};
```
The `pc` member stores the current program counter. The `start` member
stores the address of the function that the task runs. The `RegisterFile`
struct has space for the 32 general purpose registers, the 32 floating
point registers, and a few other fields we won't use yet.

We need to save the register file when the timer fires, but GCC
already did that. So, we need a new interrupt handler. Since we need
to save the register file, we have to write assembly. I put the
assembly that does this in `crt0.s`. You can refer to the function as
`extern void m_trap_vector()`. `m_trap_vector()` calls a C function
with the signature 
```
unsigned long handle_trap(unsigned long epc,
                          unsigned long tval,
                          unsigned long cause,
                          unsigned long hart,
                          unsigned long status,
                          struct RegisterFile *registers);
```
The return value of `handle_trap` is the new PC. The contents of the
register file are copied back into the registers.

### Activity
Write the body of `handle_trap` to switch between tasks that are
stored in the `tasks` global array. Test your code by defining a
couple simple tasks that print a single character.

## Conclusion
We've learned how interrupts work on RISCV. We used the timer
interrupts to create a multitasking system. Next, we'll learn how we
can protect the memory space of each task by using virtual memory.

## Resources
- https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf
- https://chromitem-soc.readthedocs.io/en/stable/clint.html
- https://github.com/riscv/riscv-aclint/blob/main/riscv-aclint.adoc
- https://five-embeddev.com/riscv-isa-manual/latest/csr.html
- https://twilco.github.io/riscv-from-scratch/2019/07/08/riscv-from-scratch-3.html
- https://qemu.readthedocs.io/en/latest/system/riscv/virt.html
- http://www.ittc.ku.edu/~heechul/courses/eecs388/lab6.pdf

## Notes:
* Interrupt controllers are platform-specific.
* Be careful! Your interrupt handler must be 4-byte aligned!
