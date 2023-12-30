#!/usr/bin/python
# Filename: updateFile.py

import os
import time
import sys

sourceDir = "./"
if(len(sys.argv) >= 2):
    sourceDir= sys.argv[1]
    if(sourceDir[-1] != '/'):
        sourceDir+='/'
if not os.path.exists(sourceDir):
    print  'source directory [%s]not exist' %sourceDir
    exit(0)
    
print 'source directory [%s]' % sourceDir

# 1. The files and directories to be backed up are specified in a list.
source = {    'shell.py':           '/usr/lib/python2.6/site-packages/cinderclient/v1/shell.py',
              'volumes.py':         '/usr/lib/python2.6/site-packages/cinderclient/v1/volumes.py',
              'api.py':             '/usr/lib/python2.6/site-packages/cinder/volume/api.py',
              'rpcapi.py':          '/usr/lib/python2.6/site-packages/cinder/volume/rpcapi.py',
              'volume_actions.py':  '/usr/lib/python2.6/site-packages/cinder/api/contrib/volume_actions.py',
              'manager.py':         '/usr/lib/python2.6/site-packages/cinder/volume/manager.py',
              'driver.py':          '/usr/lib/python2.6/site-packages/cinder/volume/driver.py',
              'fuse.py':	    '/usr/lib/python2.6/site-packages/cinder/volume/drivers/fuse.py'}

today = time.strftime('%Y%m%d%H%M%S')
# The current time is the name of the zip archive

for sourcefile, targetfile in source.items():    
    sourceFilePath = sourceDir + sourcefile
    targetFilePath =  targetfile

    if not os.path.exists(sourceFilePath):
        print  'source file [%s]not exist' % sourceFilePath
        continue

    bak_command='cp %s %s%s_bak' %(targetFilePath, targetFilePath, today)
    #print bak_command
    if os.system(bak_command) == 0:
         print 'Successful backup %s to %s%s_bak' % (targetFilePath , targetFilePath, today)
    else:
         print 'backup %s to %s%s_bak FAILED' % (targetFilePath , targetFilePath, today)
         continue

    cp_command='cp %s %s' %(sourceFilePath, targetFilePath)
    #print cp_command
    # Run the copy command
    if os.system(cp_command) == 0:
        print 'Successful copy %s to %s' % (sourceFilePath , targetFilePath)
    else:
        print 'copy %s to %s FAILED' % (sourceFilePath , targetFilePath)
