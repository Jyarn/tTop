#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>

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

void stringMult (char multend, int n, char* out) {
	// equivalent to python's string multiplication thing
	// ex. 'a' * 5 == aaaaa
	// output written into out, never checks if writing is inbounds
	// also null terminates

	int i;
	for (i = 0; i < n; i++) {
		out[i] = multend;
	}

	out[i] = '\0';
}

int processCPU_use (CPUstats* prevStats, double* prevUse, bool fancy) {
	/*
	 * Print cpu stats with --graphics enabled or disabled
	 *
	 * msg = message buffer always of size 2048. The string to be printed is written here
	 * prevStats = previous raw cpu usage stats, used to calculate current cpu stats
	 * prevUse = a pointer to previous cpu usage stats (as a percentage) used to calculate
	 *			 the change in cpu usage (cpu usage delta)
	 * fancy = specifies if additional information is to be printed (--graphics flag)
	 *
	 * returns number of lines that were printed (hardcoded)
	 */

    getCPUstats(prevStats);
    double currentUse = calculateCPUusage(*prevStats);

    if (fancy) {
		char cpuView[101];
		char cpuDelta[101];

		stringMult('|', (int)currentUse, cpuView);

        double delta = currentUse - *prevUse;
		char marker = delta < 0 ? '-' : '+';
		stringMult(marker, ABS((int)delta ), cpuDelta);

		printf("total cpu = %2.2f%c\n%s (%c%2.2f)\n%s\n", currentUse, '%', cpuDelta, marker, ABS(delta), cpuView);
        *prevUse = currentUse;

        return 3;
    }

    printf("total cpu use = %2.2f%c\n", currentUse, '%');
    return 1;
}

void pollUse (bool sequential, bool fancy, char stats, unsigned int samples, unsigned int delay) {
/*
 * Main loop for printing to screen. Takes in a series of arguments
 * sequential = print without escapes codes as though output is being redirected into a file
 * fancy = --graphics (print all data)
 * stats = 0 - print system and user, 1 - print system only, 2 - print user only
 * samples = number of poll(cycle) to perform before averaging out the results (assumed to be an unsigned int)
 * delay = time in-between each poll in seconds (assumed to be an unsigned int)
 *
 * At the end of the program, the screen will be cleared and will be averaged at the end and displayed
 */

    CPUstats cpuStats = { 0, 0 };
    double prevUse = 0;

	int jump;

    // main loop
    for (int i = 0; i < samples; i++) {
        printf("Poll %d: \n+=====================================+\n", i+1);
		jump = 2;
		if (stats != 2) {
			jump += processCPU_use(&cpuStats, &prevUse, true);
		}

		sleep(delay);

		if (!sequential && i+1 < samples) {
			printf("\x1b[%dA\x1b[0J", jump); // move cursor "jump" lines up and clear the screen
		}
		else {
			printf("\n\n");
		}
	}
}

int main (int arc, char** argc) {
	pollUse(false, true, 0, 5, 1);
}
