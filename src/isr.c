#include "isr.h"
#include "panic.h"
#include "module.h"

extern module_info_t module_info;
isr_t interrupt_handlers[256];
static const char *exception_messages[32] = {
	"Divide-by-zero error",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Bound range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack-segment fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",
	"Floating-point exception",
	"Alignment check",
	"Machine check",
	"SIMD floating-point exception",
	"Virtualization exception",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Unknown interrupt",
	"Security exception",
	"Unknown interrupt"
};

void register_interrupt_handler(u8int n, isr_t handler)
{
        interrupt_handlers[n] = handler;
}

void isr_handler(registers_t regs)
{
        // This line is important. When the processor extends the 8-bit interrupt number
        // to a 32bit value, it sign-extends, not zero extends. So if the most significant
        // bit (0x80) is set, regs.int_no will be very large (about 0xffffff80).
        u8int int_no = regs.int_no & 0xFF;
        if (interrupt_handlers[int_no] != 0) {
                isr_t handler = interrupt_handlers[int_no];
                handler(&regs);
        } else {
                char *place = (module_info.running)? "module" : "kernel";
                printf("0x%x:%s in %s\n", int_no, exception_messages[int_no], place);
                printf("(cs:0x%x  eip:0x%x  ss:0x%x  esp:0x%x  eflags:0x%x)\n", regs.cs, regs.eip, regs.ss, regs.esp, regs.eflags);
                if (module_info.running)
                        exit_module();
                PANIC("unhandled interrupt...");
        }
}

void irq_handler(registers_t regs)
{
        // Send an EOI (end of interrupt) signal to the PICs.
        // If this interrupt involved the slave.
        if (regs.int_no >= 40) {
                // Send reset signal to slave.
                outb(0xA0, 0x20);
        }
        // Send reset signal to master. (As well as slave, if necessary).
        outb(0x20, 0x20);

        if (interrupt_handlers[regs.int_no] != 0) {
                isr_t handler = interrupt_handlers[regs.int_no];
                handler(&regs);
        }

}
