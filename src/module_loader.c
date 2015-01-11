#include "module_loader.h"
#include "memory_manager.h"
#include "paging.h"
#include "module.h"
#include "panic.h"
#include "isr.h"

extern segments_info_t   segments_info;
extern module_info_t     module_info;
registers_t              kernel_state;

static void load_module_code_and_data(u32int code_offset, u32int code_size, u32int data_offset, u32int data_size)
{
        //alloc & mmap code pages
        asm ("mov %%ax, %%gs" :: "a"(0x38));
        u32int source_addr, dest_addr;
        u32int code_page_count = (code_size % PAGE_SIZE) ? (code_size / PAGE_SIZE) + 1
                                                         :  code_size / PAGE_SIZE;
        u32int index, virt_rel_addr, phys_rel_addr;
        for (index = 0; index < code_page_count; index++) {
                phys_rel_addr      = (u32int)alloc_code_page();
                virt_rel_addr = MODULE_CODE_LOAD_ADDR + index * PAGE_SIZE;
                ASSERT(phys_rel_addr != NULL);
                mmap(segments_info.code_segment, virt_rel_addr, phys_rel_addr, FALSE, TRUE);
        }
        //clean code pages
        for (index = MODULE_CODE_LOAD_ADDR; index < (MODULE_CODE_LOAD_ADDR + code_page_count * PAGE_SIZE) - 3; index++) {
                asm("movb $0x0,  %%al     \n\t"
                    "movb %%al, %%gs:(%0) \n\t" :: "c"(index));
        }
        //copy code
        for (index = 0; (code_offset + index) < (code_offset + code_size); index++) {
                source_addr = code_offset + index;
                dest_addr   = MODULE_CODE_LOAD_ADDR + index;
                asm("movb %%fs:(%0), %%al     \n\t"
                    "movb %%al,      %%gs:(%1) \n\t" :: "b"(source_addr), "d"(dest_addr));
        }
        asm ("mov %%ax, %%gs" :: "a"(0x10));

        if (data_size > 0) {
                //alloc & mmap data pages
                u32int data_page_count = (data_size % PAGE_SIZE) ? (data_size / PAGE_SIZE) + 1
                                                                 :  data_size / PAGE_SIZE;

                for (index = 0; index < data_page_count; index++) {
                        phys_rel_addr = (u32int)alloc_data_page();
                        virt_rel_addr = MODULE_DATA_LOAD_ADDR + index * PAGE_SIZE;
                        ASSERT(phys_rel_addr != NULL);
                        mmap(segments_info.data_segment, virt_rel_addr, phys_rel_addr, TRUE, TRUE);
                        memset((void*)virt_rel_addr, 0x0, PAGE_SIZE);
                }
                //copy data
                for (index = 0; (data_offset + index) < (data_offset + data_size); index++) {
                        source_addr = data_offset + index;
                        dest_addr   = MODULE_DATA_LOAD_ADDR + index;
                        asm("movb %%fs:(%0), %%al     \n\t"
                            "movb %%al,     %%ds:(%1) \n\t" :: "b"(source_addr), "d"(dest_addr));
                }
        }
        //alloc & mmap stack
        phys_rel_addr = (u32int)alloc_data_page();
        virt_rel_addr = segments_info.data_segment.len - PAGE_SIZE * 2;
        ASSERT(phys_rel_addr != NULL);
        mmap(segments_info.data_segment, virt_rel_addr, phys_rel_addr, TRUE, TRUE);
}

static void jump_to_module_code(u32int entry_point)
{
        IRQ_OFF;
        module_info.running = TRUE;
        u32int module_esp = segments_info.data_segment.len - PAGE_SIZE - 1;
        asm volatile(
                "mov   $0x23,  %%ax  \n\t"
                "mov   %%ax,   %%fs  \n\t"
                "mov   %%ax,   %%gs  \n\t"
                "mov   %%ax,   %%ds  \n\t"
                "mov   %%ax,   %%es  \n\t"
                "push  $0x23         \n\t"
                "push  %%ecx         \n\t"
                "pushf               \n\t"
                "pop   %%eax         \n\t"
                "orl   $0x200, %%eax \n\t"
                "push  %%eax         \n\t"
                "push  $0x1b         \n\t"
                "push  %0            \n\t"
                "iret                \n\t" :: "b"(entry_point), "c"(module_esp));
}

void save_kernel_state(u32int eip, u32int esp)
{
    kernel_state.eip = eip;
    kernel_state.esp = esp;
    asm ("mov %%cs,    %%eax":"=a"(kernel_state.cs)    :);
    asm ("mov %%ss,    %%eax":"=a"(kernel_state.ss)    :);
    asm ("mov %%ds,    %%eax":"=a"(kernel_state.ds)    :);
    asm ("mov %%es,    %%eax":"=a"(kernel_state.es)    :);
    asm ("mov %%fs,    %%eax":"=a"(kernel_state.fs)    :);
    asm ("mov (%%ebp), %%eax":"=a"(kernel_state.ebp)   :);
    asm ("mov %%ebx,   %%eax":"=a"(kernel_state.ebx)   :);
    asm ("mov %%esi,   %%eax":"=a"(kernel_state.esi)   :);
    asm ("mov %%edi,   %%eax":"=a"(kernel_state.edi)   :);
    asm ("pushf\n\t"
         "pop %%eax"         :"=a"(kernel_state.eflags):);
    asm ("pop  %%ebp \n\t"
         "push %%eax \n\t" ::"a"(kernel_state.ebp));
}

void restore_kernel_state()
{
        IRQ_OFF;
        module_info.running = FALSE;
        asm ("mov %%eax, %%ds"::"a"(kernel_state.ds));
        asm ("mov %%eax, %%gs"::"a"(kernel_state.ds));
        asm ("mov %%eax, %%fs"::"a"(kernel_state.fs));
        asm ("mov %%eax, %%es"::"a"(kernel_state.es));
        asm ("mov %%eax, %%ss"::"a"(kernel_state.ss));

        asm volatile ("mov %%ebx, %%esp\n\t"
                      "push %%ecx \n\t"
                      "push %%edx \n\t"
                      "push %%esi \n\t" ::  "b"(kernel_state.esp), "c"(kernel_state.eflags),
                                            "d"(kernel_state.cs),  "S"(kernel_state.eip));

        asm volatile ("mov %%eax, %%ebp \n\t"
                      "iret             \n\t" :: "b"(kernel_state.ebx), "S"(kernel_state.esi),
                                                 "D"(kernel_state.edi), "a"(kernel_state.ebp));
}

void start_module()
{
        if (segments_info.module_segment.len > 0) {
                if (init_module()) {
                        u32int exit_eip, kernel_esp;
                        if (!module_info.loaded) {
                                load_module_code_and_data(module_info.code_offset, module_info.code_size, module_info.data_offset, module_info.data_size);
                                module_info.loaded = TRUE;
                        }

                        asm("mov $exit_label, %%eax" :"=a"(exit_eip):);
                        asm("mov %%esp, %%eax"       :"=a"(kernel_esp):);
                        set_kernel_stack_in_tss(kernel_esp);
                        save_kernel_state(exit_eip, kernel_esp);
                        jump_to_module_code(module_info.entry_point);

                } else
                        printf("bad module or it doesn't exist...");
        }

        asm("exit_label:");
}

void exit_module() {
    free_module_alloced_pages();
    restore_kernel_state();
}





