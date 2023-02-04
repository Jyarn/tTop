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

void curJump (int l, bool sequential) {
	if (!sequential) {
		if (l > 0) {
			for (int i = 0; i < l; i++) {
				printf("\n");
			}
		}

		else if (l < 0) {
			for (int i = 0; i < -l; i++) {
				printf("\x1b[1A");
			}
		}
	}
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
	memstat* memStats = fetchMemStats();
	int jump = 0;

	for (int i = 0; i < samples; i++) {
		jump = 1;
		printf("Poll %d\n", i+1);

		if (stats != 2) {
			printf("+-------------------------------------------------------+\n");
			printf("cpu desc.\n");
			curJump(i, sequential);

			printf("cpu %d", i+1);
			curJump(samples-i, sequential);

			printf("+-------------------------------------------------------+\n");
			printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
			curJump(i, sequential);

			processMem_use(&memStats, fancy);
			curJump(samples-i, sequential);
			jump += samples*2 + 4;
		}
		if (!sequential) { printf("\x1b[0J"); }
		if (stats != 1) {
			printf("+-------------------------------------------------------+\n");
			printf("user desc.\n");

			jump += processSess_Use() + 3;
			printf("\n");
		}


		//printf("+-------------------------------------------------------+\n");
		//jump += fetchSysInfo() + 1;

		if (i+1 < samples) {
			sleep(delay);
			curJump(-jump, sequential);
		}
	}
}

int main (int arc, char** argc) {
	pollUse(false, true, 0, 5, 2);
}
