#include "descriptor_tables.h"
#include "memory_manager.h"
#include "screen.h"

extern segments_info_t  segments_info;

/*macros*/
#define GLOBAL_DESCRIPTOR_COUNT    20
#define INTERRUPT_DESCRIPTOR_COUNT 256

/*global variables*/
tss_entry_t      tss_entry;
u32int           idt_base_addr;
u32int           gdt_base_addr;
gdt_ptr_t        gdt_ptr;
idt_ptr_t        idt_ptr;

void set_global_descriptor(gdt_entry_t *gdt_entry, u32int base, u32int limit, u8int access, u8int gran)
{
        gdt_entry->base_low     = (base & 0xFFFF);
        gdt_entry->base_middle  = (base >> 16) & 0xFF;
        gdt_entry->base_high    = (base >> 24) & 0xFF;
        gdt_entry->limit_low    = (limit & 0xFFFF);
        gdt_entry->granularity  = (limit >> 16) & 0x0F;
        gdt_entry->granularity |=  gran & 0xF0;
        gdt_entry->access       =  access;
}

static void set_interrupt_gate(idt_entry_t *idt_entry, u32int base, u16int selector, u8int flags)
{
        idt_entry->base_low    =  base & 0xFFFF;
        idt_entry->base_high   = (base >> 16) & 0xFFFF;
        idt_entry->selector    =  selector;
        idt_entry->always_zero =  0x0;
        idt_entry->flags       =  flags;
}

static void set_task_descriptor(gdt_entry_t *gdt_entry, u16int ss0, u32int esp0)
{
    u32int base  = (u32int)&tss_entry + segments_info.data_segment.base;
    u32int limit = sizeof(tss_entry_t);
    set_global_descriptor(gdt_entry, base, limit, 0xE9, 0x00);
    memset(&tss_entry, 0x0, sizeof(tss_entry_t));

    tss_entry.ss0  = ss0;  // Set the kernel stack segment.
    tss_entry.esp0 = esp0; // Set the kernel stack pointer.
}

void set_kernel_stack_in_tss(u32int stack)
{
    tss_entry.esp0 = stack;
}

static void init_gdt()
{
        gdt_base_addr = (u32int)alloc_data_page();
        gdt_entry_t *gdt_entries = (gdt_entry_t*)(gdt_base_addr);
        gdt_ptr.limit = (sizeof(gdt_entry_t) * GLOBAL_DESCRIPTOR_COUNT) - 1;
        gdt_ptr.base  = (u32int)gdt_entries + segments_info.data_segment.base;

        memset((void*)gdt_entries, 0x0, sizeof(gdt_entry_t) * GLOBAL_DESCRIPTOR_COUNT);

        // Null segment            - 0x0
        set_global_descriptor(gdt_entries,  0, 0, 0, 0);
        // Kernel code    segment  - 0x8
        set_global_descriptor(gdt_entries + 1, segments_info.code_segment.base,   segments_info.code_segment.len >> 12,    0x9A, 0xC0);
        // Kernel data    segment  - 0x10
        set_global_descriptor(gdt_entries + 2, segments_info.data_segment.base,   segments_info.data_segment.len >> 12,    0x92, 0xC0);
        // User   code    segment  - 0x18
        set_global_descriptor(gdt_entries + 3, segments_info.code_segment.base,   segments_info.code_segment.len >> 12,    0xFA, 0xC0);
        // User   data    segment  - 0x20
        set_global_descriptor(gdt_entries + 4, segments_info.data_segment.base,   segments_info.data_segment.len >> 12,    0xF2, 0xC0);

        // Kernel video   segment  - 0x28
        set_global_descriptor(gdt_entries + 5, segments_info.video_segment.base,  segments_info.video_segment.len,         0x92, 0x40);
        // Kernel module  segment  - 0x30
        set_global_descriptor(gdt_entries + 6, segments_info.module_segment.base, segments_info.module_segment.len >> 12,  0x92, 0xC0);
        // Kernel code-data segment  - 0x38
        set_global_descriptor(gdt_entries + 7, segments_info.code_segment.base,   segments_info.code_segment.len   >> 12,  0x92, 0xC0);
        // Kernel task state segment - 0x40
        set_task_descriptor(gdt_entries   + 8, 0x10, 0x0);

        u32int gdtr_addr = (u32int)&gdt_ptr;
        asm volatile(
                "lgdtl  (%0)         \n\t"
                "mov    $0x10, %%ax  \n\t"
                "mov    %%ax,  %%ds  \n\t"
                "mov    %%ax,  %%gs  \n\t"
                "mov    %%ax,  %%ss  \n\t"
                "mov    $0x30, %%ax  \n\t"
                "mov    %%ax,  %%fs  \n\t"
                "mov    $0x28, %%ax  \n\t"
                "mov    %%ax,  %%es  \n\t"
                "ljmp   $0x8,  $label\n\t"
                "label:              \n\t"
                "mov    $0x40, %%ax  \n\t"
                "ltr    %%ax         \n\t": : "a"(gdtr_addr));
}

