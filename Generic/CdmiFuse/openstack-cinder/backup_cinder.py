#!/usr/bin/python
# Filename: backup_cinder.py

import os
import time

# 1. The files and directories to be backed up are specified in a list.
source = ['/usr/lib/python2.6/site-packages/cinderclient/v1/shell.py',
          '/usr/lib/python2.6/site-packages/cinderclient/v1/volumes.py',
          '/usr/lib/python2.6/site-packages/cinder/volume/api.py',
          '/usr/lib/python2.6/site-packages/cinder/volume/rpcapi.py',
          '/usr/lib/python2.6/site-packages/cinder/api/contrib/volume_actions.py',
          '/usr/lib/python2.6/site-packages/cinder/volume/manager.py',
          '/usr/lib/python2.6/site-packages/cinder/volume/driver.py']

# 2. The backup must be stored in a main backup directory
target_dir = '/home/cinder/' # Remember to change this to what you will be using

if not os.path.exists(target_dir):
    os.mkdir(target_dir) # make directory
    print 'Successfully created directory', target_dir
    
# 3. The files are backed up into a zip file.
# 4. The current day is the name of the subdirectory in the main directory
today = target_dir + time.strftime('%Y%m%d%H%M%S')

# Create the subdirectory if it isn't already there
if not os.path.exists(today):
    os.mkdir(today) # make directory
    print 'Successfully created directory', today

for item in source:
    sourceFile = item.split("/")
    target =  today + os.sep + sourceFile[-1]
    cp_command='cp %s %s' %(item, target)
    # print cp_command
    # Run the copy command
    if os.system(cp_command) == 0:
        print 'Successful copy %s to %s' % (item , target)
    else:
        print 'copy %s to %s FAILED' % (item , target)
