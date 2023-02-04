#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "misc.h"

void colExtract (unsigned int* ret, int sz, char* bff) {
    // extract integers in /proc files
    // used in memPoll and cpuPoll
    char* lp = NULL;

    for (int i = 0; i < sz; i++) {
        ret[i] = strtol(&bff[0], &lp, 10);
        bff = lp;
        bff = strchr(bff, ' ');
    }
}


int buffFRead (char* buff, char* path, int sz) {
    // read a file into a buffer
    // return how much of path has been read into buff

    int fd = open(path, O_RDONLY);
    if (fd < 0) { return -1; }
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

char* filterString (char* in, int sz) {
    /*
        filter all non-numerical characters in (char* in)
        so that it can be processed by colExtract.
    */

    char* ret = malloc(sz);
    int j = 0;
    bool justAdded = false;

    for (int i = 0; i < sz; i++) {
        if (in[i] >= '0' && in[i] <= '9') {
            ret[j] = in[i];
            justAdded = true;
            j++;
        }
        else if (justAdded) {
            // space each integer out with a column
            justAdded = false;
            ret[j] = ' ';
            j++;
        }
    }

    ret[ (j < sz ? j : sz-1) ] = '\0'; // null terminate index max(j, sz-1)
    return ret;
}

void stringMult (char multend, int n, char* out) {
	// equivalent to python's string multiplication thing
	// ex. 'a' * 5 == aaaaa
	// output written into out, never checks if writing is inbounds
	// also null terminates

	int i;
	for (i = 0; i < n; i++) {
		out[i] = multend;
	}

	out[i] = '\0';
}

void attachToHead (linkedList* attachend, linkedList* attachee) {
    attachend->next = attachee->next;
    attachee->next = attachend;
}

linkedList* destroy_lList (linkedList* head) {
    free(head->data);
    destroy_lList(head->next);
    free(head);
    return NULL;
}