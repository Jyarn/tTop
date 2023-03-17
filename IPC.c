#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IPC.h"

int writeStr (char* str, biDirPipe* pipe) {
    return writePacket(strlen(str)+1, str, pipe);
}

int writePacket (int len, void* bff, biDirPipe* out) {
    write(out->write, &len, sizeof(int));
    return write(out->write, bff, len);
}

void* readPacket (biDirPipe* in) {
/*
 * first 64 bits determine package size and the
 * rest is read and inputed into the buffer and returned
*/

    int bffSize;
    read(in->read, &bffSize, sizeof(int));
    void* bff = malloc(bffSize);
    read(in->read, bff, bffSize);
    return bff;
}

void killPipe (biDirPipe* pipe) {
    close(pipe->read);
    close(pipe->write);
    free(pipe);
}

biDirPipe* genChild (job childTask, void* args) {
    int child[2];
    int parent[2];

    if (pipe(child) == -1) { perror("pipe 1"); }
    if (pipe(parent) == -1) { perror("pipe 2"); }

    biDirPipe* ret = malloc(sizeof(biDirPipe));

    pid_t childPID = fork();

    if (childPID < 0) {
        perror("fork");
        return NULL;
    }
    else if (!childPID) { // child
        ret->read = parent[0];
        ret->write = child[1];
        close(parent[1]);
        close(child[0]);
        childTask(args, ret);
        killPipe(ret);
        exit(0);
    }
    else { // parent
        ret->read = child[0];
        ret->write = parent[1];
        close(child[1]);
        close(parent[0]);
        return ret;
    }
}