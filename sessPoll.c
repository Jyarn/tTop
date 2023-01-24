#include "const.h"
#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <stdio.h>

#include "misc.h"

void printUtmp (int sz, char* in) {
    for (int i = 0; i < sz; i++) {
        printf("%c", in[i]);
    }
}

int fetchSess (int fd) {
   setutent();
   struct utmp* u;

    for ( ;; ) {
        u = getutent();
        if (!u) { break; }

        if (u->ut_type != 8) {
            printf("%d (%d): ", u->ut_pid, u->ut_type);
            printUtmp(32, u->ut_user);
            char p[50];
            sprintf(p, "/proc/%d/cmdline", u->ut_pid);
            char a[2048];

            buffFRead(a, p, 2048);
            int r = a[2047] = '\0';


            if (r != -1) {
                printf(" [%s]", a);
            }
            printf("\n");
        }
    }

    printf("\n----------------------\n");

    return 0;
}