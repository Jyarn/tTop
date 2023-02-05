#include <string.h>
#include <stdio.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdbool.h>

#include "misc.h"
#include "cpuPoll.h"
#include "misc.h"

void getCPUstats (CPUstats* prev) {
    /*
        prev = pointer to a CPUstat to be updated with a new delta
    */

    unsigned int stats[10];
    char bff[2048];
    if (buffFRead(bff, "/proc/stat", 2048) == -1) {
        fprintf(stderr, "Unable able to read from /proc/stat");
        return;
    }

    char* flt = filterString(bff, 2048);

    colExtract(stats, 10, flt);

    prev->pActive = prev->active;
    prev->pTotal = prev->total;

    prev->active = stats[0]+stats[1]+stats[2]+stats[5]+stats[6];
    prev->total = prev->active + stats[3]+stats[4];
    free(flt);
}

double calculateCPUusage(CPUstats stats) {
    double crrActive = (double)(stats.active - stats.pActive);
    double crrTotal  = (double)(stats.total - stats.pTotal);
    return (crrActive / crrTotal) * 100;
}

int processCPU_use (CPUstats* prevStats, bool fancy) {
	/*
	 * Print cpu stats with --graphics enabled or disabled
	 *
	 * msg = message buffer always of size 2048. The string to be printed is written here
	 * prevStats = previous raw cpu usage stats, used to calculate current cpu stats
	 * prevUse = a pointer to previous cpu usage stats (as a percentage) used to calculate
	 *			 the change in cpu usage (cpu usage delta)
	 * fancy = specifies if additional information is to be printed (--graphics flag)
	 *
	 * returns number of lines that were printed (hardcoded)
	 */

    int lines = 1;

    if (fancy) {
        double currentUse = calculateCPUusage(*prevStats);
		char cpuView[101];
		stringMult('|', (int)currentUse, cpuView);

        printf("\t%s /(%2.2f)\n", cpuView, currentUse);
    }
    return lines;
}