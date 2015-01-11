#include "multiboot.h"
#include "descriptor_tables.h"

/*macros*/
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/*linker script variables*/
extern  u32int lma;
extern  u32int loader_size;
extern  u32int kernel_code_size;
extern  u32int kernel_data_size;
extern  u32int kernel_size;
extern  u32int null_ptr_offset;

/*global variables*/
static  gdt_ptr_t gdt_ptr;
static  u32int code_base_addr;
static  u32int data_base_addr;
static  u32int module_base_addr;
static  u32int code_segment_len;
static  u32int data_segment_len;
static  u32int module_segment_len;

/*forward declarations*/
static multiboot_memory_map_t* get_biggest_memory_chunk(multiboot_info_t*);
static void init_segments_parameters(multiboot_info_t*, multiboot_memory_map_t*);
static void init_gdt();
static void copy_data_and_code();
static void goto_kernel();

/*pointers on common.h & descriptor_tables.h functions*/
static void (*loader_memset)(void*, int, size_t);
static void (*loader_memcpy)(void*, void*, size_t);
static void (*loader_set_gdt_gate)(gdt_entry_t*, u32int, u32int, u8int, u8int);

void loader_main(multiboot_info_t *mbi, u32int magic)
{
            // 6 bit is MULTIBOOT_INFO_MEM_MAP (multiboot.h)
            if (CHECK_FLAG(mbi->flags, 6) && magic == MULTIBOOT_BOOTLOADER_MAGIC) {
                //1. find biggest memory chunk
                multiboot_memory_map_t *max_mmap = get_biggest_memory_chunk(mbi);
                //2. calc code & data -> base address and size (and module if exists)
                init_segments_parameters(mbi, max_mmap);
                //3. initialize global descriptor table
                init_gdt();
                //4. copy data & code kernel
                copy_data_and_code();
                //5. set stack, load gdt and jump to kernel
                goto_kernel();
            }
}

static multiboot_memory_map_t* get_biggest_memory_chunk(multiboot_info_t *mbi)
{
        u32int counter = 0, max_len = 0, upper_mem_size = 0;
        multiboot_memory_map_t *max_mmap = 0x0;
        multiboot_memory_map_t *mmap     = (multiboot_memory_map_t *)mbi->mmap_addr;
        multiboot_uint32_t      end      = mbi->mmap_addr + mbi->mmap_length;
        while ((multiboot_uint32_t)mmap < end) {
                if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        upper_mem_size += mmap->len;
                        if ((mmap->len > max_len) && (upper_mem_size != (mbi->mem_upper * 1000))) {
                                max_mmap = mmap;
                                max_len  = mmap->len;
                        }
                }
                counter++;
                mmap = (multiboot_memory_map_t *) ((multiboot_uint32_t)mmap + mmap->size + sizeof (mmap->size));
        }

        return max_mmap;
}

static void init_segments_parameters(multiboot_info_t *mbi, multiboot_memory_map_t *max_mmap)
{
        //init module segment
        if (CHECK_FLAG(mbi->flags, 3)) {
            u32int module_end_addr = *(u32int*)(mbi->mods_addr+4);
            module_base_addr = *((u32int*)mbi->mods_addr);
            module_segment_len =  module_end_addr - module_base_addr;
            module_segment_len = (module_segment_len % PAGE_SIZE) ? (module_segment_len & 0xFFFFF000) + PAGE_SIZE
                                                                  : module_segment_len;
        }
        //init code & data segments
        u32int min_load_addr, len;
        min_load_addr = (module_segment_len > 0) ? module_base_addr + module_segment_len
                                                 : (u32int)&lma + (u32int)&kernel_size;
        min_load_addr = (min_load_addr % PAGE_SIZE) ? (min_load_addr & 0xFFFFF000) + PAGE_SIZE
                                                    : min_load_addr;
        if (max_mmap->addr < min_load_addr) {
                data_base_addr = min_load_addr;
                len = max_mmap->len - (min_load_addr - max_mmap->addr);
        } else {
                data_base_addr = max_mmap->addr;
                len = max_mmap->len;
        }
        data_segment_len = len >> 1;
        if (data_segment_len % PAGE_SIZE > 0)
                data_segment_len = (data_segment_len & 0xFFFFF000) + PAGE_SIZE;
        code_base_addr = data_base_addr + data_segment_len;
        code_segment_len = len - data_segment_len;
}

