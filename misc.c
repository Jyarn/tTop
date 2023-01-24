#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void colExtract (unsigned int* ret, int sz, char* bff) {
    char* lp = NULL;

    for (int i = 0; i < 10; i++) {
        ret[i] = strtol(&bff[1], &lp, 10);
        bff = lp;
        bff = strchr(bff, ' ');
    }
}


int buffFRead (char* buff, char* path, int sz) {
    // read a file into a buffer
    int fd = open(path, O_RDONLY);
    int ret = read(fd, buff, sz);
    close(fd);
    return ret; // note if read fails it will return -1
}

char* readFile (char* path) {
    struct stat fInfo;
    stat(path, &fInfo);

    // allocate enough space for the file + a null terminator
    char* ret = malloc(fInfo.st_size + sizeof(char));

    int code = buffFRead(ret, path, fInfo.st_size);
    ret[fInfo.st_size] = '\0';

    if (code < 0) {
        // file read failed
        return NULL;
    }

    return ret;
}
