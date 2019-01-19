/**
 ** ceylock/mote.c
 ** simple associative storage
 ** memory: we allocate key storage
 **         caller allocates value storage
 **/ 
#include <stdio.h>  /* go away */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ceylock.h"
#include "mote.h"

#define MOTEMAX 128

typedef struct {
    char *k;
    void *v;
} mote_t;


mote_t *MoteStorage[MOTEMAX];
int MoteReady=0;
int MoteNextIndex=0;
int MoteSize=0;

void mote_init() {
    MoteReady = 1;
    MoteNextIndex=0;
    MoteSize=0;
}

void mote_fini() {
    MoteReady = 0;
}

int _mote_idx(char *k) {
	/* return the index of the mote with key k, or -1 */
	int i;
	for (i=0; i<MoteSize; i++) {
		if ( MoteStorage[i] && !strcmp(MoteStorage[i]->k, k) ) {
			return i;
   		}
	}
	return -1;
}

void *mote_get (char *k) {
    /* return the value associated with key k, or NULL */
    assert(MoteReady);

	int i = _mote_idx(k);
	if (i>=0) return MoteStorage[i]->v;
	else return NULL;
}

void mote_del(char *k) {
    /* remove the mote with key k */
    assert(MoteReady);

	int i = _mote_idx(k);
	if (i<0) return;

	free(MoteStorage[i]->k);
	free(MoteStorage[i]);
	MoteStorage[i] = NULL;
}

void * mote_set(char *k, void *v) {
    /* add a new mote with key k, value v 
     * return old value or NULL 
     */
    assert(MoteReady);

	mote_t *new;
    void *old_v = NULL;
	int i = _mote_idx(k);

	if (i>=0) {
        old_v = MoteStorage[i]->v;
		MoteStorage[i]->v = v;
	} else {
		new = (mote_t *) xalloc(sizeof(mote_t));
		new->k = (char *) xstrdup(k);
		new->v = v;
		MoteStorage[MoteSize++] = new;
	}
    return old_v;
}

char *mote_next() {
    /* iterate through all stored motes
     * return the next mote key, or NULL.
     */
    assert(MoteReady);
    char *k = NULL;
	while ( !k && MoteNextIndex < MoteSize ) {
		if ( MoteStorage[MoteNextIndex] ) {
            k = MoteStorage[MoteNextIndex]->k;
        }
        MoteNextIndex++;
	}
    return k;
}

void mote_rewind() {
	MoteNextIndex=0;
}

int mote_len() {
	int i;
	int c=0;
	for (i=0; i<MoteSize; i++) if (MoteStorage[i]) c++;
	return c;
}

void mote_test_init(){
    mote_init();
	assert( mote_len() == 0 );
    mote_set("a", "A");
	assert( mote_len() == 1 );
	mote_set("b", "B");
	assert( !strcmp(mote_get("a"), "A") );
    mote_del("a");
	assert( mote_get("a") == NULL );
	assert( mote_len() == 1 );
    assert( !strcmp("b", mote_next()) );
    mote_del("b");

}

