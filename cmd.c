/*
 * ceylock/client.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ceylock.h"

#define EXE "keylock"
#define SZ 4096

int do_server();
int do_client(char *, char *, char *);

int main(int argc, char *argv[]) {        
    char *cmd = NULL, *key = NULL, *val = NULL;

    if ( argc > 1 ) cmd = argv[1];
    if ( argc > 2 ) key = argv[2];
    if ( argc > 3 ) val = argv[3];

    if ( cmd && ! strcmp(cmd, "--server") ) {
        return do_server();
    }
    else {
        return do_client(cmd, key, val);
    }
}
