typedef struct s_CPUstats {
    unsigned int pActive;
    unsigned int  pTotal;
    unsigned int active;
    unsigned int  total;
} CPUstats;

void getCPUstats (int fd, char* bff, CPUstats* prev) {
    /*
        fd = file descriptor for /proc/stat
        bff = buffer reading in files
        prev = pointer to a CPUstat to be updated with new delta
    */

    unsigned int stats[10];
    read(fd, bff, BUFFSZ);

    char* lp = NULL;

    for (int i = 0; i < 10; i++) {
        bff = strchr(bff, ' ');
        stats[i] = strtol(&bff[1], &lp, 10);
        bff = lp;
    }

    prev->pActive = prev->active;
    prev->pTotal = prev->total;

    prev->active = stats[0]+stats[1]+stats[2]+stats[5]+stats[6];
    prev->total = prev->active + stats[3]+stats[4];

    lseek(fd, 0, SEEK_SET);
}

double calculateCPUusage(CPUstats stats) {
    double crrActive = (double)(stats.active - stats.pActive);
    double crrTotal  = (double)(stats.total - stats.pTotal);
    return (crrActive / crrTotal) * 100;
}