#ifndef KBITMAP_H
#define KBITMAP_H

#include "common.h"

u32int  bitmap_in_bytes(u32int size);
void    set_bit(u8int *bitmap, u32int num);
void    clear_bit(u8int *bitmap, u32int num);
bool    test_bit(u8int *bitmap, u32int num);
u32int  first_clear_bit(u8int *bitmap, u32int size);

#endif //KBITMAP_H
