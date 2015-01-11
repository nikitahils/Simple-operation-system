#ifndef MUTEX_H
#define MUTEX_H

#include "common.h"

#define DEFINE_MUTEX(name) static mutex_t name = {.locked=0}
#define LOCKED   1
#define UNLOCKED 0

typedef struct {
	volatile u8int locked;
} mutex_t;

extern void mutex_lock(mutex_t* m);
extern void mutex_unlock(mutex_t* m);
bool mutex_trylock(mutex_t* m);

#endif //MUTEX_H
