#include <string.h>
#include <stdlib.h>

void colExtract (unsigned int* ret, int sz, char* bff) {
    char* lp = NULL;

    for (int i = 0; i < 10; i++) {
        ret[i] = strtol(&bff[1], &lp, 10);
        bff = lp;
        bff = strchr(bff, ' ');
    }
}