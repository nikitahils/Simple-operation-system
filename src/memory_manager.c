#include "memory_manager.h"
#include "panic.h"
#include "screen.h"

extern  u32int   kernel_data_size;
extern  u32int   kernel_code_size;

bool             memory_bitmap_initialized = FALSE;
segments_info_t  segments_info;
memory_bitmap_t  memory_bitmap;
u32int           data_top_address = (u32int)&kernel_data_size;

void init_segments_info(u32int code_base_addr,    u32int code_segment_len,
                         u32int data_base_addr,   u32int data_segment_len,
                         u32int module_base_addr, u32int module_segment_len)
{
        segments_info.code_segment.base  = code_base_addr;
        segments_info.code_segment.len   = code_segment_len;
        segments_info.data_segment.base  = data_base_addr;
        segments_info.data_segment.len   = data_segment_len;
        segments_info.video_segment.base = VIDEO_BASE_ADDR;
        segments_info.video_segment.len  = SCREEN_WIDE * SCREEN_HIGH * 2;
        segments_info.module_segment.base = module_base_addr;
        segments_info.module_segment.len  = module_segment_len;
}

void* alloc_page(u8int* bitmap)
{
        void *mem_ptr = NULL;
        if (memory_bitmap_initialized == TRUE) {
                u32int bitmap_size  = (bitmap == memory_bitmap.code_bitmap) ? memory_bitmap.code_size
                                                                            : memory_bitmap.data_size;
                u32int bitmap_index = first_clear_bit(bitmap, bitmap_size);
                if (bitmap_index != -1) {
                        u32int rel_free_page_address = bitmap_index * PAGE_SIZE;
                        ASSERT((bitmap_index < bitmap_size) && (bitmap_index >= 0));
                        set_bit(bitmap, bitmap_index);
                        mem_ptr = (void*)rel_free_page_address;
                }
        }

        return mem_ptr;
}

bool free_page(u32int rel_address, u8int *bitmap)
{
        ASSERT((rel_address % PAGE_SIZE) == 0);
        if (memory_bitmap_initialized == TRUE) {
                u32int bitmap_index = rel_address / PAGE_SIZE;
                u32int bitmap_size  = (bitmap == memory_bitmap.code_bitmap) ? memory_bitmap.code_size
                                                                            : memory_bitmap.data_size;
                if (bitmap_index < bitmap_size) {
                    clear_bit(bitmap, bitmap_index);
                    return TRUE;
                }
        }

        return FALSE;
}

void* alloc_code_page()
{
    return alloc_page(memory_bitmap.code_bitmap);
}

void* alloc_data_page()
{
    return alloc_page(memory_bitmap.data_bitmap);
}

bool free_code_page(u32int rel_address)
{
    return free_page(rel_address, memory_bitmap.code_bitmap);
}

bool free_data_page(u32int rel_address)
{
    return free_page(rel_address, memory_bitmap.data_bitmap);
}

void init_memory_bitmap()
{
        u32int bytes, pages;
        //create code bitmap
        u32int code_pages_count   =  segments_info.code_segment.len / PAGE_SIZE;
        bytes                     =  bitmap_in_bytes(code_pages_count);
        pages                     = (bytes % PAGE_SIZE) ? (bytes / PAGE_SIZE) + 1 : (bytes / PAGE_SIZE);
        memory_bitmap.code_bitmap = (u8int*)data_top_address;
        memory_bitmap.code_size   =  code_pages_count;
        memset(memory_bitmap.code_bitmap, 0x0, PAGE_SIZE);
        data_top_address         +=  PAGE_SIZE * pages;
        //create data bitmap
        u32int data_pages_count   =  segments_info.data_segment.len / PAGE_SIZE;
        bytes                     =  bitmap_in_bytes(data_pages_count);
        pages                     = (bytes % PAGE_SIZE) ? (bytes / PAGE_SIZE) + 1 : (bytes / PAGE_SIZE);
        memory_bitmap.data_bitmap = (u8int*)data_top_address;
        memory_bitmap.data_size   =  data_pages_count;
        data_top_address         +=  PAGE_SIZE * pages;
        memset(memory_bitmap.data_bitmap, 0x0, PAGE_SIZE);

        ASSERT((data_top_address % PAGE_SIZE) == 0);

        //set used data pages
        u32int used_data_pages = data_top_address / PAGE_SIZE;
        u32int index;
        for (index = 0; index < used_data_pages; index++) {
                set_bit(memory_bitmap.data_bitmap, index);
        }
        //stack top page
        set_bit(memory_bitmap.data_bitmap, memory_bitmap.data_size - 1);

        //set used code pages
        u32int used_code_pages = (u32int)&kernel_code_size / PAGE_SIZE;
        for (index = 0; index < used_code_pages; index++) {
                set_bit(memory_bitmap.code_bitmap, index);
        }

        memory_bitmap_initialized = TRUE;
}

void init_memory_manager(u32int code_base_addr,   u32int code_segment_len,
                         u32int data_base_addr,   u32int data_segment_len,
                         u32int module_base_addr, u32int module_segment_len)
{
        //segments info
        init_segments_info(code_base_addr, code_segment_len, data_base_addr, data_segment_len, module_base_addr, module_segment_len);
        //memory info
        init_memory_bitmap();
}

void print_bitmap_info()
{
        int i;
        printf("========= used pages:\n");
        for (i = 0; i < memory_bitmap.data_size; i++) {
                if (test_bit(memory_bitmap.data_bitmap, i)) {
                        printf("0x%x\n", i * PAGE_SIZE + segments_info.data_segment.base);
                }
        }
        for (i = 0; i < memory_bitmap.code_size; i++) {
                if (test_bit(memory_bitmap.code_bitmap, i)) {
                        printf("0x%x\n", i * PAGE_SIZE + segments_info.code_segment.base );
                }
        }
        printf("=========\n");
}
