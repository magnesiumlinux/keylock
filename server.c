/*
 * ceylock/server.c
 * C implementation of the keylock server
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ceylock.h"
#include "mote.h"

#define SZ 4096
#define TEMP_MARKER '.'

#ifdef _GNU_SOURCE
#define getenv secure_getenv
#endif

int waitsocket() {
    int sock;
    char *sockname;

    sockname = usocket_getname(1);

    /* create and open the socket for listening */
    if ( unlink(sockname) < 0 && errno != ENOENT ) die("unlink");

    sock = usocket_listen(sockname);

    if ( chmod(sockname, 0600) < 0 ) dies ("chmod");

    free(sockname);
    return sock;
}


void serve(int sock) {
    int len;
    char *cmd = NULL, *key = NULL, *val = NULL, *p;
    char inbuf[SZ], outbuf[SZ];
    void *old_val;

    bzero(inbuf, SZ);
    bzero(outbuf, SZ);

    len = read(sock, inbuf, SZ);
    if ( len < 0 || len >=SZ ) {
        len = snprintf(outbuf, SZ, "BAD overflow");
        goto send;
    }
    p = strrchr(inbuf, '\n');
    if (p) *p = '\0';
    
    cmd = strtok(inbuf, " ");
    key = strtok(NULL, " ");
    val = strtok(NULL, " ");

    if ( ! cmd || ! key ) {
       len = snprintf(outbuf, SZ, "BAD");
    }
    else if ( !strcmp(cmd, "set") ) {
        if ( ! val ){
            len = snprintf(outbuf, SZ, "BAD");
        } else {
            fprintf(stderr, "set: %s: %s\n",  key, "<hidden>");
            old_val = mote_set(key, xstrdup(val));
            if ( old_val ) free(old_val);
            len = snprintf(outbuf, SZ, "OK");
        }
    }
    else if ( !strcmp(cmd, "get") ) {
        fprintf(stderr, "get: %s\n", key);
        val = (char *) mote_get(key);
        if ( val ) {
            len = snprintf(outbuf, SZ, "OK %s", val); 
        } else {
            len = snprintf(outbuf, SZ, "NO");
        }
    }

    else if ( !strcmp(cmd, "del") ) {
        fprintf(stderr, "del: %s\n", key);
        val = (char *) mote_get(key);
        if ( val ) {
            free(val);
            mote_del(key);
            len = snprintf(outbuf, SZ, "OK %s", key);

            len = snprintf(outbuf, SZ, "NO");
        }     
    }
    else {
        fprintf(stderr, "Bad command: %s", cmd);
        strcpy(outbuf, "BAD");
    }

    if ( len >= SZ-2 ) {
        snprintf(outbuf, SZ, "BAD overflow");
    }

    send:

    strcat(outbuf, "\n");
    write(sock, outbuf, strlen(outbuf));
    close(sock);
}


void write_on_quit(int signum) {
    fprintf(stderr, "Exit on signal %d\n", signum);
    exit(0);
}


int do_server() {
    int master_sock, connsock;
    struct sigaction siggy;

    mote_test_init();

    /* install signal handler */
    siggy.sa_handler = write_on_quit;
    sigfillset(&siggy.sa_mask);
    sigaction(SIGTERM, &siggy, NULL);

   /* read data from stdin */

    master_sock = waitsocket();
    fprintf(stderr, "ready.\n");

    for (;;) {
        connsock = accept(master_sock, NULL, NULL);
        serve(connsock);
    }
    return 0;
}
