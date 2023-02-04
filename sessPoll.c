#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "misc.h"
#include "sessPoll.h"

void fetchProcName (char* bff, int bffSz, int pid) {
    char path[32];
    sprintf(path, "/proc/%d/cmdline", pid);

    // since we're do need the output to print to the screen
    // we might as well consider 0 and -1 a failure to read
    if (buffFRead(bff, path, bffSz - 1) <= 0) {
        bff[0] = '\0';
        return ;
    }

    bff[bffSz-1] = '\0';
}

sessInfo* processUTMP (struct utmp* u) {
    sessInfo* n = malloc(sizeof(sessInfo));

    strncpy(n->tty, u->ut_line, UT_LINESIZE);
    n->user[UT_LINESIZE] = '\0';

    strncpy(n->id, u->ut_id, 4);
    n->id[4] = '\0';

    strncpy(n->user, u->ut_user, UT_NAMESIZE);
    n->user[UT_NAMESIZE] = '\0';

    strncpy(n->host, u->ut_host, 32);
    n->user[UT_NAMESIZE] = '\0';

    fetchProcName(n->procName, PROCNAMELEN, u->ut_pid);
    return n;
}

int processSess_Use () {
    int lines = 0;

    setutent();
    struct utmp* u = getutent();

    while (u != NULL) {
        if (u->ut_type == USER_PROCESS) {
            lines++;

            sessInfo* s = processUTMP(u);
            printf("%s\t%s\t%s\n", s->user, s->tty, (s->host[0] ? s->host : s->procName) );
        }

        u = getutent();
    }

    endutent();
    return lines;
}