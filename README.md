### keylock

a client-server protocol and tools for key-value lookup

# st

# wire protocol

request
=======

command SPACE key [ SPACE val ] LF

keys and values can contain any bytes other than SPACE 
and LF.  

commands can be up to 4096 bytes.

commands
--------
   set
   get
   del


response
========
 
BAD | OK | NO  [ PAYLOAD ] 

OK response indicates a valid request was successful.
NO response indicates a valid request was not successful
BAD responses indicate an invalid request or server failure

OK responses to get requests will have the requested data in the payload
BAD responses may have the reason for the failure in the payload
