char getchar() {
	char key;
	asm("mov $0x1, %eax\n\t"
	    "int $0x80\n\t");
	asm ("mov %%eax, %%ebx":"=b"(key):);
        return key;
}

void putchar(char c) {
	asm("mov $0x0, %%eax\n\t"
	    "int $0x80\n\t"::"b"(c));
}

void exit(int status) {
	asm("mov $0x4, %%eax\n\t"
	    "int $0x80\n\t"::"b"(status));
}
