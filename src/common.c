#include "common.h"
#include "screen.h"

void outb(u16int port, u8int value)
{
asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int inb(u16int port)
{
        u8int ret;
        asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

u16int inw(u16int port)
{
        u16int ret;
        asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
        return ret;
}

void *memset(void *s, int c, size_t n)
{
        u8int *p = (u8int *)s;
        u8int *end = p + n;

        while (p != end) {
                *p++ = c;
        }

        return s;
}

void *memcpy(void *dst, const void *src, size_t n)
{
        u8int *p = (u8int*)src;
        u8int *q = (u8int*)dst;
        u8int *end = p + n;

        while (p != end) {
                *q++ = *p++;
        }

        return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
        u8int *p = (u8int*)src;
        u8int *q = (u8int*)dst;
        u8int *end = p + n;

        if (q > p && q < end) {
                p = end;
                q += n;

                while (p != src) {
                        *--q = *--p;
                }
        } else {
                while (p != end) {
                        *q++ = *p++;
                }
        }

        return dst;
}

void *memchr(const void *buf, int c, size_t n)
{
        u8int *p = (u8int*)buf;
        u8int *end = p + n;

        while (p != end) {
                if (*p == c) {
                        return p;
                }

                ++p;
        }

        return 0;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
        const u8int *byte1 = (const u8int*)s1;
        const u8int *byte2 = (const u8int*)s2;
        while ((*byte1 == *byte2) && (n > 0)) {
                ++byte1;
                ++byte2;
                --n;
        }

        if (n == 0) {
                return 0;
        }
        return *byte1 - *byte2;
}

size_t strlen(const char *str)
{
        const char *s = str;
        while (*s++)
                ;

        return s - str - 1;
}

char *strcpy(char *dst, const char *src)
{
        char c;
        char *p = dst;

        while ((c = *src++)) {
                *p++ = c;
        }

        *p = '\0';
        return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
        size_t i;

        for (i = 0 ; i < n && src[i] != '\0' ; i++) {
                dst[i] = src[i];
        }

        for ( ; i < n ; i++) {
                dst[i] = '\0';
        }

        return dst;
}

int strcmp(const char *s1, const char *s2)
{
        while (*s1 == *s2) {
                if (*s1 == '\0') {
                        return 0;
                }

                ++s1;
                ++s2;
        }

        return *s1 - *s2;
}

void itoa(char *buf, int base, int d)
{
        char *p = buf;
        char *p1, *p2;
        unsigned long ud = d;
        int divisor = 10;

        /* If %d is specified and D is minus, put `-' in the head. */
        if (base == 'd' && d < 0) {
                *p++ = '-';
                buf++;
                ud = -d;
        } else if (base == 'x')
                divisor = 16;

        /* Divide UD by DIVISOR until UD == 0. */
        do {
                int remainder = ud % divisor;

                *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
        } while (ud /= divisor);

        /* Terminate BUF. */
        *p = 0;

        /* Reverse BUF. */
        p1 = buf;
        p2 = p - 1;
        while (p1 < p2) {
                char tmp = *p1;
                *p1 = *p2;
                *p2 = tmp;
                p1++;
                p2--;
        }
}

void putchar(int c)
{
        if(c == '\n') {
                new_line();
                return;
        } else if (c == '\r') {
                move_back_cursor_postion();
                return;
        }
        print_symbol(c);
        move_next_cursor_postion();
}

void printf(const char *format, ...)
{
        char **arg = (char **) &format;
        int c;
        char buf[20];

        arg++;

        while ((c = *format++) != 0) {
                if (c != '%')
                        putchar (c);
                else {
                        char *p;

                        c = *format++;
                        switch (c) {
                        case 'd':
                        case 'u':
                        case 'x':
                                itoa (buf, c, *((int *) arg++));
                                p = buf;
                                goto string;
                                break;

                        case 's':
                                p = *arg++;
                                if (!p)
                                        p = "(null)";

string:
                                while (*p)
                                        putchar (*p++);
                                break;
                        default:
                                putchar (*((int *) arg++));
                                break;
                        }
                }
        }
}
