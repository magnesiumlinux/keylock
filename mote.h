/*
 * ceylock/mote.h
 * the simplest associative lookup
 * that might possibly work
 */

void mote_init();
void mote_fini();

void *mote_get(char *);
void *mote_set(char *, void *);
void mote_del(char *);

char *mote_next();
void mote_rewind();
int mote_len();

void mote_test_init();