#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "misc.h"
#include "memPoll.h"
#include "IPC.h"

void async_processMem_use (void* args, biDirPipe* in) {
    cmdArgs* arg = (cmdArgs*)args;
	memstat* memStats = fetchMemStats();

    for (int i = 0; i < arg->nSamples; i++ ) {
        sleep(arg->tDelay);
        processMem_use(&memStats, arg->fancy, in);
    }

    free(memStats);
}

memstat* fetchMemStats () {
    /*
    * report RAM, swap, and virtual memory usage in gigabytes, returns a memstat* containing this information
    */
    char bff[2048] = { 0 };
    if (buffFRead(bff, "/proc/meminfo", 2047) == -1) {
        fprintf(stderr, "Unable to open /proc/meminfo\n");
    }

    unsigned int stats[24];
    char* filteredBff = filterString(bff, 2048);
    colExtract(stats, 24, filteredBff);
    free(filteredBff);

    memstat* ret = malloc(sizeof(memstat) );

    ret->rUsed = stats[0] - stats[1] - stats[3] - stats[4] - stats[23] + stats[20];
    ret->rUsed /= 1024*1024; // convert to kb to gb
    ret->rTotal = (float)(stats[0]) / (1024.0f*1024.0f);

    ret->sTotal = (float)(stats[14]) / (1024.0f*1024.0f);
    ret->sUsed = ret->sTotal - (float)(stats[15]) / (1024.0f*1024.0f);

    ret->vUsed = ret->rUsed + ret->sUsed;
    ret->vTotal = ret->rTotal + ret->sTotal;

    return ret;
}

void processMem_use (memstat** prev, bool fancy, biDirPipe* pipe) {
    /*
    * print memory usage and if requested a bar representing the percentage of memory being used and the change in memory use
    * this additional information is specified by fancy
    */
	memstat* current = fetchMemStats();
    char bff[2048] = { 0 };
    int bffPointer = sprintf(bff, "%2.2f/%2.2f (GB) -- %2.2f/%2.2f (GB)", current->rUsed, current->rTotal, current->vUsed, current->vTotal);

    if (fancy) {
        char virtVis[201] = { 0 };

        double vPercent = current->vUsed / current->vTotal * 100;
        char vMarker = (current->vUsed - (*prev)->vUsed) < 0 ? '-' : '+';

        double delta = current->vUsed - (*prev)->vUsed;

        stringMult(vMarker, (int)vPercent, virtVis);
        bffPointer += sprintf(&bff[bffPointer], " | %s (%c%2.2f)", virtVis, vMarker, ABS(delta));
	}

    bff[bffPointer] = '\n';
    bff[bffPointer+1] = '\0';
    writeStr(bff, pipe);
    free(*prev);
    *prev = current;
}
