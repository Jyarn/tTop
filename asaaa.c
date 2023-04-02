#include "misc.h"
#include <string.h>

int main () {
    char cpuInfo[4096];
    buffFRead(cpuInfo, "/proc/cpuinfo", 4096);

    // find field "core id"
    for (int i = 0; i < 4096-7; i++) {
        strncmp(cpuInfo, "cpu cores", 10);
    }
}