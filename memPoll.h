#ifndef __MEMPOLL__
#define __MEMPOLL__

// (mem)ory (stat)us
typedef struct s_memstat {
    unsigned int        tMem;
    unsigned int   swapTotal;

    unsigned int   available;
    unsigned int        free;
    unsigned int        used;

    unsigned int    swapFree;
} memstat;

memstat* fetchMemStats ();

#endif