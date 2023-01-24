#include "const.h"
#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "misc.h"
#include "sessPoll.h"

void printUtmp (int sz, char* in) {
    for (int i = 0; i < sz; i++) {
        printf("%c", in[i]);
    }
}

sessInfo* fetchSess () {
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

    return head;
}