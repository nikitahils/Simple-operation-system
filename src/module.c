#include "module.h"
#include "module_loader.h"
#include "memory_manager.h"

#define GET_MAGIC(var)            asm("mov %%fs:(0x0),   %%eax":"=a"(var):)
#define GET_CODE_OFFSET(var)      asm("mov %%fs:(0x4),   %%eax":"=a"(var):)
#define GET_CODE_SIZE(var)        asm("mov %%fs:(0x8),   %%eax":"=a"(var):)
#define GET_DATA_OFFSET(var)      asm("mov %%fs:(0xc),   %%eax":"=a"(var):)
#define GET_DATA_SIZE(var)        asm("mov %%fs:(0x10),  %%eax":"=a"(var):)
#define GET_ENTRY_POINT_ADDR(var) asm("mov %%fs:(0x14),  %%eax":"=a"(var):)

#define MAGIC                 0xDEADBEEF

extern segments_info_t   segments_info;
module_info_t            module_info;

bool init_module()
{
        if(module_info.initialized)
            return TRUE;

        u32int magic, code_offset, code_size, data_offset, data_size, entry_point;
        GET_MAGIC(magic);
        if (magic == MAGIC) {
                GET_CODE_OFFSET(code_offset);
                GET_CODE_SIZE(code_size);
                GET_DATA_OFFSET(data_offset);
                GET_DATA_SIZE(data_size);
                GET_ENTRY_POINT_ADDR(entry_point);

                u32int code_max_offset      = code_offset + code_size;
                u32int data_max_offset      = data_offset + data_size;
                u32int entry_point_offset   = entry_point - MODULE_CODE_LOAD_ADDR;
                u32int free_data_space_size = segments_info.data_segment.len - MODULE_DATA_LOAD_ADDR - PAGE_SIZE;
                u32int free_code_space_size = segments_info.code_segment.len - MODULE_CODE_LOAD_ADDR;

                if (code_size          >  0x0                                          &&
                    code_max_offset    <  segments_info.module_segment.len             &&
                    data_max_offset    <  segments_info.module_segment.len             &&
                    entry_point        >  MODULE_CODE_LOAD_ADDR                        &&
                    entry_point_offset <  code_max_offset                              &&
                    entry_point_offset >= code_offset                                  &&
                    segments_info.data_segment.len > MODULE_DATA_LOAD_ADDR + PAGE_SIZE &&
                    segments_info.code_segment.len > MODULE_CODE_LOAD_ADDR             &&
                    code_size          < free_code_space_size                          &&
                    data_size          < free_data_space_size) {

                        module_info.code_offset = code_offset;
                        module_info.code_size   = code_size;
                        module_info.data_offset = data_offset;
                        module_info.data_size   = data_size;
                        module_info.entry_point = entry_point;
                        module_info.running     = FALSE;
                        module_info.initialized = TRUE;


                        return TRUE;
                    }
        }

        return FALSE;
}

/*TO-DO: write implementations*/
void* alloc_module_data_page()
{
    //stub
    return NULL;
}

bool free_module_data_page(u32int rel_address)
{
    //stub
    return FALSE;
}

bool free_module_alloced_pages()
{
    //stub
    return FALSE;
}
