#include "screen.h"

static screen_cursor_t screen_cursor = {.x = 0, .y = 0};
static color_t background_color      = black;
static color_t foreground_color      = white;

static void set_cursor_position(u8int x, u8int y)
{
        screen_cursor.x = x % SCREEN_WIDE;
        screen_cursor.y = y % SCREEN_HIGH;

        u16int cursorLocation = screen_cursor.y * SCREEN_WIDE + screen_cursor.x;
        u8int  low_byte       = (u8int)cursorLocation;
        u8int  high_byte      = (u8int)(cursorLocation >> 8);

        outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
        outb(0x3D5, high_byte);
        outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
        outb(0x3D5, low_byte);
}

void scroll_up()
{
        u32int i = SCREEN_WIDE * 2;
        for(i; i < SCREEN_HIGH * SCREEN_WIDE * 2; i += 2) {
            asm volatile("mov %%es:(%0), %%eax    \n\t"
                         "mov %%eax,     %%es:(%1)\n\t" : : "r"(i), "r"(i - SCREEN_WIDE * 2));
        }
        for(i = (SCREEN_HIGH - 1) * SCREEN_WIDE * 2; i < SCREEN_HIGH * SCREEN_WIDE * 2; i += 2) {
            asm volatile("movb $0x0, %%es:(%0)" :: "r"(i));
        }
        set_cursor_position(screen_cursor.x, (screen_cursor.y == 0)? 0 : screen_cursor.y - 1);
}

void new_line()
{
        if ((screen_cursor.y + 1) == SCREEN_HIGH)
            scroll_up();
        set_cursor_position(0, screen_cursor.y + 1);
}

void move_next_cursor_postion()
{
        if ((screen_cursor.x + 1) >= SCREEN_WIDE && (screen_cursor.y + 1) == SCREEN_HIGH)
            scroll_up();
        set_cursor_position((screen_cursor.x + 1), ((screen_cursor.x + 1) >= 80)? screen_cursor.y + 1 : screen_cursor.y);
}

void move_back_cursor_postion()
{
        set_cursor_position((((screen_cursor.x) == 0)? 79 : screen_cursor.x - 1), ((screen_cursor.x) == 0)? screen_cursor.y - 1 : screen_cursor.y);
}

void print_symbol(char c)
{
        vga_symbol_parameter_t output;
        output.symbol = c;
        output.foreground_color = foreground_color;
        output.background_color = background_color;

        u16int *output_ptr = (u16int*)&output;
        u32int position = (screen_cursor.y * SCREEN_WIDE + screen_cursor.x) * 2;
        asm volatile("movw    %0,%%es:(%1)" : : "r"(*output_ptr), "r"(position));
}

void clear_screen()
{
        vga_symbol_parameter_t output;
        output.symbol = 0x20;
        output.background_color = background_color;
        output.foreground_color = foreground_color;

        u32int i = 0;
        u16int *output_ptr = (u16int*)&output;
        for(i; i < SCREEN_HIGH * SCREEN_WIDE * 2; i += 2) {
            asm volatile("movw    %0,%%es:(%1)" : : "r"(*output_ptr), "r"(i));
        }

        set_cursor_position(0,0);
}

void set_colors(color_t bg_color, color_t fg_color)
{
        background_color = bg_color;
        foreground_color = fg_color;
}

void init_screen(color_t bg_color, color_t fg_color)
{
        set_colors(bg_color, fg_color);
        clear_screen();
}
