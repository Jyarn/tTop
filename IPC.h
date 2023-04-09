#ifndef __IPC__H
#define __IPC__H

#define NPIPES 3

typedef struct {
// bi-directional pipe
    int write;
    int read;
} biDirPipe;

typedef void (*job)(void*, biDirPipe*);
void printStr (biDirPipe* pipe);
int writeStr (char* str, biDirPipe* pipe);
int writePacket (int len, void* write, biDirPipe* out);
void* readPacket (biDirPipe* in);
void killPipe (biDirPipe** pipe);
biDirPipe* genChild (job childTask, void* args);
#endif