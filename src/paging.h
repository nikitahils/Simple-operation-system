#ifndef PAGING_H
#define PAGING_H

#include "common.h"
#include "memory_manager.h"

typedef struct paging_entry_struct {
        unsigned present        : 1;
        unsigned rw             : 1;
        unsigned user           : 1;
        unsigned write_through  : 1;
        unsigned cache_disabled : 1;
        unsigned accessed       : 1;
        unsigned dirty          : 1;
        unsigned size           : 1;
        unsigned global         : 1;
        unsigned unused         : 3;
        unsigned address        : 20;
} paging_entry_t;

typedef struct page_table_struct {
        paging_entry_t pages[1024];
} page_table_t;

typedef struct page_directory_struct {
        paging_entry_t page_tables[1024];
        page_table_t  *page_table_ptrs[1024];
} page_directory_t;

void init_paging();
bool mmap(segment_t segment, u32int virt_rel_address, u32int phys_rel_address, bool rw, bool user);
bool munmap(segment_t segment, u32int virt_rel_address);
bool is_paging_enabled();
void print_page_info();

#endif //PAGING_H
