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
/*
struct utmp* fetchSess (char user[UT_NAMESIZE + 1], ) {
    setutent();
    struct utmp* u = getutent();
    sessInfo* head = NULL; // return this

    for (; u != NULL; u = getutent()) {
        // link new node with head
        sessInfo* n = malloc(sizeof(sessInfo));
        n->next = head;
        n->pid = u->ut_pid;
        n->type = u->ut_type;

        n->user = malloc((UT_NAMESIZE + 1)*sizeof(char) );
        strncpy(n->user, u->ut_user, 32);
        n->user[UT_NAMESIZE] = '\0';

        char path[32];
        sprintf(path, "/proc/%d/cmdline", u->ut_pid);
        n->procName = malloc(2048*sizeof(char));
        if (buffFRead(n->procName, path, 2048) == -1) {
            // read failed, free memory
            free(n->procName);
            n->procName = NULL;
        }

        head = n;
    }

    endutent();
    return head;
}
*/
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
            if (u->ut_addr_v6[0] || u->ut_addr_v6[1] || u->ut_addr_v6[2] || u->ut_addr_v6[3] != 0) {
                printf("%s\t%s\t(%d.%d.%d.%d)\n", s->user, s->host, u->ut_addr_v6[0], u->ut_addr_v6[1], u->ut_addr_v6[2], u->ut_addr_v6[3]);
            }
            else {
                printf("%s\t%s\t(%s(%d)%s)\n", s->user, s->host, s->procName, u->ut_pid, s->id);
            }
        }

        u = getutent();
    }

    endutent();
    return lines;
}