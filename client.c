/*
 * ceylock/client.c
 */

#define _DEFAULT_SOURCE  /* for getpass() */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ceylock.h"


#define SZ 4096

#define EXE "keylock"
void usage() {
    fprintf(stderr, "Usage: " EXE " get KEY\n");
    fprintf(stderr, "      |" EXE " set KEY [ VAL ]\n");
    fprintf(stderr, "      |" EXE " del KEY\n");
    fprintf(stderr, "      |" EXE " --server\n");
    exit(1);
}


int do_client(char *cmd, char *key, char *val) {
    char *sockname, buf[SZ];
    char *p;
    int sock, len, ret;

    sockname = usocket_getname(0);
    sock = usocket_connect(sockname);

    if ( ! cmd ) usage();

    if ( ! strcmp("get", cmd) )  {
        len = snprintf(buf, SZ, "get %s\n", key);          
    }
    else if ( ! strcmp("set", cmd) ) {
        if ( ! val ) {
	    snprintf(buf, SZ, "Password for %s: ", key); 
            val = getpass(buf);
        }
        len = snprintf(buf, SZ, "set %s %s\n", key, val);
    }
    else if ( ! strcmp("del", cmd) ) {
        len = snprintf(buf, SZ, "del %s\n", key);
    }

    ret = write(sock, buf, len);
    if ( ret < 0 ) dies("writing");
        
    bzero(buf, SZ);

    len = read(sock, buf, sizeof(buf));
    if ( len < 0 ) dies("reading");
    p = strrchr(buf, '\n');
    if (p) *p = '\0';

    if ( strncmp(buf, "OK", 2) == 0 ) {
        if ( strlen(buf) > 2) {
            printf("%s\n", buf+3);
        }
    }
    else if (strncmp(buf, "NO", 2) == 0) {
        return 1;
    }
    else {
        fprintf(stderr, "%s", buf);
        return 1;
    }

    close(sock);
    free(sockname);
    return 0;
}
