#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include "common.h"

#define MODULE_CODE_LOAD_ADDR 0x00400000
#define MODULE_DATA_LOAD_ADDR 0x00C00000

void   start_module();
void   exit_module();

#endif //MODULE_LOADER_H
