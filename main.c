#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#include <stdlib.h>

#include <time.h>

#define BUFFSZ 2048


#include "cpuPoll.h"
#include "sessPoll.h"

#include "const.h"

void printSess (sessInfo* s) {
    sessInfo* p = s;
    while (s != NULL) {
        printf("%d(%d) %s-->%s\n", s->pid, s->type, s->user, s->procName);
        s = s->next;
        free(p);
        p = s;
    }
}

int main (int arc, char** argc) {
    // misc
    CPUstats cpuStats = { 0, 0 };

    // main loop
    for (int i = 0; i < 5; i++) {
        printf("\nPoll %d: \n+=====================================+\n", i+1);
        getCPUstats(&cpuStats);
        printf("Current CPU usage: %f\%\n", calculateCPUusage(cpuStats));
        sessInfo* s = fetchSess();
        printSess(s);
        sleep(1);
    }
}