#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "IPC.h"

int writePacket (int len, void* write, biDirPipe* out) {
    fwrite(&len, sizeof(int), 1, out->write);
    return fwrite(write, len, 1, out->write);
}

void* readPacket (biDirPipe* in) {
/*
 * first 64 bits determine package size and the
 * rest is read and inputed into the buffer and returned
*/

    int bffSize;
    fread(&bffSize, sizeof(int), 1, in->read);
    void* bff = malloc(bffSize);
    fread(bff, 1, bffSize, in->read);
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

    pid_t child = fork();

    if (child < 0) {
        perror("fork");
        return NULL;
    }
    else if (!child) { // child
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