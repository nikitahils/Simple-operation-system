#include "kbitmap.h"

u32int bitmap_in_bytes(u32int size)
{
        u32int  bytes     = (size % 8)? ((size >> 3) + 1) : (size >> 3);
        return  bytes;
}

void set_bit(u8int *bitmap, u32int num)
{
        u32int offset   = num / 8;
        u8int bit       = num % 8;
        bitmap[offset] |= (1 << bit);
}

void clear_bit(u8int *bitmap, u32int num)
{
        u32int offset   = num / 8;
        u8int bit       = num % 8;
        bitmap[offset] &= ~(1 << bit);
}

bool test_bit(u8int *bitmap, u32int num)
{
        u32int offset   = num / 8;
        u8int bit       = num % 8;
        return ((bitmap[offset] & (1 << bit)) != 0)? TRUE : FALSE;
}

u32int first_clear_bit(u8int *bitmap, u32int size)
{
        u32int  bytes  = (size % 8)? ((size >> 3) + 1) : (size >> 3);
        u32int  offset;
        u8int   bit;

        for (offset = 0; offset < bytes; offset++) {
                if (bitmap[offset] != 0xff) {
                        for (bit = 0; bit < 8; bit++) {
                                if ((bitmap[offset] & (1 << bit)) == 0)
                                        return (offset * 8) + bit;
                        }
                }
        }

        return -1;
}