static void init_gdt()
{
        gdt_entry_t *gdt_entries = (gdt_entry_t*)(data_base_addr + (u32int)&kernel_data_size);
        gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
        gdt_ptr.base  = (u32int)gdt_entries;

        loader_memset = (void (*)(void*, int, size_t))((u32int)&lma + (u32int)&loader_size + memset - (u32int)&null_ptr_offset);
        loader_memset(gdt_entries, 0x0, sizeof(gdt_entry_t) * 5);

        loader_set_gdt_gate = (void (*)(gdt_entry_t*, u32int, u32int, u8int, u8int))((u32int)&lma + (u32int)&loader_size + set_global_descriptor - (u32int)&null_ptr_offset);
        loader_set_gdt_gate(gdt_entries, 0, 0, 0, 0);                                               // Null   segment  - 0x0
        loader_set_gdt_gate(gdt_entries + 1, data_base_addr,   data_segment_len >> 12, 0x92, 0xC0); // Data   segment  - 0x8
        loader_set_gdt_gate(gdt_entries + 2, code_base_addr,   code_segment_len >> 12, 0x9A, 0xC0); // Code   segment  - 0x10
}

static void copy_data_and_code()
{
        loader_memcpy = (void (*)(void*, void*, size_t))((u32int)&lma + (u32int)&loader_size + memcpy - (u32int)&null_ptr_offset);
        //data kernel
        u8int *kernel_data_ptr = (u8int*)((u32int)&lma + (u32int)&loader_size + (u32int)&kernel_code_size);
        loader_memcpy((void*)(data_base_addr + (u32int)&null_ptr_offset), kernel_data_ptr, (u32int)&kernel_data_size - (u32int)&null_ptr_offset);
        //code kernel
        u8int *kernel_code_ptr = (u8int*)((u32int)&lma + (u32int)&loader_size);
        loader_memcpy((void*)(code_base_addr + (u32int)&null_ptr_offset), kernel_code_ptr, (u32int)&kernel_code_size - (u32int)&null_ptr_offset);
}

static void goto_kernel()
{
        u32int gdtr_addr = (u32int)&gdt_ptr;
        u32int kernel_stack_phys_addr = data_segment_len + data_base_addr;

        //pass parameters to start_kernel(...) function.
        loader_memcpy((void*)(kernel_stack_phys_addr - sizeof(u32int) * 1), (void*)(&module_segment_len), sizeof(u32int));
        loader_memcpy((void*)(kernel_stack_phys_addr - sizeof(u32int) * 2), (void*)(&module_base_addr),   sizeof(u32int));
        loader_memcpy((void*)(kernel_stack_phys_addr - sizeof(u32int) * 3), (void*)(&data_segment_len),   sizeof(u32int));
        loader_memcpy((void*)(kernel_stack_phys_addr - sizeof(u32int) * 4), (void*)(&data_base_addr),     sizeof(u32int));
        loader_memcpy((void*)(kernel_stack_phys_addr - sizeof(u32int) * 5), (void*)(&code_segment_len),   sizeof(u32int));
        loader_memcpy((void*)(kernel_stack_phys_addr - sizeof(u32int) * 6), (void*)(&code_base_addr),     sizeof(u32int));
        u32int stack_base_addr = data_segment_len    - sizeof(u32int) * 7;

        asm volatile(
                "lgdtl  (%0)       \n\t"
                "mov    $0x8,%%ax  \n\t"
                "mov    %%ax,%%ds  \n\t"
                "mov    %%ax,%%ss  \n\t"
                "mov    %1,%%esp   \n\t"
                "ljmp   $0x10,%2   \n\t" : : "r"(gdtr_addr), "r"(stack_base_addr), "i"((u32int)&null_ptr_offset));
}



