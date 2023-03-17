#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

#include "IPC.h"


void task (void* args, biDirPipe* in) {
    printf(args);
    for (int i = 0; i < 7; i++) {
        void* temp = readPacket(in);
        printf(temp);
        free(temp);
    }

    writePacket(12, "exiting...\n", in);
}

int main () {
    biDirPipe* pipe = genChild(task, "start\n");
    writeStr("loop 1\n", pipe);
    writeStr("loop 2\n", pipe);
    writeStr("loop 3\n", pipe);
    writeStr("loop 4\n", pipe);
    writeStr("loop 5\n", pipe);
    writeStr("loop 6\n", pipe);
    writeStr("sending kill signal\n", pipe);

    void* bff;
    printf(bff = readPacket(pipe));
    free(bff);

    printf("main exit\n");
    killPipe(pipe);
}