#include "kheap.h"
#include "panic.h"
#include "paging.h"
#include "memory_manager.h"

extern segments_info_t   segments_info;
extern u32int            page_tables_rel_virt_addr;
u32int                   heap_start_rel_virt_addr;
u32int                   heap_brk;
u32int                   heap_size;


void init_heap()
{
        heap_start_rel_virt_addr    = page_tables_rel_virt_addr + PAGE_SIZE * 1024;
        u32int free_virt_space_size = (segments_info.data_segment.len - heap_start_rel_virt_addr);
        heap_size            = (free_virt_space_size / 2) & 0xFFFFF000;
        heap_size                   =  (heap_size > PAGE_SIZE * 1024) ? PAGE_SIZE * 1024 : heap_size;
        heap_brk                    = heap_start_rel_virt_addr;
}

void* sbrk(u32int increment)
{
        ASSERT(increment % PAGE_SIZE == 0);
        ASSERT(heap_brk  % PAGE_SIZE == 0);
        ASSERT(heap_brk + increment < heap_start_rel_virt_addr + heap_size);
        u32int address = heap_brk;
        u32int i, *ptr;
        for (i = heap_brk; i < heap_brk + increment; i += PAGE_SIZE) {
                ptr = alloc_data_page();
                ASSERT(ptr != NULL);
                mmap(segments_info.data_segment, i, (u32int)ptr, TRUE, FALSE);
        }

        heap_brk += increment;
        memset((void *)address, 0x0, increment);

        return (void *)address;
}

