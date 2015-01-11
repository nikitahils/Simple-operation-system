#ifndef MODULE_H
#define MODULE_H

#include "common.h"

typedef struct module_info_struct {
        bool    running;
        bool    initialized;
        bool    loaded;
        u32int  code_offset;
        u32int  code_size;
        u32int  data_offset;
        u32int  data_size;
        u32int  entry_point;
} module_info_t;

bool  init_module();
void* alloc_module_data_page();
bool  free_module_data_page(u32int rel_address);
bool  free_module_alloced_pages();

#endif //MODULE_H
