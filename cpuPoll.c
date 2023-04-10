#include <string.h>
#include <stdio.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdbool.h>

#include "misc.h"
#include "cpuPoll.h"
#include "misc.h"
#include "IPC.h"

int fetchNCores () {
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo == NULL) { perror("ERROR: failed to open /proc/cpuinfo"); return -1; }


    int nCores = -1;
    int nRead;
    char* line = NULL;
    size_t n = 0;

    while ((nRead = getline(&line, &n, cpuinfo)) != -1) {
        if ( !strncmp(line, "cpu cores", strlen("cpu cores"))) {
            char* conv = filterString(line, nRead);
            nCores = strtol(conv, NULL, 10);
            free(conv);
            break;
        }
    }
    free(line);
    fclose(cpuinfo);
    return nCores;
}

void async_processCPU_use (void* args, biDirPipe* pipe) {
    CPUstats cpuUse = { 0, 0, 0, 0 };
    cmdArgs* arg = (cmdArgs*)args;
    getCPUstats(&cpuUse);

    for (int i = 0; i < arg->nSamples; i++) {
        sleep(arg->tDelay);
        printCPUHeader(&cpuUse, pipe);
        processCPU_use(&cpuUse, arg->fancy, pipe);
    }
}

void printCPUHeader (CPUstats* prev, biDirPipe* pipe) {
	/*
	* Print cpu usage
	* prev = previous cpu usage stats, used to calculate the current cpu usage
	*/
    char bff[2048];
    int bffPointer = 0;
    int nCores = fetchNCores();
	double cpuUsage = getCPUstats(prev);

    if (nCores == -1) {
        int nCores = sysconf(_SC_NPROCESSORS_CONF);
	    bffPointer += sprintf(bff, "Number of logical cores: %d\n", nCores);
    }
    else {
        bffPointer += sprintf(bff, "Number of cores: %d\n", nCores);
    }


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
    char bff[2048]; for (int i = 0; i < 2048; i++ ) { bff[i] = '\0'; }
    if (buffFRead(bff, "/proc/stat", 2048) == -1) {
        fprintf(stderr, "Unable able to read from /proc/stat");
        return calculateCPUusage(*prev);
    }

    char* flt = filterString(bff, 2048);

    colExtract(stats, 10, flt);

// active  = Total - Idle
// Total = Idle + Active

    prev->pActive = prev->active;
    prev->pTotal = prev->total;
                 //user     nice     system   iowait   irq      softirq
    prev->active = stats[0]+stats[1]+stats[2]+stats[4]+stats[5]+stats[6];
    prev->total = prev->active +stats[3];
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

        bffPointer += sprintf(bff, "\t%s - (%2.2f)\n", cpuView, currentUse);
    }
    bff[bffPointer] = '\0';
    writeStr(bff, pipe);
}