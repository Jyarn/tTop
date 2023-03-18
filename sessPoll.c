#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "misc.h"
#include "sessPoll.h"
#include "IPC.h"

void async_processSess_use (void* args, biDirPipe* pipe) {
    cmdArgs* arg = (cmdArgs* )args;
    for (int i = 0; i < arg->nSamples; i++ ) {
        processSess_Use(pipe);
        sleep(arg->tDelay);
    }
}

int printSessUse (biDirPipe* pipe) {
    int lines = 0;
    void* line = readPacket(pipe);

    while (*(char* )line != 0) {
        printf(line);
        lines++;
        line = readPacket(pipe);
    }

    return lines;
}

void fetchProcName (char* bff, int bffSz, int pid) {
    /*
    * fetch commandline arguments for pid,
    * return value will be written into bff and bytes written will be < bffSz
    * always null terminated
    */
    char path[32];
    sprintf(path, "/proc/%d/cmdline", pid);

    if (buffFRead(bff, path, bffSz - 1) <= 0) {
        // read failed
        // null terminate first character and return
        bff[0] = '\0';
        return ;
    }

    bff[bffSz-1] = '\0';
}

sessInfo* processUTMP (struct utmp* u) {
    /*
    * create a sessInfo struct and fill each entry with null terminated strings
    * n.tty = ut_line
    * n.user = ut_user
    * n.host = ut_host
    * n.procName = /proc/ut_pid/cmdline
    */
    sessInfo* n = malloc(sizeof(sessInfo));

    strncpy(n->tty, u->ut_line, UT_LINESIZE);
    n->tty[UT_LINESIZE] = '\0';

    strncpy(n->user, u->ut_user, UT_NAMESIZE);
    n->user[UT_NAMESIZE] = '\0';

    strncpy(n->host, u->ut_host, 32);
    n->host[UT_NAMESIZE] = '\0';

    fetchProcName(n->procName, PROCNAMELEN, u->ut_pid);
    return n;
}

int processSess_Use (biDirPipe* pipe) {
    /*
    * process utmp entries
    * only print user processes and print lines in this format:
    * ut_user ut_line s,
    * where s = ut_host if not NULL, otherwise it will be the string /proc/ut_pid/cmdline
    *
    * return number of lines printed to screen to be used by pollUse
    */
    int lines = 0;

    setutent(); // rewind file pointer to beggining of utmp file
    struct utmp* u = getutent();

    char bff[2048];
    int bffPointer;

    while (u != NULL) {
        bffPointer = 0;

        if (u->ut_type == USER_PROCESS) {
            lines++;

            sessInfo* s = processUTMP(u);
            bffPointer = sprintf(bff, "%-8s\t%s\t%s\n", s->user, s->tty, (s->host[0] ? s->host : s->procName) );
            bff[bffPointer] = '\0';

            writeStr(bff, pipe);
            free(s);
        }

        u = getutent();
    }

    endutent(); // close utmp file
    char w = '\0';
    writePacket(sizeof(char), &w, pipe);
    return lines;
}
