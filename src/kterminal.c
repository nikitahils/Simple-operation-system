#include "kterminal.h"
#include "screen.h"

#define CMD_BUF_SIZE (SCREEN_HIGH * SCREEN_WIDE)

char   *cmd_buf;
u32int  cur_cmd_buf_pos;

void start_terminal()
{
        char c;
        init_buf();
        printf(">> ");
        while(1) {
                c = get_keyboard_key();
                if(c != 0x0) {
                        putchar(c);
                        add_char_to_buf(c);
                        if (c == '\n') {
                            run_cmd();
                            clear_buf();
                            printf("\n>> ");
                        }

                }
        }
}

void add_char_to_buf(char c)
{
    if (cur_cmd_buf_pos < CMD_BUF_SIZE && c != '\n' && c != '\r') {
        cmd_buf[cur_cmd_buf_pos] = c;
        cur_cmd_buf_pos++;
    }
}

void clear_buf(char c)
{
    memset(cmd_buf, 0x0, cur_cmd_buf_pos);
    cur_cmd_buf_pos = 0;
}

void init_buf() {
        cmd_buf = malloc(CMD_BUF_SIZE);
        memset(cmd_buf, 0x0, CMD_BUF_SIZE);
        cur_cmd_buf_pos = 0;
}

void run_cmd()
{
    if (!strcmp("clear", cmd_buf)) {
        clear_screen();
    } else if(!strcmp("help", cmd_buf)) {
        printf("commands:\n  1. help\n  2. clear");
    } else {
        printf("unknown command \"%s\"", cmd_buf);
    }
}
