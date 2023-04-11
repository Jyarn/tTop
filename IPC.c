#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "IPC.h"

void printStr (biDirPipe* pipe) {
 /*
  * print a string from pipe
 */
    void* bff;
    if ((bff = readPacket(pipe)) == NULL) { return; }
    printf("%s", (char* )bff);
    free(bff);
}

int writeStr (char* str, biDirPipe* pipe) {
/*
 * write a string to pipe
*/
    return writePacket(strlen(str)+1, str, pipe);
}

int writePacket (int len, void* bff, biDirPipe* out) {
/*
 * write a packet to out
 * first write an int indidcating the size of the packet
 * then write the object into out
*/
    if (out == NULL) { fprintf(stderr, "ERROR: write to NULL pipe is invalid\n"); return -1; }
    if (write(out->write, &len, sizeof(int)) == -1) { perror("ERROR: writing integer to pipe"); }
    int lines = 0;
    if ((lines = write(out->write, bff, len)) == -1) { perror("ERROR: writing message to pipe"); }
    return lines;
}

void* readPacket (biDirPipe* in) {
/*
 * read a packet from (biDirPipe*)in
 * the first sizeof(int) bits determine package size and the
 * rest is read and inputed into a buffer and returned
*/
    if (in == NULL) { fprintf(stderr, "ERROR: read from NULL pipe is invalid\n"); return NULL; }
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

void killPipe (biDirPipe** pipe) {
/*
 * free all FDs in the biDirPipe
 * and free the pipe, and set it *pipe to NULL
*/
    if (pipe == NULL) { return ; }
    if (*pipe == NULL) { return ; }
    if (close((*pipe)->read) == -1) { perror("ERROR: read pipe close failed"); return ; }
    if (close((*pipe)->write) == -1) { perror("ERROR: write pipe close failed"); return ; }
    free(*pipe);
    *pipe = NULL;
}

biDirPipe* genChild (job childTask, void* args) {
/*
 * fork wrapper
 * the child runs the function pointed by childTask, and is passed
 * args and a biDirPipe
 * the parent exits genChild, and is given the other end of
 * the biDirPipe
 *
 * After exiting the child expects an biDirPipe* [NPIPES] array, which
 * are all the other biDirPipes allocated in the main program. This is done
 * to prevent memory leaks (since the child receives a duplicate of the parents memory)
*/
    int child[2];
    int parent[2];

    if (pipe(child) == -1) { perror("pipe 1"); }
    if (pipe(parent) == -1) { perror("pipe 2"); }

    biDirPipe* ret = malloc(sizeof(biDirPipe));

    pid_t childPID = fork();

    if (childPID < 0) {
        perror("ERROR: fork (genChild) failed");
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

        biDirPipe** freeArr = readPacket(ret);
        for (int i = 0; i < NPIPES; i++ ) {
            killPipe(freeArr+i);
        }
        free(freeArr);
        killPipe(&ret);
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
