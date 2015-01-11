#include "paging.h"
#include "isr.h"
#include "screen.h"
#include "panic.h"
#include "module_loader.h"
#include "module.h"
#include "descriptor_tables.h"

extern segments_info_t   segments_info;
extern u32int            kernel_code_size;
page_directory_t        *kernel_page_directory;
u32int                   page_tables_rel_virt_addr;

static void switch_page_directory(page_directory_t *dir)
{
        u32int phys_dir_addr = (u32int)dir->page_tables + segments_info.data_segment.base;
        asm volatile("mov %%eax, %%cr3": :"a"(phys_dir_addr));
        asm volatile("mov %cr0, %eax");
        asm volatile("orl $0x80000000, %eax");
        asm volatile("mov %eax, %cr0");
}

bool is_paging_enabled()
{
        u32int cr0;
        asm volatile("mov %%cr0, %%eax" :"=a"(cr0):);
        return (cr0 >> 31);
}

static void set_pt_entry(paging_entry_t *entry, bool present, bool rw, bool user, u32int address)
{
        entry->present        =  present;
        entry->rw             =  rw;
        entry->user           =  user;
        entry->write_through  =  0x1;
        entry->cache_disabled =  0x1;
        entry->accessed       =  0x0;
        entry->dirty          =  0x0;
        entry->size           =  0x0;
        entry->global         =  0x0;
        entry->unused         =  0x0;
        entry->address        = (address >> 12);
}

static paging_entry_t* get_pt_entry(u32int virt_lin_address, page_directory_t *dir, bool make_page_table)
{
        u32int phys_pt_address;
        paging_entry_t *pt_entry = NULL;
        u32int page_number       = virt_lin_address / PAGE_SIZE;
        u32int page_table_index  = page_number      / 1024;
        u32int page_index        = page_number      % 1024;

        if (dir->page_table_ptrs[page_table_index] != NULL) {
                pt_entry = (is_paging_enabled())? &dir->page_table_ptrs[page_table_index]->pages[page_index]
                                                : &((page_table_t*)(dir->page_tables[page_table_index].address * PAGE_SIZE - segments_info.data_segment.base))->pages[page_index];
                return pt_entry;

        } else if (make_page_table == TRUE) {
                page_table_t *pt = (page_table_t*)alloc_data_page();
                u32int pt_phys_addr = segments_info.data_segment.base + (u32int)pt;
                //set ppt entry
                u32int ptt_index = (page_tables_rel_virt_addr + segments_info.data_segment.base) / (PAGE_SIZE * 1024);
                page_table_t *ptt = (is_paging_enabled())? dir->page_table_ptrs[ptt_index]
                                                         : (page_table_t*)(dir->page_tables[ptt_index].address * PAGE_SIZE - segments_info.data_segment.base);
                paging_entry_t *ptt_entry = &ptt->pages[page_table_index];
                set_pt_entry(ptt_entry, TRUE, TRUE, FALSE, pt_phys_addr);
                //set dir entry
                dir->page_table_ptrs[page_table_index] = (page_table_t*)(page_tables_rel_virt_addr + page_table_index * PAGE_SIZE);
                paging_entry_t *pd_entry = &dir->page_tables[page_table_index];
                set_pt_entry(pd_entry, TRUE, TRUE, TRUE, pt_phys_addr);
                //return value & clean new page table
                if (is_paging_enabled()) {
                        pt_entry = &dir->page_table_ptrs[page_table_index]->pages[page_index];
                        memset(dir->page_table_ptrs[page_table_index], 0x0, sizeof(page_table_t));
                } else {
                        pt_entry = &pt->pages[page_index];
                        memset(pt, 0x0, sizeof(page_table_t));
                }
        }

        return pt_entry;
}

