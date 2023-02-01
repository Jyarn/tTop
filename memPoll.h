#ifndef __MEMPOLL__
#define __MEMPOLL__

#include <stdbool.h>

typedef struct s_memstat {
    double rUsed;
    double rTotal;

    double sUsed;
    double sTotal;

    double vUsed;
    double vTotal;
} memstat;

memstat* fetchMemStats ();
int processMem_use (memstat* prev, bool fancy);

#endif
