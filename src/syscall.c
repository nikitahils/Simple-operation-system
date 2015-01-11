#include "syscall.h"
#include "isr.h"
#include "screen.h"
#include "panic.h"
#include "module_loader.h"
#include "module.h"
#include "keyboard.h"

#define SYSCALL_COUNT (sizeof(syscalls)/sizeof(int))

static void syscall_handler(registers_t *regs);

static void* syscalls[] = {
        putchar,                //set console's putchar
        get_keyboard_key,
        alloc_module_data_page,
        free_module_data_page,
        exit_module,
};

void initialise_syscalls()
{
        register_interrupt_handler (0x80, &syscall_handler);
}

void syscall_handler(registers_t *regs)
{
        u32int syscall_num = regs->eax;
        if (syscall_num < SYSCALL_COUNT) {

                void *syscall = syscalls[regs->eax];

                int ret;
                asm volatile ("push  %1    \n\t"
                              "push  %2    \n\t"
                              "push  %3    \n\t"
                              "push  %4    \n\t"
                              "push  %5    \n\t"
                              "call *%6    \n\t"
                              "pop   %%ecx \n\t"
                              "pop   %%ecx \n\t"
                              "pop   %%ecx \n\t"
                              "pop   %%ecx \n\t"
                              "pop   %%ecx \n\t" : "=a" (ret) : "r" (regs->edi), "r" (regs->esi),
                                      "r" (regs->edx), "r" (regs->ecx),
                                      "r" (regs->ebx), "r" (syscall));
                regs->eax = ret;
        }
}
