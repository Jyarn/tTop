#include <string.h>
#include <stdbool.h>

#include "misc.h"
#include "IPC.h"

typedef struct s_CPUstats {
    unsigned int pActive;
    unsigned int  pTotal;
    unsigned int active;
    unsigned int  total;
} CPUstats;

void async_processCPU_use (void* args, biDirPipe* pipe);
int printCPUHeader (CPUstats* prev, biDirPipe* pipe);
double getCPUstats (CPUstats* prev);
void processCPU_use (CPUstats* prevStats, bool fancy, biDirPipe* pipe);