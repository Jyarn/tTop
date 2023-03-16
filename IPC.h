#ifndef __IPC__H
#define __IPC__H

typedef struct {
    int len;
    void* msg;
} packet;

typedef struct {
// bi-directional pipe
    int write;
    int read;
} biDirPipe;

typedef void (*job)(void*, biDirPipe*);

int writePacket (int len, void* write, biDirPipe* out);
void* readPacket (biDirPipe* in);
void killPipe (biDirPipe* pipe);
biDirPipe* genChild (job childTask, void* args);
#endif