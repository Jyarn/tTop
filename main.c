#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#include <stdlib.h>

#include <time.h>

#define BUFFSZ 2048


#include "cpuPoll.h"
#include "usrPoll.h"

#include "const.h"

int main (int arc, char** argc) {
    // initialize message buffer
    char* in = malloc(BUFFSZ);

    // define file descriptors
    int cpuStat = open("/proc/stat", O_RDONLY);
    int memStat = open("/proc/meminfo", O_RDONLY);

    // misc
    CPUstats cpuStats = { 0, 0 };
    pid* usrStat = NULL;

    // main loop
    for (int i = 0; i < 100; i++) {
        //getCPUstats(cpuStat, in, &cpuStats);
        //printf("Current CPU usage: %f\%\n", calculateCPUusage(cpuStats));
        getProc(&usrStat);
        sleep(1);
    }

    free(in);
    close(cpuStat);
}