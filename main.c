#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <time.h>

#include "cpuPoll.h"
#include "sessPoll.h"
#include "memPoll.h"

#define ABS(a) (a < 0 ? -a : a)

/*
void printSess (sessInfo* s) {
    sessInfo* p = s;
    while (s != NULL) {
        printf("%d(%d) %s-->%s\n", s->pid, s->type, s->user, s->procName);
        s = s->next;
	    free(p->user);
	    free(p->procName);
        free(p);
        p = s;
    }
}
*/

/*
void printMem (memstat* m) {
    const float conv = 1024.0*1024.0; // convert kb into gb
    printf("(total-free)/mtotal: %d/%d = %f\n", m->tMem - m->free, m->tMem, (float)(m->tMem-m->free) / (float)m->tMem);
    printf("used/mtotal: %d/%d = %f\n", m->used, m->tMem, (float)m->used / (float)m->tMem);
    free(m);
}
*/

char* processCPU_use (CPUstats* prevStats, double* prevUse, bool fancy) {
    getCPUstats(prevStats);
    double crrActive = (double)(prevStats->active - prevStats->pActive);
    double crrTotal  = (double)(prevStats->total - prevStats->pTotal);
    double currentUse = (crrActive / crrTotal)*100;

    char* body = malloc(sizeof(char)*23 ); // allocate enough space for "total cpu use = __.__%"

    if (fancy) {
        char* cpuView = malloc(sizeof(char)*100 );
        char* cpuDelta = malloc(sizeof(char)*100 );
        char* numDelta = malloc(sizeof(char)*6 );

        int i = 0;

        for (i = 0; i < (int)currentUse; i++) {
            cpuView[i] = '|';
        }

        cpuView[i] = '\0';


        double delta = currentUse - *prevUse;
        char marker = delta < 0 ? '-' : '+';
        for (i = 0; i < ABS(-1*delta); i++) {
            cpuDelta[i] = marker;

        }

        cpuDelta[i] = '\0';

        sprintf(numDelta, "%c%2.2f", marker, ABS(delta));
        printf("(%s) total cpu use = %2.2f%\n%s\n%s\n", numDelta,currentUse, cpuView, cpuDelta);

        *prevUse = currentUse;
        free(cpuView);
        free(cpuDelta);
        free(numDelta);

        return NULL;
    }

    printf("total cpu use = %2.2f%\n", currentUse);
    return NULL;
}

int main (int arc, char** argc) {
    // misc
    CPUstats cpuStats = { 0, 0 };
    double prevUse = 0;

    // main loop
    for (int i = 0; i < 100; i++) {
        printf("\nPoll %d: \n+=====================================+\n", i+1);
        processCPU_use(&cpuStats, &prevUse, true);
        sleep(1);
    }
}
