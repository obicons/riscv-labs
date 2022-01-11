#include <stdint.h>

#define UART_OFFSET 0x10000000

void puts(const char *);

int main(int argc, char *argv[]) {
    puts("Hello world\n");
    return 0;
}

void puts(const char *str) {
    char *uart = (char *) UART_OFFSET;
    while (*str) {
	(*uart) = (*str);
	str++;
    }
}
