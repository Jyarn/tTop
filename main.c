#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#include <stdlib.h>

#include <time.h>

#define BUFFSZ 2048


#include "cpuPoll.h"
#include "sessPoll.h"
#include "memPoll.h"

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

void printMem (memstat* m) {
    const float conv = 1024.0*1024.0; // convert kb into gb
    printf("(total-free)/mtotal: %d/%d = %f\n", m->tMem - m->free, m->tMem, (float)(m->tMem-m->free) / (float)m->tMem);
    printf("used/mtotal: %d/%d = %f\n", m->used, m->tMem, (float)m->used / (float)m->tMem);
    free(m);
}

int main (int arc, char** argc) {
    // misc
    CPUstats cpuStats = { 0, 0 };

    // main loop
    for (int i = 0; i < 100; i++) {
        printf("\nPoll %d: \n+=====================================+\n", i+1);
        getCPUstats(&cpuStats);
        printf("Current CPU usage: %f\%\n", calculateCPUusage(cpuStats));
        sessInfo* s = fetchSess();
        printSess(s);
        memstat* m = fetchMemStats();
        printMem(m);
        sleep(1);
    }
}