#ifndef COMMON_H
#define COMMON_H

#define PAGE_SIZE 0x1000
#define NULL      0x0
#define TRUE      0x1
#define FALSE     0x0

#define IRQ_OFF        { asm volatile ("cli"); }
#define IRQ_RES        { asm volatile ("sti"); }
#define HLT_CPU        { asm volatile ("hlt"); }
#define STOP_KERNEL    { while (1) { IRQ_OFF; HLT_CPU; } }

typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;
typedef unsigned char  bool;
typedef	unsigned int   size_t;

void   outb(u16int port, u8int value);
u8int  inb(u16int port);
u16int inw(u16int port);

void  *memset(void *s, int c, size_t n);
void  *memcpy(void *dst, const void *src, size_t n);
void  *memmove(void *dst, const void *src, size_t n);
void  *memchr(const void *buf, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *str);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);
int    strcmp(const char *s1, const char *s2);
void   itoa(char *buf, int base, int d);
void   putchar(int c);
void   printf(const char *format, ...);

#endif //COMMON_H
