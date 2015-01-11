#include "keyboard.h"
#include "mutex.h"
#include "isr.h"

#define KEY_CACHE_SIZE 256
DEFINE_MUTEX(key_loc_mtx);

u8int  *keycache;
s32int  key_loc = 0;

enum keycodes {
        zero_pressed       = 0xB,
        nine_pressed       = 0xA,
        one_pressed        = 0x2,
        point_pressed      = 0x34,
        point_released     = 0xB4,
        slash_released     = 0xB5,
        slash_pressed      = 0x35,
        backspace_pressed  = 0xE,
        backspace_released = 0x8E,
        space_pressed      = 0x39,
        space_released     = 0xB9,
        enter_pressed      = 0x1C,
        enter_released     = 0x9C,
};

static char* qwertyuiop = "qwertyuiop";
static char* asdfghjkl  = "asdfghjkl";
static char* zxcvbnm    = "zxcvbnm";
static char* num        = "123456789";

static u8int keyboard_to_ascii(u8int key)
{
        if(key == enter_pressed)     return '\n';
        if(key == space_pressed)     return  ' ';
        if(key == backspace_pressed) return '\r';
        if(key == point_pressed)     return  '.';
        if(key == slash_pressed)     return  '/';
        if(key == zero_pressed)      return  '0';
        if(key >= one_pressed && key <= zero_pressed)
                return num[key - one_pressed];
        if(key >= 0x10 && key <= 0x1C) {
                return qwertyuiop[key - 0x10];
        } else if(key >= 0x1E && key <= 0x26) {
                return asdfghjkl[key - 0x1E];
        } else if(key >= 0x2C && key <= 0x32) {
                return zxcvbnm[key - 0x2C];
        }

        return 0;
}

static void keyboard_callback(registers_t regs)
{
        if (mutex_trylock(&key_loc_mtx)) {
                if (key_loc < (KEY_CACHE_SIZE - 1)) {
                        keycache[key_loc++] = keyboard_to_ascii(inb(0x60));
                }
                mutex_unlock(&key_loc_mtx);
        }
}

void init_keyboard()
{
        keycache = (u8int*)malloc(KEY_CACHE_SIZE);
        register_interrupt_handler(IRQ1, (isr_t)keyboard_callback);
}

static char ch = 0;
char get_keyboard_key()
{
        ch = 0;
        if(key_loc != 0) {
                mutex_lock(&key_loc_mtx);
                ch = *keycache;
                s32int i;
                for(i = 0; i < key_loc; i++)
                        keycache[i] = keycache[i+1];
                key_loc--;
                mutex_unlock(&key_loc_mtx);
        }

        return ch;
}
