#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "IPC.h"

void printStr (biDirPipe* pipe) {
    void* bff = readPacket(pipe);
    printf("%s", (char* )bff);
    free(bff);
}

int writeStr (char* str, biDirPipe* pipe) {
    return writePacket(strlen(str)+1, str, pipe);
}

int writePacket (int len, void* bff, biDirPipe* out) {
    if (write(out->write, &len, sizeof(int)) == -1) { perror("ERROR: writing integer to pipe"); }
    int lines = 0;
    if ((lines = write(out->write, bff, len)) == -1) { perror("ERROR: writing message to pipe"); }
    return lines;
}

void* readPacket (biDirPipe* in) {
/*
 * first 64 bits determine package size and the
 * rest is read and inputed into the buffer and returned
*/
    if (in == NULL) { return '\0'; }
    int bffSize;
    if (read(in->read, &bffSize, sizeof(int)) <= 0) {
        perror("ERROR: unable to read packet size");
        return NULL;
    }
    void* bff = malloc(bffSize);
    if (read(in->read, bff, bffSize) <= 0) {
        perror("ERROR: unable to read message");
        free(bff);
        return NULL;
    }
    return bff;
}

void killPipe (biDirPipe* pipe) {
    if (close(pipe->read) == -1) { perror("ERROR: read pipe close failed"); }
    if (close(pipe->write) == -1) { perror("ERROR: write pipe close failed"); }
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
        perror("ERROR: fork failed");
        return NULL;
    }
    else if (!childPID) { // child
	// block ctrl-c
	sigset_t block;
	sigemptyset(&block);
	sigaddset(&block, SIGINT);
	sigprocmask(SIG_BLOCK, &block, NULL);

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
