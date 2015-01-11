#ifndef SCREEN_H
#define SCREEN_H

#include "common.h"

#define  SCREEN_WIDE        80
#define  SCREEN_HIGH        25
#define  VIDEO_BASE_ADDR  0xB8000

typedef struct screen_cursor_struct {
        u8int x;
        u8int y;
} screen_cursor_t;

struct vga_symbol_parameter_struct {
        u8int symbol;
        unsigned foreground_color : 4;
        unsigned background_color : 4;
} __attribute__((packed));
typedef struct vga_symbol_parameter_struct vga_symbol_parameter_t;

typedef enum color_enum {
        black         = 0x0,
        blue          = 0x1,
        green         = 0x2,
        cyan          = 0x3,
        red           = 0x4,
        magenta       = 0x5,
        brown         = 0x6,
        light_grey    = 0x7,
        dark_grey     = 0x8,
        light_blue    = 0x9,
        light_green   = 0xA,
        light_cyan    = 0xB,
        light_red     = 0xC,
        light_magneta = 0xD,
        light_brown   = 0xE,
        white         = 0xF
} color_t;

void move_next_cursor_postion();
void move_back_cursor_postion();
void new_line();
void scroll_up();
void clear_screen();
void print_symbol(char c);
void set_colors(color_t bg_color, color_t fg_color);
void init_screen(color_t bg_color, color_t fg_color);

#endif //SCREEN_H
