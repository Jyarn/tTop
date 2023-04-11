#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>

#include <unistd.h>
#include <sys/utsname.h>

#include <getopt.h>

#include "cpuPoll.h"
#include "sessPoll.h"
#include "memPoll.h"
#include "sysPoll.h"
#include "IPC.h"

void terminate (int signum) {
	exit(0);
}

void confirmExit (int signum) {
	for (;;) {
		char bff[2048];
		int nRead;
		write(STDOUT_FILENO, "Would you like to exit (y/n) ", 30);
		if ((nRead = read(STDIN_FILENO, bff, 2047)) <= 0) {
			write(STDERR_FILENO, "ERROR: user exit confirm failed\n", 33);
		}

		bff[nRead-1] = '\0'; 	// remove '\n'
		if (!strcmp("y", bff) ) {
			if (killpg(getpgid(getpid()), SIGUSR1) == -1) {
				write(STDOUT_FILENO, "ERROR: unable to kill child processes\n", 39);
			}
			exit(0);
		}
		else if (!strcmp("n", bff) ) { write(STDOUT_FILENO, "continuing....\n", 16);
			return;
		}
		else { write(STDOUT_FILENO, "invalid response\n", 18); }
	}
}
int printHeader (unsigned int samples, unsigned int delay) {
	/*
	* Print the number of samples to be taken and their interval
	* If in debug mode print arguments passed to pollUse
	*/
	// fetch memory usage
	char statm[50];
	buffFRead(statm, "/proc/self/statm", 49);
	statm[49] = '\0';
	unsigned int vmSize = strtol(statm, NULL, 10);

	printf("Nbr of samples: %d -- every %d secs\n", samples, delay);
	printf(" Memory usage: %d kilobytes\n", vmSize);
	return 2;
}

void printSequential (bool fancy, char stats, unsigned int samples, unsigned int delay) {
	cmdArgs args = { fancy, delay, samples };
	biDirPipe* cpuPipe = NULL;
	biDirPipe* memPipe = NULL;
	biDirPipe* sesPipe = NULL;
	biDirPipe* sysPipe = genChild(async_processSys_stats, &args);

	biDirPipe* freeArr[NPIPES] = { NULL, NULL, NULL };
	writePacket(NPIPES*sizeof(biDirPipe*), freeArr, sysPipe);
	freeArr[0] = sysPipe;

	if (stats != 2) {
		cpuPipe = genChild(async_processCPU_use, &args);
		writePacket(NPIPES*sizeof(biDirPipe*), freeArr, cpuPipe);
		freeArr[1] = cpuPipe;

		memPipe = genChild(async_processMem_use, &args);
		writePacket(NPIPES*sizeof(biDirPipe*), freeArr, memPipe);
		freeArr[2] = memPipe;
	}
	if (stats != 1) {
		sesPipe = genChild(async_processSess_use, &args);
		writePacket(NPIPES*sizeof(biDirPipe*), freeArr, sesPipe);
	}

	//sleep(delay);

	for (int i = 0; i < samples; i++) {
		if (i != 0) { printf("\n\n"); }
		printf("Poll %d\n", i+1);

		printHeader(samples, delay);

		if (stats != 2) {
			printf("+-------------------------------------------------------+\n");
			printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
			printStr(memPipe);
		}
		if (stats != 1) {
			printf("+-------------------------------------------------------+\n");
			printf("### Sessions/users ###\n");
			printSessUse(sesPipe);
		}
		if (stats != 2) {
			printf("+-------------------------------------------------------+\n");
			printStr(cpuPipe);
			printStr(cpuPipe);
		}

		printf("+-------------------------------------------------------+\n");
		printStr(sysPipe);
		printStr(sysPipe);
		printStr(sysPipe);
		printStr(sysPipe);
		printStr(sysPipe);

		if (i < (samples-1)) {
			sleep(delay);
		}
	}

	killPipe(&cpuPipe);
	killPipe(&memPipe);
	killPipe(&sesPipe);
	killPipe(&sysPipe);
}

