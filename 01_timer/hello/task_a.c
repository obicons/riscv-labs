#include "sanix.h"

void run_a() {
    unsigned long x = 0;
    while (1) {
	if (x == 10000000) {
	    puts("a\n");
	    x = 0;
	}
	x++;
    }
}

void run_b() {
    unsigned long x = 0;
    while (1) {
	if (x == 10000000) {
	    puts("b\n");
	    x = 0;
	}
	x++;
    }    
}
