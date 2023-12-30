#!/usr/bin/env python
import struct
import fcntl

# the command code converted from the .h definitions 
clone_cmd = 1107584257     
query_cmd = -1072143102
qos_cmd = 1074291971

# function to copy a file, or so called full clone with client offloaded
# @param srcFilename - the filename of the file that will be cloned from
# @param destFilename - the filename that will be clone to
def full_clone( srcFilename, destFilename ):
	clone_params = struct.pack("@HH512s", 0, 0, destFilename)  # build up the param struct
	f = open(srcFilename, "r")
	if not f:
		raise Exception("can't open specified file")
	rc = fcntl.ioctl(f.fileno(), clone_cmd, clone_params)
	f.close()
	return rc

# function to fast clone a file
# @param srcFilename  - the filename of the file that will be cloned from
# @param destFilename - the filename that will be clone to
def fast_clone( srcFilename, destFilename ):
	clone_params = struct.pack("@HH512s", 0, 1, destFilename)  # build up the param struct
	f = open(srcFilename, "r")
	if not f:
		raise Exception("can't open specified file")
	rc = fcntl.ioctl(f.fileno(), clone_cmd, clone_params)
	f.close()
	return rc

def status_2_str(status):
	if status == 0 :
		return "complete"
	elif status == 1:
		return "processing"
	elif status == 2:
		return "error"
	else:
		return "unknown"

# function to query the status of a clone/copy procedure
# @param fName - the name of the destination file
def query_status( fName ):
	query_para = struct.pack("@HHQB",0,0,0,0)
	with open(fName,"r") as f:
		rc = fcntl.ioctl(f.fileno(), query_cmd, query_para )
		(_,status,_,percent) = struct.unpack("@HHQB",rc)
		print "%s --> %s %d%%" % (fName,status_2_str(status),percent)
	return status

def qos_throughput( fName ):
	qos_para = struct.pack("@HI", 0, 1) # to limit transfer at 1KBps
	f = open (fName, "r")
	if not f:
		raise Exception ("can't open specifid file")
	rc = fcntl.ioctl(f.fileno(), qos_cmd, qos_para)
	f.close()
	return rc

# the test entry
if __name__ == "__main__":
	import sys
	import time
	oldfile ="1"
	newfile ="1_to_12"
	if( len(sys.argv) >2 ):
		oldfile = sys.argv[1]
		newfile = sys.argv[2]

#	fast_clone(oldfile, newfile)	# or
	full_clone(oldfile, newfile)
	
	cstat = 1
	while (cstat ==1) :
		time.sleep(2)
		cstat = query_status(newfile)