void printNotSequential (bool fancy, char stats, unsigned int samples, unsigned int delay) {
/*
 * this is essentially just a buffered version of printSequential
*/
	cmdArgs args = { fancy, delay, samples };
	biDirPipe* cpuPipe = NULL;
	biDirPipe* memPipe = NULL;
	biDirPipe* sesPipe = NULL;
	biDirPipe* sysPipe = genChild(async_processSys_stats, &args);
	biDirPipe* freeArr[NPIPES] = { NULL, NULL, NULL };

	writePacket(NPIPES*sizeof(biDirPipe*), freeArr, sysPipe);
	freeArr[0] = sysPipe;

	int cpuBffPtr = -1;
	int memBffPtr = -1;
	buffer memBff = NULL;
	buffer cpuBff = NULL;

	int jump;

	if (stats != 1) {
		sesPipe = genChild(async_processSess_use, &args);
		writePacket(NPIPES*sizeof(biDirPipe*), freeArr, sesPipe);
		freeArr[1] = sesPipe;
	}

	if (stats != 2) {
		memPipe = genChild(async_processMem_use, &args);
		writePacket(NPIPES*sizeof(biDirPipe*), freeArr, memPipe);
		freeArr[2] = memPipe;
		cpuPipe = genChild(async_processCPU_use, &args);
		writePacket(NPIPES*sizeof(biDirPipe*), freeArr, cpuPipe);

		cpuBffPtr = fancy ? 0 : -1;
		memBffPtr = 0;

		memBff = malloc(samples * sizeof(bffObject));
		cpuBff = fancy ? malloc(samples*sizeof(bffObject)) : NULL;
	}

	sleep(delay);

	for (int i = 0; i < samples; i++) {
		jump = 0;
		printf("Poll %d\n", i+1);
		jump += printHeader(samples, delay) + 1;

		if (stats != 2) {
			memBff[memBffPtr++] = readPacket(memPipe);
			printf("+-------------------------------------------------------+\n");
			printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
			jump += 2;

			for (int j = 0; j < memBffPtr; j++ ) {
				printf("%s", memBff[j] ? memBff[j] : "NULL\n");
				jump++;
			}
		}
		if (stats != 1) {
			printf("+-------------------------------------------------------+\n");
			printf("### Sessions/users ###\n");
			jump += printSessUse(sesPipe) + 2;
		}
		if (stats != 2) {
			printf("+-------------------------------------------------------+\n");
			printStr(cpuPipe); // prints 2 lines
			jump += 3;

			if (fancy) {
				cpuBff[cpuBffPtr++] = readPacket(cpuPipe);
				for (int j = 0; j < cpuBffPtr; j++) {
					printf("%s", cpuBff[j] ? cpuBff[j] : "NULL\n");
					jump++;
				}
			}
			else {
				void * temp = readPacket(cpuPipe);	// flush '\0' character
				free(temp);
			}
		}

		printf("+-------------------------------------------------------+\n");
		printStr(sysPipe);
		printStr(sysPipe);
		printStr(sysPipe);
		printStr(sysPipe);
		printStr(sysPipe);
		jump += 6;

		if (i < (samples-1)) {
			printf("\x1b[%dA", jump);
			printf("\x1b[0J");
			sleep(delay);
		}
	}

	killPipe(&cpuPipe);
	killPipe(&memPipe);
	killPipe(&sesPipe);
	killPipe(&sysPipe);

	for (int i = 0; i < memBffPtr; i++ ) {
		free(memBff[i]);
	}
	for (int i = 0; i < cpuBffPtr; i++ ) {
		free(cpuBff[i]);
	}

	free(memBff);
	free(cpuBff);
}

int main (int argc, char** argv) {
	struct sigaction newAct;

	// set ctrl-c to confirmExit
	newAct.sa_handler = confirmExit;
	sigemptyset(&newAct.sa_mask);
	newAct.sa_flags = 0;
	sigaction(SIGINT, &newAct, NULL);

	// set SIGUSR1 to terminate
	newAct.sa_handler = terminate;
	sigaction(SIGUSR1, &newAct, NULL);

	// block ctrl-z
	sigset_t block;
	sigemptyset(&block);
	sigaddset(&block, SIGTSTP);
	sigprocmask(SIG_BLOCK, &block, NULL);

	// begin command line argument processing
	bool sequential = false;
	bool fancy = false;
	char stats = 0;
	unsigned int samples = 10;
	unsigned int delay = 1;

	bool acceptPostitional = true;

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
		else if ( !strncmp(argv[i], "--samples", strlen("--samples"))) {
			int t = processFlag(strlen("--samples"), i, argc, argv);
			if (t != -1) {
				acceptPostitional = false;
				samples = t;
			}
		}
		else if ( !strncmp(argv[i], "--tdelay", strlen("--tdelay"))) {
			int t = processFlag(strlen("--tdelay"), i, argc, argv);
			if (t != -1) {
				acceptPostitional = false;
				delay = t;
			}
		}
	}
	// handles positional arguments
	if (acceptPostitional && argc >= 2) {
		int t = 0;
		t = strToInt(argv[argc-2]);
		if (t == -1) {
			samples = ((t = strToInt(argv[argc-1])) == -1) ? samples : t;
		}
		else {
			samples = t;
			delay = ((t = strToInt(argv[argc-1])) == -1) ? delay : t;
		}
	}
	if (sequential) {
		printSequential(fancy, stats, samples, delay);
	}
	else {
		printNotSequential(fancy, stats, samples, delay);
	}
}
