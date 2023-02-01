#ifndef __MISC__
#define __MISC__

#define ABS(a) (a < 0 ? -a : a)

void colExtract (unsigned int* ret, int sz, char* bff);

char* readFile (char* path);
int buffFRead (char* buff, char* path, int sz);
char* filterString (char* in, int sz);
void stringMult (char multend, int n, char* out);

#endif
