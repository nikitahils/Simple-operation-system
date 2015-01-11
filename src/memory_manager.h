#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "common.h"

typedef struct segment_struct {
        u32int base;
        u32int len;
} segment_t;

typedef struct segments_info_struct {
        segment_t code_segment;
        segment_t data_segment;
        segment_t video_segment;
        segment_t module_segment;
} segments_info_t;

typedef struct memory_bitmap_struct {
        u8int  *code_bitmap;
        u8int  *data_bitmap;
        u32int  code_size;
        u32int  data_size;
} memory_bitmap_t;

void init_memory_manager(u32int code_base_addr,   u32int code_segment_len,
                         u32int data_base_addr,   u32int data_segment_len,
                         u32int module_base_addr, u32int module_segment_len);

void  print_bitmap_info();
void* alloc_data_page();
bool  free_data_page(u32int rel_address);
void* alloc_code_page();
bool  free_code_page(u32int rel_address);

#endif //MEMORY_MANAGER_H


