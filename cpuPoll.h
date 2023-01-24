#include <string.h>
#include "misc.h"

typedef struct s_CPUstats {
    unsigned int pActive;
    unsigned int  pTotal;
    unsigned int active;
    unsigned int  total;
} CPUstats;

void getCPUstats (CPUstats* prev);
double calculateCPUusage(CPUstats stats);