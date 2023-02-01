#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/utsname.h>

#include "cpuPoll.h"
#include "sessPoll.h"
#include "memPoll.h"

int fetchSysInfo () {
	struct utsname sysInfo;
	uname(&sysInfo);

	printf("OS: %s\n", sysInfo.sysname);
	printf("Hostname: %s\n", sysInfo.nodename);
	printf("Version: %s\n", sysInfo.version);
	printf("Release: %s\n", sysInfo.release);
	printf("Machine: %s\n", sysInfo.machine);
	return 5;
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
	memstat* memStats = fetchMemStats();
    double prevUse = 0;

	int jump;

    // main loop
    for (int i = 0; i < samples; i++) {
        printf("Poll %d: \n+=====================================+\n", i+1);
		jump = 2;

		if (stats != 2) {
			jump += processCPU_use(&cpuStats, &prevUse, fancy) + 1;
			printf("+=====================================+\n");
			jump += processMem_use(memStats, fancy) + 1;
			printf("+=====================================+\n");
		}

		if (stats != 1) {
			jump += processSess_Use(fancy) + 1;
			printf("+=====================================+\n");
		}

		jump += fetchSysInfo();
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
