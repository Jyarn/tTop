#include <unistd.h>
#include <sys/utsname.h>
#include <stdio.h>

#include "IPC.h"
#include "misc.h"

void fetchSysInfo (biDirPipe* pipe) {
	/*
	* fetch system info, output should be similar to uname -a except formatted better
	*/
	struct utsname sysInfo;
	uname(&sysInfo);

	char bff[2048];

	bff[sprintf(bff, "OS: %s\n", sysInfo.sysname)] = '\0';
	writeStr(bff, pipe);

	bff[sprintf(bff, "Hostname: %s\n", sysInfo.nodename)] = '\0';
	writeStr(bff, pipe);

	bff[sprintf(bff, "Version: %s\n", sysInfo.version)] = '\0';
	writeStr(bff, pipe);

	bff[sprintf(bff, "Release: %s\n", sysInfo.release)] = '\0';
	writeStr(bff, pipe);

	bff[sprintf(bff, "Machine: %s\n", sysInfo.machine)] = '\0';
	writeStr(bff, pipe);
}

void async_processSys_stats (void* args, biDirPipe* pipe) {
	cmdArgs* arg = (cmdArgs*)args;
	for (int i = 0; i < arg->nSamples; i++) {
		fetchSysInfo(pipe);
	}
}