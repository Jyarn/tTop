#include "const.h"
#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>

int fetchSess (int fd) {
    struct stat fInfo;
    stat("/var/run/utmp", &fInfo);

    struct utmp* ret = malloc(fInfo.st_size);
    read(fd, ret, fInfo.st_size);

    float numSess = fInfo.st_size / sizeof(struct utmp); // number of user sessions
    printf("There are %f user session(s)\n\n", numSess);

    for (int i = 0; i < numSess; i++) {
        printf("%s\n", ret[i].ut_user);
    }


    free(ret);
    return 0;
}