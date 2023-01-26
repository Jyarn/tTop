#ifndef __MISC__
#define __MISC__

void colExtract (unsigned int* ret, int sz, char* bff);

char* readFile (char* path);
int buffFRead (char* buff, char* path, int sz);
char* filterString (char* in, int sz);

#endif