#!/usr/bin/env python
import struct
import fcntl
import os
import sys

def cacheSetting( enable ):
    filename = "usdhflisadfjo84efueo8ut7w4ojiro84rfhreli"
    cache_cmd = 1073833267
    para = struct.pack("@b",1 if enable else 0)
    with open(filename,"w+") as f:
        fcntl.ioctl(f.fileno(), cache_cmd, para )
    os.unlink( filename )

if __name__=="__main__":
    enable = False
    if len(sys.argv) > 1:
        if (sys.argv[1].lower() == "true"):
            enable = True
        else:
            enable = False
    print("%s cache" %( "enable" if enable else "disable") )
    cacheSetting( enable )
