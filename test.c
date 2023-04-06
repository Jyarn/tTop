#include <stdio.h>
#include "cpuPoll.h"
#include "IPC.h"

int main () {
    cmdArgs c = { 0, 0, 0};
    biDirPipe* p = NULL;
    printStr(p);
    killPipe(p);
}