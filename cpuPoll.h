#include <string.h>
#include "misc.h"
#include <stdbool.h>

typedef struct s_CPUstats {
    unsigned int pActive;
    unsigned int  pTotal;
    unsigned int active;
    unsigned int  total;
} CPUstats;

double getCPUstats (CPUstats* prev);
int processCPU_use (CPUstats* prevStats, bool fancy);