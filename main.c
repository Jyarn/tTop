#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/utsname.h>

#include <getopt.h>

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

int printCPUHeader (CPUstats* prev) {
	int jump = 2;

	getCPUstats(prev);
	double cpuUsage = calculateCPUusage(*prev);

	int nCores = sysconf(_SC_NPROCESSORS_CONF);
	printf("Number of logical cores: %d\n", nCores);


	printf(" total cpu use = %2.2f%c\n", cpuUsage, '%');
	return jump;
}

int printHeader (bool sequential, bool fancy, char stat, unsigned int samples, unsigned int delay, bool debug) {
	if (!debug) {
		// fetch memory usage
		char statm[50];
		buffFRead(statm, "/proc/self/statm", 49);
		statm[49] = '\0';
		unsigned int vmSize = strtol(statm, NULL, 10);

		printf("Nbr of samples: %d -- every %d secs\n", samples, delay);
		printf(" Memory usage: %d kilobytes\n", vmSize);
		return 2;
	}
	printf("sequential - %s\n", sequential ? "true" : "false");
	printf("graphics - %s\n", fancy ? "true" : "false");
	printf("stats - ");

	switch (stat) {
	  case 0:
	  	printf("Reporting System + User Stats!\n");
		break;
	  case 1:
	  	printf("Repoting System Stats Only!\n");
		break;
	  case 2:
		printf("Reporting User Stats Only!\n");
		break;
	  default:
	  	printf("????????\n");
		break;
	}

	printf("samples - %d\n", samples);
	printf("delay - %d\n", delay);
	return 5;
}

void pollUse (bool sequential, bool fancy, char stats, unsigned int samples, unsigned int delay, bool debug) {
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
	CPUstats cpuUse = { 0, 0, 0, 0 };
	int jump = 0;

	for (int i = 0; i < samples; i++) {
		jump = 1;
		printf("Poll %d\n", i+1);
		jump += printHeader(sequential, fancy, stats, samples, delay, debug);

		if (stats != 2) {
			printf("+-------------------------------------------------------+\n");
			jump += printCPUHeader(&cpuUse) + ((!fancy || sequential) ? 0 : samples);
			curJump(i, !fancy || sequential);

			processCPU_use(&cpuUse, fancy);
			curJump(samples-i-1, !fancy || sequential);

			printf("+-------------------------------------------------------+\n");
			printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
			curJump(i, sequential);

			processMem_use(&memStats, fancy);
			curJump(samples-i-1, sequential);
			jump += samples + 3;
		}
		if (!sequential) { printf("\x1b[0J"); }
		if (stats != 1) {
			printf("+-------------------------------------------------------+\n");
			printf("user desc.\n");

			jump += processSess_Use() + 3;
			printf("\n");
		}


		printf("+-------------------------------------------------------+\n");
		jump += fetchSysInfo() + 1;

		if (i+1 < samples) {
			sleep(delay);

			if (sequential) {
				printf("\n\n");
			}
			else {
				curJump(-jump, sequential);
			}
		}
	}
}

int strToInt (char* in) {
	int ret = strtol(in, NULL, 10);
	if ((!ret && in[0] == '0') || ret) { return ret; }
	return -1;
}

int processFlag (int argLen, int argPos, int argc, char** argv) {
	char c = argv[argPos][argLen];
	if (c == '\0' || c == '=' || ('0'<= c && c <= '9')) {
		int ret = -1;
		// handle cases: --samples 10  --samples= 10
		if (argPos+1 < argc) {
			ret = strToInt(argv[argPos+1]);
			if (ret != -1) { return ret; }
		}

		int adj = 0;
		if (argv[argPos][argLen] == '=') { adj = 1; }

		return strToInt(&argv[argPos][argLen+adj]);
	}
	return -1;
}

int main (int argc, char** argv) {
	bool sequential = false;
	bool fancy = false;
	bool debug = false;
	char stats = 0;
	unsigned int samples = 10;
	unsigned int delay = 1;

	bool accecptPostitional = true;

	for (int i = 1; i < argc; i++) {
		if ( !strcmp(argv[i], "--system") || !strcmp(argv[i], "--user")) {
			if (!stats) {
				stats = (!strcmp(argv[i], "--system")) ? 1 : 2;
			}
			else {
				fprintf(stderr, "ERROR: --system and --user passed\n");
			}
		}
		else if ( !strcmp(argv[i], "--graphics")) { fancy = true; }
		else if ( !strcmp(argv[i], "--sequential")) { sequential = true; }
		else if ( !strcmp(argv[i], "--DEBUG")) { debug = true; }
		else if ( !strncmp(argv[i], "--samples", strlen("--samples"))) {
			int t = processFlag(strlen("--samples"), i, argc, argv);
			if (t != -1) {
				accecptPostitional = false;
				samples = t;
			}
		}
		else if ( !strncmp(argv[i], "--tdelay", strlen("--tdelay"))) {
			int t = processFlag(strlen("--tdelay"), i, argc, argv);
			if (t != -1) {
				accecptPostitional = false;
				delay = t;
			}
		}
	}
	if (accecptPostitional && argc >= 2) {
		int t = 0;
		samples = ((t = strToInt(argv[argc-2])) == -1) ? sequential : t;
		delay = ((t = strToInt(argv[argc-1])) == -1) ? delay : t;
	}
	else if (accecptPostitional && argc == 1) {
		int t = 0;
		samples = ((t = strToInt(argv[0])) == -1) ? sequential : t;
	}

	pollUse(sequential, fancy, stats, samples, delay, debug);
}
