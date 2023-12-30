#!/usr/bin/env python

import subprocess
import os
import shlex
import sys

#   0     0 DNAT       tcp  --  *      *       0.0.0.0/0            172.16.20.40        tcp dpt:9527 to:10.2.232.68:22

CMD="/sbin/iptables -t nat -L -v -n"

def findruleindex( minport=9500,maxport=9999):
	args = shlex.split(CMD)
	iptableinfo = subprocess.Popen( args, stderr=subprocess.STDOUT, stdout = subprocess.PIPE )
	(out,err) = iptableinfo.communicate()
	lines =  out.split('\n')
	count = 0
	for line in lines:
		count = count + 1
		if count <= 2: 
			continue
		if line.find( "Chain POSTROUTING" ) >= 0 :
			break
		if line.find("DNAT") < 0:
			continue
		words = line.split(' ')
		wordcount = len(words)
		if wordcount < 5 :
			continue
		newwords = []
		for word in words:
			word.strip()
			if len(word) > 0:
				newwords.append(word)
		for i in range(0,wordcount-2):
			if words[i] == "tcp" and words[i+1].find("dpt:") >= 0:
				tmp = words[i+1].split(':')
				if len(tmp) != 2:
					continue
				try:
					port = int(tmp[1])
					if port>=minport and port <= maxport:
						return count - 2
				except:
					pass
	return -1

def cleanup(minport,maxport):
	if minport > maxport:
		minport,maxport=maxport,minport
	while True:
		index = findruleindex(minport,maxport)
		if ( index < 0 ):
			break
		cmd ="/sbin/iptables -t nat -D PREROUTING %d" %(index)
		print cmd
		os.system(cmd)
		
if __name__ == "__main__":
	cmdstr = "echo 1 > /proc/sys/net/ipv4/ip_forward"
	os.system(cmdstr)
	minport= 9500
	maxport= 9999
	if len(sys.argv) > 1:
		minport = int(sys.argv[1])
	if len(sys.argv) > 2:
		maxport = int(sys.argv[2])
	cleanup(minport,maxport)
	
