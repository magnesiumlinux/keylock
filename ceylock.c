/*
 * ceylock/ceylock.c
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "ceylock.h"

#define PATHSEP "/"
#define RUNDIR_ENV "KEYLOCK_RUNDIR"
#define RUNDIR_DEFAULT ".keylock"


void dies(char *s) {
    /* fatal failed system call */
    fprintf(stderr, "%s: %s\n", s, strerror(errno));
    abort();
}

void die(const char *fmt, ...) {
    /* fatal, not a system call */
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    abort();
}

void *xalloc (unsigned sz) {
    /* alloc or die */
    void *p = malloc(sz);
    if (!p) dies("Memory Error.");
    return p;
}

char *xstrdup(char *s) {
    /* strdup or die */
    char *c = strdup(s);
    if (!c) dies("Memory Error.");
    return c;
}

char *mkpathjoin(char *p1, char *p2) {
    /* concatenate paths p1 and p2, joined with the path separator
     * return the newly allocated string, or die
     */
    int len = strlen(p1) + strlen(PATHSEP) + strlen(p2) + 1;
    char *pj = (char *) xalloc(len);

    strcpy(pj, p1);
    strcat(pj, PATHSEP);
    strcat(pj, p2);

    return pj;
}

char *mkrundirname() {
    /* return the full path to the rundir 
     * in a newly allocated string, or die
     */
    char *rundir, *home;

    rundir = getenv(RUNDIR_ENV);
    if (rundir) return strdup(rundir);
		
    home = getenv("HOME");
    if ( !home ) dies("Homeless.");

    return mkpathjoin(home, RUNDIR_DEFAULT);
}


void getdir(char *dirname, unsigned char do_lock) {
    /* ensure that the service directory exists
     * and has secure permissions, or die
     */
    int fd;
    struct stat st_buf;

    fd = open(dirname, O_RDONLY);
    if ( fd < 0 && errno == ENOENT ) {
        /* dir doesn't exist.  create it */
        if ( mkdir(dirname, 0700) < 0) dies("mkdir");
        return;
    }

    if ( fstat(fd, &st_buf) < 0 ) dies("stat");
    if ( ! S_ISDIR(st_buf.st_mode) ) die("not a dir: %s", dirname);
    if ( st_buf.st_mode&(S_IRWXO|S_IRWXG) ) die("loose permissions: %s", dirname);	

    if ( do_lock ) {
        if ( flock(fd, (LOCK_EX|LOCK_NB)) < 0 ) die ("flock");
    }
    return;
}


char *usocket_getname(unsigned char do_lock) {
    char *rundirname, *sockname;

    rundirname = mkrundirname();
    getdir(rundirname, do_lock);
    sockname = mkpathjoin(rundirname, "socket");

    free(rundirname);
    return sockname;
}


int usocket_listen (char *sockname) {
    /* return a socket listening at sockname 
     */
    int sock;
    struct sockaddr_un sa;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( sock < 0 ) dies("socket");

    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, sockname, sizeof(sa.sun_path)-1);
    if ( bind(sock, (struct sockaddr *) &sa, 
         sizeof(struct sockaddr_un))  < 0 ) dies("bind");

    if ( listen(sock, 5) < 0 ) dies("listen");
    return sock;
}

int usocket_connect(char *sockname) {
    /* return a socket connected to sockname 
     */
    int sock;
    struct sockaddr_un sa;
    
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( sock < 0 ) dies("socket");

    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, sockname, sizeof(sa.sun_path)-1);
    if ( connect(sock, (struct sockaddr *) &sa, 
         sizeof(struct sockaddr_un)) < 0 ) dies("connect");

    return sock;
}
