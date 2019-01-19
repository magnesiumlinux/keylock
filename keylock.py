#
# keylock.py
# store secrets in memory
#
# define a simple client/server protocol
# communicating over a pair of pipes
# using short text commands.
# 
# works for python 2 and 3

import fcntl, os, socket, stat

PIPE_BUF=4096
RUNDIR_ENV="KEYLOCK_RUNDIR"
TEMP_MARKER='.'


class KeylockConfigError(Exception):
    pass

class KeylockCommandError(Exception):
    pass


def rundirname():
    """Get the runtime directory name
    """
    dir = os.environ.get(RUNDIR_ENV)
    if not dir:
        parts = [os.environ['HOME'], '.keylock']
        dir = os.path.join(*parts)
    return dir

def createdir(lock=False):
    """ensure the runtime directory exists with the correct permissions,    
       if lock is True, place an advisory lock on it.
    """
    dir = rundirname()
    if not os.path.exists(dir):
        os.mkdir(dir, 700)
    st = os.stat(dir)
    if not stat.S_ISDIR(st.st_mode):
        raise KeylockConfigError("'%s' is not a directory." % dir)
    if st.st_mode&(stat.S_IRWXO|stat.S_IRWXG):
        raise KeylockConfigError("Bad permissions on '%s'." % dir)
    if lock:
        dfd = os.open(dir, os.O_RDONLY)
        fcntl.flock(dfd, fcntl.LOCK_EX|fcntl.LOCK_NB)

def waitsocket():
    """  open a listening (server) socket
    """
    dir = rundirname()
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sockname = os.path.join(dir, 'socket')
    if os.path.exists(sockname):
        os.unlink(sockname)
    sock.bind(os.path.join(dir, 'socket'))
    sock.listen(5)
    return sock

def connsocket():
    """ open a connecting (client) socket
    """
    dir = rundirname()
    path = os.path.join(dir, 'socket')
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(path)
    except socket.error:
        raise ValueError("No socket at: '%s'" % path)
    return sock


def server():
    storage = {}
    try:
        createdir()
        wsock = waitsocket()
    except KeylockConfigError as e:
        print (str(e))
        return

    print ("ready")
    while True:
        (sock, addr) = wsock.accept()
        buf = sock.recv(PIPE_BUF)

        try:
            parts = buf.split(' ', 1)
            cmd = parts[0]
            if len(parts) > 1: 
                args = parts[1]

            if cmd == 'set':
                key, val = args.split(' ', 1)
                #print('set %s' % key)
                storage[key] = val
                sock.send('OK')

            elif cmd == 'get':
                key = args.strip()
                #print ('get %s' % key)
                if key in storage:
                    sock.send('OK %s'%storage[key])
                else:
                    sock.send('NO')

            elif cmd == 'dump':
                print ("dumping %s keys" % len(storage))
                ret = 'OK '
                for k,v in storage.items():
                    if k.startswith(TEMP_MARKER): continue
                    ret += 'set %s %s\n' % (k,v)
                sock.send(ret)

            else:
                raise ValueError(cmd)
        
        except (ValueError, KeylockCommandError):
            print ("Bad request: '%s'" % buf)
            sock.send('BAD')

        finally:
            sock.close()


def client(req):
    """Handle the client side of the protocol
    Returns the response string,
    or raises an error
    """
    import sys
    sock = connsocket()
	
    sock.send(req.encode("UTF8"))
    resp = sock.recv(PIPE_BUF).decode()
    sock.close()
    

    if resp.startswith('OK '):
        return resp[3:].rstrip('\n')
    elif resp.startswith('BAD'):
        raise KeylockCommandError("Bad request: %s" % req)
    else:
        return None

