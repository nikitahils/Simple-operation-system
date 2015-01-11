#include "common.h"
#include "memory_manager.h"
#include "descriptor_tables.h"
#include "module_loader.h"
#include "paging.h"
#include "screen.h"
#include "keyboard.h"
#include "syscall.h"

void start_kernel(u32int code_base_addr,   u32int code_segment_len,
                  u32int data_base_addr,   u32int data_segment_len,
                  u32int module_base_addr, u32int module_segment_len)
{
        init_memory_manager(code_base_addr, code_segment_len, data_base_addr, data_segment_len, module_base_addr, module_segment_len);
        init_descriptor_tables();
        init_paging();
        init_heap();
        init_screen(black, green);
        init_keyboard();
        initialise_syscalls();

        IRQ_RES;
        start_terminal();
        STOP_KERNEL;
}


