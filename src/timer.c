#include "timer.h"
#include "isr.h"

static void timer_callback(registers_t regs)
{
        static u32int tick = 0;
        tick++;
        printf("Tick: %d\n", tick);
}

void init_timer(u32int frequency)
{
        register_interrupt_handler(IRQ0, (isr_t)timer_callback);
        u32int divisor = 1193180 / frequency;

        outb(0x43, 0x36);
        u8int l = (u8int)( divisor & 0xFF);
        u8int h = (u8int)((divisor>>8) & 0xFF);
        outb(0x40, l);
        outb(0x40, h);
}
