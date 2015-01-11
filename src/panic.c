#include "panic.h"

void panic(const char *message, const char *file, u32int line)
{
        IRQ_OFF;
        printf("PANIC(%s) at %s:%u\n", message, file, line);
        for(;;);
}

void panic_assert(const char *file, u32int line, const char *desc)
{
        IRQ_OFF;
        printf("ASSERT(%s) at %s:%u\n", desc, file, line);
        for(;;);
}
