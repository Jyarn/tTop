#ifndef __MISC__
#define __MISC__

#include <stdbool.h>

#define ABS(a) ((a) < 0 ? (-a) : (a)) // absolute value function
typedef char** buffer;
typedef char* bffObject;

typedef struct {
    bool            fancy;
    unsigned int    tDelay;
    unsigned int    nSamples;
} cmdArgs;

void colExtract (unsigned int* ret, int sz, char* bff);
int isNum (char* chk);
int buffFRead (char* buff, char* path, int sz);
char* filterString (char* in, int sz);
void stringMult (char multend, int n, char* out);
int strToInt (char* in);
int processFlag (int off, int argOff, int argc, char** argv);

#endif
