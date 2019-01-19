/*
 * ceylock/ceylock.h
 *
 * Protocol notes:
 *  server waits on a filesystem socket
 *  client connects and sends a request in a single packet
 *  for simple commands, the server sends single packet response
 *    and closes the connection
 *  for the dump command, the server sends multiple packets/lines
 *    sends a final blank line (packet containing "\n")
 *    and closes the connection
 *  
 *  packets are newline ("\n") terminated, 
 *   and consist or space (0x20) separated fields
 *
 *  maximum packet size is 4096 bytes
 *  
 *  request include up to three fields: the command, the key, and the value
 *    command is one of: "get [key]", "set [key] [value]", "del [key]", "dump"
 *  server (simple) responses include two fields: the status indicator, and the value
 *    status indicator is one of: "OK ([value])", "NO ([value])", "BAD"
 */

void dies(char *s); /* fatal error on syscall: stderr(s+errno), abort */
void die(const char *fmt, ...); /* other fatal: stderr(s), abort */
char *xstrdup(char *);
void *xalloc(unsigned);
char *mkpathjoin(char *p1, char *p2);
char *usocket_getname(unsigned char do_lock);
int usocket_listen(char *sockname);
int usocket_connect(char *sockname);
