#!/usr/bin/env python

# -*- encdoing:utf-8 -*-

import subprocess,shlex
import os,sys,fcntl
import daemon
from pidlockfile import TimeoutPIDLockFile,PIDLockFile
import logging
import signal
try:
	import json
except ImportError:
	import simplejson as json

import time

logger = None

def openLogger():
	global logger
	logger = None
	logger = logging.getLogger('cdmifuse_shell')
	hdlr = logging.FileHandler('/var/log/cdmifuse_shell.log')
	formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
	hdlr.setFormatter(formatter)
	logger.addHandler(hdlr) 
	logger.setLevel(logging.DEBUG)

class FuseDaemon(daemon.DaemonContext):
	mountpoint = ""
	fuseprocess = None
	running = True
	def __init__( self, chroot_directory=None, working_directory='/', umask=0, uid=None, gid=None, prevent_core=True,
			 detach_process=None, files_preserve=None, pidfile=None, stdin=None, stdout=None, stderr=None, signal_map=None ):
		daemon.DaemonContext.__init__(self,chroot_directory,working_directory,umask,uid,gid,prevent_core,detach_process,
				files_preserve,pidfile,stdin,stdout,stderr,signal_map)

	def terminate(self,signal_number, stack_frame):
		self.running = False
		logger.info("stopping service, kill subprocess if available")
		self.stopFuse()

	def startFuse(self, cmdline):
		self.stopFuse()
		cmdline = cmdline + " 1"
		args = shlex.split(cmdline)
		p = subprocess.Popen(args)
		if not p:
			logger.error("failed to start fuse, please check")
			return None
		p.poll()
		if p.returncode:
			#subprocess terminated, something bad happend
			logger.error("fuse started, but terminated immediately")
			return None
		self.fuseprocess = p
		return self.fuseprocess

	def stopFuse(self):
		if self.fuseprocess:
			try:
				#self.fuseprocess.send_signal(signal.SIGTERM)
				os.kill(self.fuseprocess.pid,signal.SIGTERM)
				self.fuseprocess.wait()
			except OSError:
				pass
			self.fuseprocess = None
		if len(self.mountpoint) <= 0 :
			return
		cmd = "fusermount -u -z %s > /dev/null 2>&1" % ( self.mountpoint)
		os.system(cmd)

	def getMountpoint(self, cmdline):
		args = cmdline.split(' ')
		if len(args)<2:
			return
		openLogger()
		confPath = args[-1]
		logger.debug("got configuration path %s", confPath)
		f = None
		try:
			f = open(confPath)
			config = json.load(f)
			fuse_setting = config["fuse_setting"]
			settings = fuse_setting.split(' ')
			if len(settings) > 1:
				self.mountpoint = settings[-1]
			logger.debug("got mountpoint [%s]", self.mountpoint)
			f.close()
			return self.mountpoint
		except OSError, e:
			logger.exception("failed to load configuration file:%s", confPath)
			return

	def run(self, cmdline):
		self.getMountpoint(cmdline)
		while (self.running):
			logger.info("trying to start subprocess with commandline [%s]", cmdline)
			p = self.startFuse(cmdline)
			if not p:
				logger.error("failed to start fuse process, quit ...")
				return
			logger.info("subporess is running, pid[%d]", p.pid)
			while(self.running):
				time.sleep(0.5)
				p.poll()
				if p.returncode is not None:
					logger.info("subprocess [%d] quit with return code [%d]", p.pid, p.returncode)
					break

class LockFailed(object):
	pass

class PoorPIDLockFile(object):
	def __init__(self,path):
		self.path = path
		self.locker = None

	def release(self):
		if self.locker is None:
			return
		try:
			fcntl.lockf(self.locker,fcntl.LOCK_UN)
		except:
			pass
		self.locker.close()

	def acquire(self):
		self.release()
		self.locker = open(self.path,"a+")
		try:
			fcntl.lockf(self.locker,fcntl.LOCK_EX|fcntl.LOCK_NB)
		except IOError, err:
			raise LockFailed
		self.locker.truncate(0)
		self.locker.write(str(os.getpid()))
		self.locker.flush()

	def __enter__(self):
		self.acquire()

	def __exit__(self,exc_type,exc_value,traceback):
		self.release()

if __name__ == "__main__":
	#cmdline = "/home/hongquan.zhang/cdmifuse/linux/cdmifuse.1.0.1.0 /home/hongquan.zhang/cdmifuse/linux/cdmifuse.conf"
	cmds = sys.argv
	del cmds[0]
	cmdline = ' '.join(cmds)
	#print cmdline
	pidfile_path = "/var/run/cdmifuse_shell.pid"
	try:
		fPid = PoorPIDLockFile(pidfile_path)
		fPid.acquire()
	except LockFailed:
		openLogger()
		print("do not start service due to failed to lock pid file %s" % (pidfile_path))
		sys.exit(-2)
	except:
		openLogger()
		print("do not start service due to failed to open pid file %s" % (pidfile_path))
		sys.exit(-1)
	fPid.release()
	fPid = None
	try:
		f = PoorPIDLockFile(pidfile_path)
		d = None
		try:
			d = FuseDaemon(pidfile=f)
			d.open()
			d.run( cmdline )
		except:
			if( d ):
				d.close()
			logger.exception("caught exception while run daemon")
	except LockFailed:
		s = "can't lock pid file, maybe there is already a running process"
		logger.exception(s)
		print(s)
		sys.exit(-1)
	except:
		logger.exception("something wrong")
		print("failed to start daemon")
		raise
		sys.exit(-1)
