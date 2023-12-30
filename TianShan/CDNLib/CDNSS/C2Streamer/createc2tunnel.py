#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys
import shlex
import subprocess

# NAT       tcp  --  *      eth3    0.0.0.0/0            0.0.0.0/0           tcp to:10.2.232.63

def findsnat( ethname, sourceip ):
	CMD="/sbin/iptables -t nat -L -v -n"
	args = shlex.split(CMD)
	iptableinfo = subprocess.Popen( args, stderr=subprocess.STDOUT, stdout = subprocess.PIPE )
	(out,err) = iptableinfo.communicate()
	lines =  out.split('\n')
	count = 0
	begin = False
	for line in lines:
		count = count + 1
		if count <= 2: 
			continue
		if (not begin) and  (line.find( "Chain POSTROUTING" ) < 0) :
			continue
		else:
			begin = True

		if line.find("SNAT") < 0:
			continue
		if line.find(ethname) < 0:
			continue
		tosrc="to:"+sourceip
		if line.find(tosrc) < 0:
			continue
		return True
	return False

def createrule(ethname,sourceip,localip,localport,peerip,peerport):
	if not findsnat(ethname, sourceip):
		CMD = "/sbin/iptables -t nat -A POSTROUTING -p tcp -m tcp -o %s -j SNAT --to-source %s" % (ethname, sourceip)
		os.system(CMD) #should I check the return value of this command
	CMD="/sbin/iptables -t nat -A PREROUTING -p tcp -m tcp -d %s --dport %s -j DNAT --to-destination %s:%s" % ( localip,localport, peerip, peerport )
	return os.WEXITSTATUS(os.system(CMD))

import sys
if __name__=="__main__":
	if len(sys.argv)<7:
		print("usage:%s out-eth-name eth-ip localServeIp localServePort peerIp peerPort" % (sys.argv[0]) )
		sys.exit(-1)
	arg = sys.argv
	ret = createrule( arg[1], arg[2], arg[3], arg[4], arg[5], arg[6] )
	sys.exit(ret)

