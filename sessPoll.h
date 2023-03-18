#ifndef __SESSFETCH__
#define __SESSFETCH__

#include <utmp.h>
#include "IPC.h"
#define PROCNAMELEN 100

typedef struct s_sessInfo {
    char tty[UT_LINESIZE+1];

    char user[UT_NAMESIZE+1];
    char host[UT_NAMESIZE+1];

    char procName[PROCNAMELEN];
} sessInfo;

void async_processSess_use (void* args, biDirPipe* pipe);
int printSessUse (biDirPipe* pipe);
int processSess_Use (biDirPipe* pipe);

#endif