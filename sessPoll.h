#ifndef __SESSFETCH__
#define __SESSFETCH__

#include <utmp.h>
#define PROCNAMELEN 2048

typedef struct s_sessInfo {
    char tty[UT_LINESIZE+1];

    char user[UT_NAMESIZE+1];
    char host[UT_NAMESIZE+1];

    char procName[PROCNAMELEN];
} sessInfo;

int processSess_Use ();

#endif