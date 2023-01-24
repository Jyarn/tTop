#include <string.h>
#include <stdio.h>

#include <unistd.h>

#include "misc.h"
#include "const.h"

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
    bff[2047] = '\0';

    colExtract(stats, 10, strchr(bff, ' '));

    prev->pActive = prev->active;
    prev->pTotal = prev->total;

    prev->active = stats[0]+stats[1]+stats[2]+stats[5]+stats[6];
    prev->total = prev->active + stats[3]+stats[4];
}

double calculateCPUusage(CPUstats stats) {
    double crrActive = (double)(stats.active - stats.pActive);
    double crrTotal  = (double)(stats.total - stats.pTotal);
    return (crrActive / crrTotal) * 100;
}