bool mmap(segment_t segment, u32int virt_rel_address, u32int phys_rel_address, bool rw, bool user)
{
        u32int virt_lin_address  = virt_rel_address + segment.base;
        u32int phys_lin_address  = phys_rel_address + segment.base;
        paging_entry_t *pt_entry = get_pt_entry(virt_lin_address, kernel_page_directory, TRUE);
        if (pt_entry != NULL) {
                set_pt_entry(pt_entry, TRUE, rw, user, phys_lin_address);
                return TRUE;
        }

        return FALSE;
}

bool munmap(segment_t segment, u32int virt_rel_address)
{
        u32int virt_lin_address  = virt_rel_address + segment.base;
        paging_entry_t *pt_entry = get_pt_entry(virt_lin_address, kernel_page_directory, FALSE);
        if (pt_entry != NULL) {
                set_pt_entry(pt_entry, FALSE, FALSE, FALSE, 0x0);
                return TRUE;
        }

        return FALSE;
}

static void page_fault_handler(registers_t *regs)
{
        u32int faulting_address;
        asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
        u32int present  = !(regs->err_code & 0x1) ? 1 : 0;
        u32int rw       =   regs->err_code & 0x2  ? 1 : 0;
        u32int user     =   regs->err_code & 0x4  ? 1 : 0;
        u32int reserved =   regs->err_code & 0x8  ? 1 : 0;
        u32int id       =   regs->err_code & 0x10 ? 1 : 0;

        printf("Page fault at 0x%x - EIP: 0x%x\n", faulting_address, regs->eip);
        u32int code = (user<<2) + (rw<<1) + present;
        char *description;
        switch (code) {
            case 0x0:
                description = "Supervisory process tried to read a non-present page entry";
                break;
            case 0x1:
                description = "Supervisory process tried to read a page and caused a protection fault";
                break;
            case 0x2:
                description = "Supervisory process tried to write to a non-present page entry";
                break;
            case 0x3:
                description = "Supervisory process tried to write a page and caused a protection fault";
                break;
            case 0x4:
                description = "User process tried to read a non-present page entry";
                break;
            case 0x5:
                description = "User process tried to read a page and caused a protection fault";
                break;
            case 0x6:
                description = "User process tried to write to a non-present page entry";
                break;
            case 0x7:
                description = "User process tried to write a page and caused a protection fault";
                break;
        }
        printf("(%s)\n", description);

        if (user)
            exit_module();
        ASSERT(faulting_address < segments_info.code_segment.base);
        u32int rel_virt_faulting_address = faulting_address - segments_info.data_segment.base;
        u32int rel_phys_alloced_address  = (u32int)alloc_data_page();
        if (rel_phys_alloced_address == NULL)
            PANIC("No free memory...");
        mmap(segments_info.data_segment, rel_virt_faulting_address, rel_phys_alloced_address, TRUE, FALSE);
}

void print_page_info()
{
        int i,j;
        u32int db = segments_info.data_segment.base;
        u32int dl = segments_info.data_segment.len;
        u32int cb = segments_info.code_segment.base;
        u32int cl = segments_info.code_segment.len;
        printf("data range = [%uMB,%uMB] (%x..%x)\n", db/(1024*1024), (db+dl)/(1024*1024), db, (db+dl));
        printf("code range = [%uMB,%uMB] (%x..%x)\n", cb/(1024*1024), (cb+cl)/(1024*1024), cb, (cb+cl));
        for(i = 0; i < 1024; i++) {
                paging_entry_t pd_e = kernel_page_directory->page_tables[i];
                u32int value = *((u32int*)&pd_e);
                u32int pt_addr = (is_paging_enabled())? (u32int)kernel_page_directory->page_table_ptrs[i]
                                                      : value & 0xFFFFF000;
                if (pt_addr != NULL) {
                        //printf("kernel_page_directory[%u]=%x(table phys addr=0x%x;range=[%uMB,%uMB])\n", i, value, pt_addr, i*4, i*4 + 4);
                        page_table_t *table = (is_paging_enabled())? (page_table_t*)pt_addr
                                                                   : (page_table_t*)(pt_addr - db);
                        for(j=0; j<1024; j++) {
                                paging_entry_t pt_e = table->pages[j];
                                value = *((u32int*)&pt_e);
                                if (value != NULL) {
                                        //printf("    table[%u]=%x\n", j, value, value);
                                        u32int virt_addr = 4 * 1024 * 1024 * i + 4096 * j;
                                        u32int phys_addr = *((u32int*)&pt_e);
                                        phys_addr &= 0xFFFFF000;
                                        printf("i = %x, j = %x : %x -> %x\n", i, j, virt_addr, phys_addr);
                                }

                        }
                }
        }

}

