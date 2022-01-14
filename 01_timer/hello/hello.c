#define UART_OFFSET 0x10000000
#define VIRT_CLINT_BASE 0x2000000
#define VIRT_CLINT_TIME 0xbff8
#define VIRT_CLINT_TIMECMP 0x4000
#define STACK_SIZE_BYTES 1000

#include <stdint.h>
#include <stddef.h>
#include "sanix.h"

uint64_t get_time();

void enable_interrupts();

void disable_interrupts();

unsigned long handle_trap(unsigned long epc,
			  unsigned long tval,
			  unsigned long cause,
			  unsigned long hart,
			  unsigned long status,
			  struct RegisterFile *registers);

void set_timer(uint64_t offset);

void init_task(struct Task *t);

extern void m_trap_vector();

#define READ_CSR(name) ({						\
	    unsigned long result = 0;					\
	    asm volatile("csrrc %0," #name ",zero" : "=r"(result));	\
	    result;							\
})

#define WRITE_CSR(name, val) ({asm volatile("csrrw zero," #name ",%0" : :"r"(val));})

volatile int current_task;
static int initialized;

extern void run_a();
extern void run_b();

extern char __stack_top;

struct Task tasks[2];

int main(int argc, char *argv[]) {
    initialized = 0;
    current_task = 0;
    tasks[0] = (struct Task) {.start = run_a};
    tasks[1] = (struct Task) {.start = run_b};

    int num_tasks = sizeof(tasks) / sizeof(struct Task);
    for (uint64_t i = 1; i <= num_tasks; i++) {
	tasks[i - 1].register_file.registers[2] = (uint64_t) (&__stack_top - (i * STACK_SIZE_BYTES) - 10000000);
	puts("task stack: ");
	lprint(tasks[i - 1].register_file.registers[2]);
	puts("\n");

	init_task(&tasks[i-1]);
    }

    lprint(READ_CSR(mvendorid));
    puts("\n");

    lprint(READ_CSR(marchid));
    puts("\n");

    lprint(READ_CSR(mstatus));
    puts("\n");

    lprint(get_time());
    puts("\n");

    for (int i = 0; i < argc; i++) {
	puts(argv[i]);
	puts("\n");
    }

    int x;
    puts("Stack address: ");
    lprint((uint64_t) &x);
    puts("\n");

    enable_interrupts();
    set_timer(1000000);

    while (1) {}

    return 0;
}

void puts(const char *str) {
    char *uart = (char *) UART_OFFSET;
    while (*str) {
	(*uart) = (*str);
	str++;
    }
}

void lprint(unsigned long x) {
    char buff[28] = {0};
    int i = 0;
    do {
	buff[i++] = x % 10;
	x /= 10;
    } while (x);

    char *uart = (char *) UART_OFFSET;
    for (i = i - 1; i >= 0; i--) {
	(*uart) = buff[i] + '0';
    }
}

uint64_t get_time() {
    volatile uint64_t *mtime = (volatile uint64_t *) (VIRT_CLINT_BASE + VIRT_CLINT_TIME);
    return *mtime;
}

void set_timer(uint64_t offset) {
    volatile uint64_t *mtimecmp = (volatile uint64_t *) (VIRT_CLINT_BASE + VIRT_CLINT_TIMECMP);
    *mtimecmp = get_time() + offset;
}

void enable_interrupts() {
    // Enable M-Mode interrupts in the machine status CSR.
    const unsigned long mie_mask = 0x8;
    unsigned long status = READ_CSR(mstatus) | mie_mask;
    WRITE_CSR(mstatus, status);

    // Enable timer interrupts in the machine interrupt CSR.
    const unsigned long mtie_mask = 0x80;
    unsigned long mie_register = READ_CSR(mie) | (1 << 7);
    WRITE_CSR(mie, mie_register);

    WRITE_CSR(mtvec, m_trap_vector);
}

void disable_interrupts() {
    // Enable M-Mode interrupts in the machine status CSR.
    const unsigned long mie_mask = 0x8;
    unsigned long status = READ_CSR(mstatus) & ~mie_mask;
    WRITE_CSR(mstatus, status);

    // Enable timer interrupts in the machine interrupt CSR.
    const unsigned long mtie_mask = 0x80;
    unsigned long mie_register = READ_CSR(mie) & ~(1 << 7);
    WRITE_CSR(mie, mie_register);
}

unsigned long handle_trap(unsigned long epc,
			  unsigned long tval,
			  unsigned long cause,
			  unsigned long hart,
			  unsigned long status,
			  struct RegisterFile *registers) {
    disable_interrupts();
    const unsigned long code_mask = 0x1ff;
    unsigned long next_pc = epc;

    if ((cause & code_mask) == 7) {
	if (initialized) {
	    tasks[current_task].register_file = *registers;
	    tasks[current_task].pc = epc;
	    current_task = (current_task + 1) % (sizeof(tasks) / sizeof(struct Task));
	} else {
	    initialized = 1;
	}
	next_pc = tasks[current_task].pc;
	*registers = tasks[current_task].register_file;
    } else {
	lprint(epc);
	puts(" <-\n");
	while (1) {}
    }

    set_timer(100000);
    enable_interrupts();
    return next_pc;
}

void init_task(struct Task *t) {
    t->pc = (uint64_t) t->start;
}
