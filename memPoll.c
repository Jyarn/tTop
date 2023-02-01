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

int processMem_use (memstat* prev, bool fancy) {
	memstat* current = fetchMemStats();

	char ramVis[51] = { 0 }; // RAM memory Visualiation
	char swapVis[51] = { 0 };
	char virtVis[51] = { 0 };

    double rPercent = current->rUsed / current->rTotal * 50;
    double sPercent = current->sUsed / current->sTotal * 50;
    double vPercent = current->vUsed / current->vTotal * 50;

	if (fancy) {
		stringMult('|', (int)rPercent, ramVis);
        stringMult('|', (int)sPercent, swapVis);
        stringMult('|', (int)vPercent, virtVis);
	}

    char rMarker = (current->rUsed - prev->rUsed) < 0 ? '-' : '+';
    char sMarker = (current->sUsed - prev->sUsed) < 0 ? '-' : '+';
    char vMarker = (current->vUsed - prev->vUsed) < 0 ? '-' : '+';

    printf("Physical (GB): %2.2f/%2.2f - %s\n", current->rUsed, current->rTotal, ramVis);
    printf("(%c%2.2f)\n\n", rMarker, ABS(current->rUsed - prev->rUsed));

    printf("Swap (GB): %2.2f/%2.2f - %s\n", current->sUsed, current->sTotal, swapVis);
    printf("(%c%2.2f)\n\n", sMarker, ABS(current->sUsed - prev->sUsed));

    printf("Virtual (GB): %2.2f/%2.2f - %s\n", current->vUsed, current->vTotal, virtVis);
    printf("(%c%2.2f)\n", vMarker, ABS(current->vUsed - prev->vUsed));

    free(prev);
    *prev = *current;
    return 8;
}