static void init_paging_tables()
{
        //init kernel page directory + table of mapping page tables
        //dirty hack with allocation pages
        //sizeof dir is 2 pages!
        kernel_page_directory =  (page_directory_t*)alloc_data_page();
        alloc_data_page();
        memset(kernel_page_directory, 0x0, sizeof(page_directory_t));

        u32int top_data_phys_address   = ((u32int)kernel_page_directory + sizeof(page_directory_t) + segments_info.data_segment.base);
        u32int table_index             = (top_data_phys_address % (PAGE_SIZE * 1024)) ? top_data_phys_address / (PAGE_SIZE * 1024) + 1
                                         :top_data_phys_address / (PAGE_SIZE * 1024);
        u32int page_tables_base_virt_addr       = table_index * PAGE_SIZE * 1024;
        page_tables_rel_virt_addr = page_tables_base_virt_addr - segments_info.data_segment.base;
        page_table_t *page_tables_table   = alloc_data_page();
        memset(page_tables_table, 0x0, sizeof(page_table_t));
        paging_entry_t *pd_entry         = &kernel_page_directory->page_tables[table_index];
        set_pt_entry(pd_entry, TRUE, TRUE, FALSE, (u32int)page_tables_table + segments_info.data_segment.base);
        kernel_page_directory->page_table_ptrs[table_index] = (page_table_t*)(page_tables_rel_virt_addr + table_index * PAGE_SIZE);
        paging_entry_t *pt_entry         = &page_tables_table->pages[table_index];
        set_pt_entry(pt_entry, TRUE, TRUE, FALSE, (u32int)page_tables_table + segments_info.data_segment.base);//teeest!!!
}

static void map_kernel_pages()
{
        //map data
        u32int top_data_rel_address    = (u32int)kernel_page_directory + sizeof(page_directory_t);
        u32int index;
        for (index = 0; index < top_data_rel_address; index += PAGE_SIZE) {
                mmap(segments_info.data_segment, index, index, TRUE, FALSE);
        }
        //map stack
        u32int stack_rel_addr = segments_info.data_segment.len - PAGE_SIZE;
        mmap(segments_info.data_segment, stack_rel_addr, stack_rel_addr, TRUE, FALSE);
        //map code
        u32int top_code_rel_address    = (u32int)&kernel_code_size;
        for (index = 0; index < top_code_rel_address; index += PAGE_SIZE) {
                mmap(segments_info.code_segment, index, index, FALSE, FALSE);
        }
        //map video
        mmap(segments_info.video_segment, 0x0, 0x0, TRUE, FALSE);
        //map module (if exists)
        if (segments_info.module_segment.len > 0) {
            u32int top_module_rel_address = segments_info.module_segment.len;
            for (index = 0; index < top_module_rel_address; index += PAGE_SIZE) {
                mmap(segments_info.module_segment, index, index, TRUE, FALSE);
            }
        }
}

void init_paging()
{
        //alloc page directory and table of page tables
        init_paging_tables();
        //map pages
        map_kernel_pages();
        //init isr handler
        register_interrupt_handler(PAGE_FAULT, page_fault_handler);
        //enable paging
        switch_page_directory(kernel_page_directory);
        ASSERT(is_paging_enabled() == TRUE);
}



