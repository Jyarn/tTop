#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "misc.h"
#include "memPoll.h"

memstat* fetchMemStats () {
    char bff[2048] = { 0 };
    if (buffFRead(bff, "/proc/meminfo", 2047) == -1) {
        fprintf(stderr, "Unable to open /proc/meminfo\n");
    }

    unsigned int stats[22];
    char* filteredBff = filterString(bff, 2048);
    colExtract(stats, 22, filteredBff);
    free(filteredBff);

    memstat* ret = malloc(sizeof(memstat) );

    ret->rUsed = stats[0] - stats[1] - stats[3] - stats[4] - stats[21];
    ret->rUsed /= 1024*1024; // convert to kb to gb
    ret->rTotal = (float)(stats[0]) / (1024.0f*1024.0f);

    ret->sTotal = (float)(stats[14]) / (1024.0f*1024.0f);
    ret->sUsed = ret->sTotal - (float)(stats[15]) / (1024.0f*1024.0f);

    ret->vUsed = ret->rUsed + ret->sUsed;
    ret->vTotal = ret->rTotal + ret->sTotal;

    return ret;
}

void processMem_use (memstat** prev, bool fancy) {
	memstat* current = fetchMemStats();
    printf("%2.2f/%2.2f (GB) -- %2.2f/%2.2f (GB)", current->rUsed, current->rTotal, current->vUsed, current->vTotal);

    if (fancy) {
        char virtVis[201] = { 0 };

        double vPercent = current->vUsed / current->vTotal * 50;
        char vMarker = (current->vUsed - (*prev)->vUsed) < 0 ? '-' : '+';

        double delta = current->vUsed - (*prev)->vUsed;

        stringMult(vMarker, (int)vPercent, virtVis);
        printf(" | %s (%c%2.2f)", virtVis, vMarker, ABS(delta));
	}

    printf("\n");
    free(*prev);
    *prev = current; // seg faulting
}
