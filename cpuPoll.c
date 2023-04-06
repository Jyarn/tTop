#include <string.h>
#include <stdio.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdbool.h>

#include "misc.h"
#include "cpuPoll.h"
#include "misc.h"
#include "IPC.h"

void async_processCPU_use (void* args, biDirPipe* pipe) {
    CPUstats cpuUse = { 0, 0, 0, 0 };
    cmdArgs* arg = (cmdArgs*)args;

    for (int i = 0; i < arg->nSamples; i++) {
        printCPUHeader(&cpuUse, pipe);
        processCPU_use(&cpuUse, arg->fancy, pipe);
        sleep(arg->tDelay);
    }
}

void printCPUHeader (CPUstats* prev, biDirPipe* pipe) {
	/*
	* Print cpu usage
	* prev = previous cpu usage stats, used to calculate the current cpu usage
	*/
    char bff[2048];
    int bffPointer = 0;

	double cpuUsage = getCPUstats(prev);

	int nCores = sysconf(_SC_NPROCESSORS_CONF);
	bffPointer += sprintf(bff, "Number of logical cores: %d\n", nCores);
	bffPointer += sprintf(&bff[bffPointer], " total cpu use = %2.2f%c\n", cpuUsage, '%');
    bff[bffPointer] = '\0';
    writeStr(bff, pipe);
}

double calculateCPUusage(CPUstats stats) {
    /*
    * calculate cpu usage based on values provided by stats
    */
    double crrActive = (double)(stats.active - stats.pActive);
    double crrTotal  = (double)(stats.total - stats.pTotal);
    return (crrActive / crrTotal) * 100;
}

double getCPUstats (CPUstats* prev) {
    /*
        prev = pointer to a CPUstat to be updated with new usage stats
    */

    unsigned int stats[10];
    char bff[2048];
    if (buffFRead(bff, "/proc/stat", 2048) == -1) {
        fprintf(stderr, "Unable able to read from /proc/stat");
        return calculateCPUusage(*prev);
    }

    char* flt = filterString(bff, 2048);

    colExtract(stats, 10, flt);

    prev->pActive = prev->active;
    prev->pTotal = prev->total;

    prev->active = stats[0]+stats[1]+stats[2]+stats[5]+stats[6];
    prev->total = prev->active + stats[3]+stats[4];
    free(flt);
    return calculateCPUusage(*prev);
}
void processCPU_use (CPUstats* prevStats, bool fancy, biDirPipe* pipe) {
	/*
	 * Print cpu stats with --graphics enabled or disabled
	 *
	 * prevStats = previous raw cpu usage stats, used to calculate current cpu stats (assumes cpu stats are upto date)
	 * fancy = specifies if anyting is to printed
	 *
     *
	 * returns 1
	 */
    char bff[2048] = { 0 };
    int bffPointer = 0;

    if (fancy) {
        double currentUse = calculateCPUusage(*prevStats); // don't call getCPUstats because prevStats is up to date
		char cpuView[101];
		stringMult('|', (int)currentUse, cpuView);

        bffPointer += sprintf(bff, "\t%s /(%2.2f)\n", cpuView, currentUse);
    }
    bff[bffPointer] = '\0';
    writeStr(bff, pipe);
}