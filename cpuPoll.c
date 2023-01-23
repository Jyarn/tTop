#include <string.h>

#include <unistd.h>

#include "misc.h"
#include "const.h"

#include "cpuPoll.h"

void getCPUstats (int fd, char* bff, CPUstats* prev) {
    /*
        fd = file descriptor for /proc/stat
        bff = buffer reading in files
        prev = pointer to a CPUstat to be updated with new delta
    */

    unsigned int stats[10];
    read(fd, bff, BUFFSZ);


    bff = strchr(bff, ' ');
    colExtract(stats, 10, bff);

    prev->pActive = prev->active;
    prev->pTotal = prev->total;

    prev->active = stats[0]+stats[1]+stats[2]+stats[5]+stats[6];
    prev->total = prev->active + stats[3]+stats[4];

    lseek(fd, 0, SEEK_SET);
}

double calculateCPUusage(CPUstats stats) {
    double crrActive = (double)(stats.active - stats.pActive);
    double crrTotal  = (double)(stats.total - stats.pTotal);
    return (crrActive / crrTotal) * 100;
}