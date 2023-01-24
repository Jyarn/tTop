#ifndef __SESSFETCH__
#define __SESSFETCH__

// linked list of session info structures
typedef struct s_sessInfo {
    unsigned int    pid;
    unsigned int    type; // as defined by ut_type in utmp.h
    char*           user;
    char*           procName;
    struct s_sessInfo*       next;
} sessInfo;

sessInfo* fetchSess ();

#endif