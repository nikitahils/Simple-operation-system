#include "mutex.h"

void mutex_lock(mutex_t* m)
{
    asm volatile("start_lock:         \n\t"
                 "xor  %%eax,  %%eax  \n\t"
                 "inc  %%eax          \n\t"
                 "xchg %%eax, (%%ecx) \n\t"
                 "test %%eax,  %%eax  \n\t"
                 "je ok               \n\t"
                 "jmp start_lock      \n\t"
                 "ok:                 \n\t" :: "c"(&m->locked));
}

void mutex_unlock(mutex_t* m)
{
	asm volatile("movb $0x0, (%%eax)" :: "a"(&m->locked));
}

bool mutex_trylock(mutex_t* m)
{
    bool result;
    asm volatile("xor  %%eax,  %%eax   \n\t"
                 "inc  %%eax           \n\t"
                 "xchg %%eax, (%%ecx)  \n\t"
                 "mov  %%eax,  %%ebx   \n\t" : "=b"(result) : "c"(&m->locked));
    return (result == LOCKED)? FALSE : TRUE;
}

