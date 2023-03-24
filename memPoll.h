#ifndef __MEMPOLL__
#define __MEMPOLL__

#include <stdbool.h>
#include "IPC.h"

typedef struct s_memstat {
    double rUsed;
    double rTotal;

    double sUsed;
    double sTotal;

    double vUsed;
    double vTotal;
} memstat;

memstat* fetchMemStats ();
void processMem_use (memstat** prev, bool fancy, biDirPipe* pipe);
void async_processMem_use (void* args, biDirPipe* in);
#endif
