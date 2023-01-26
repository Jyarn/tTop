#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

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

    memstat* r = malloc(sizeof(memstat) );
    r->tMem = stats[0];
    r->free = stats[1];
    r->used = stats[0] - stats[1] - stats[3] - stats[4] - stats[21];

    r->swapTotal = stats[14];
    r->swapFree = stats[15];
    return r;
}
