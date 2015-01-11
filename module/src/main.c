#include "system.h"

char *str = "Hello world!";
int k     = 0xDEADFFFF;

void main(int ebx, int eax) {    
	char c;
        while(1) {
               c = getchar();
               if(c != 0x0) {
                    putchar(c);
                }
        }
	exit(0);    
}