static void init_idt()
{
        idt_base_addr = (u32int)alloc_data_page();
        idt_entry_t *idt_entries = (idt_entry_t*)(idt_base_addr);
        idt_ptr.limit = (sizeof(idt_entry_t) * INTERRUPT_DESCRIPTOR_COUNT) - 1;
        idt_ptr.base  = (u32int)idt_entries  + segments_info.data_segment.base;

        memset((void*)idt_entries, 0x0, sizeof(idt_entry_t) * INTERRUPT_DESCRIPTOR_COUNT);

        // Remap the irq table.
        outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 0x28);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0x21, 0x0);
        outb(0xA1, 0x0);

        //isr
        set_interrupt_gate(idt_entries,      (u32int)isr0,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 1,  (u32int)isr1,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 2,  (u32int)isr2,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 3,  (u32int)isr3,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 4,  (u32int)isr4,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 5,  (u32int)isr5,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 6,  (u32int)isr6,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 7,  (u32int)isr7,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 8,  (u32int)isr8,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 9,  (u32int)isr9,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 10, (u32int)isr10, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 11, (u32int)isr11, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 12, (u32int)isr12, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 13, (u32int)isr13, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 14, (u32int)isr14, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 15, (u32int)isr15, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 16, (u32int)isr16, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 17, (u32int)isr17, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 18, (u32int)isr18, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 19, (u32int)isr19, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 20, (u32int)isr20, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 21, (u32int)isr21, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 22, (u32int)isr22, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 23, (u32int)isr23, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 24, (u32int)isr24, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 25, (u32int)isr25, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 26, (u32int)isr26, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 27, (u32int)isr27, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 28, (u32int)isr28, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 29, (u32int)isr29, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 30, (u32int)isr30, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 31, (u32int)isr31, 0x08, 0x8E);
        //irq
        set_interrupt_gate(idt_entries + 32, (u32int)irq0,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 33, (u32int)irq1,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 34, (u32int)irq2,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 35, (u32int)irq3,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 36, (u32int)irq4,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 37, (u32int)irq5,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 38, (u32int)irq6,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 39, (u32int)irq7,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 40, (u32int)irq8,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 41, (u32int)irq9,  0x08, 0x8E);
        set_interrupt_gate(idt_entries + 42, (u32int)irq10, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 43, (u32int)irq11, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 44, (u32int)irq12, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 45, (u32int)irq13, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 46, (u32int)irq14, 0x08, 0x8E);
        set_interrupt_gate(idt_entries + 47, (u32int)irq15, 0x08, 0x8E);
        //syscall
        set_interrupt_gate(idt_entries + 128, (u32int)isr128, 0x08, 0xEF);

        u32int idtr_addr = (u32int)&idt_ptr;
        asm volatile("lidtl  (%0)" : : "a"(idtr_addr));
}

void init_descriptor_tables()
{
        init_gdt();
        init_idt();
}

