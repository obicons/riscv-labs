#ifndef SANIX_H
#define SANIX_H

enum State {
    OFF = 0,
    RUNNING
};

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

// Prints message over the console UART.
void puts(const char *);

// Prints the longer over the console UART.
void lprint(unsigned long);

#endif
