#!/usr/bin/env python
#-*- encoding:utf-8 -*-

#
#
import errno, os, struct, sys, time, threading,datetime,re
from os.path import stat
import fuse,ftplib
import traceback
import urlparse

fuse.fuse_python_api = (0,2)

codec = "utf-8"

def decode_(str_):
    """
    """
    text = str_
    charests = ('utf8', 'gbk', 'gb2312', 'big5', 'ascii',
                'shift_jis', 'euc_jp', 'euc_kr', 'iso2022_kr',
                'latin1', 'latin2', 'latin9', 'latin10', 'koi8_r',
                'cyrillic', 'utf16', 'utf32'
                 )
    if isinstance(text, unicode):
        return text
    else:
        for i in charests:
            try:
                return text.decode(i)
                break
            except:
                pass
        else:
            return None

class FtpStat(fuse.Stat):
	def __init__(self):
		self.st_mode = stat.S_IFDIR | 0744
		self.st_ino = 0
		self.st_dev = 0
		self.st_nlink = 2
		self.st_uid = os.getuid()
		self.st_gid = os.getgid()
		self.st_size = 4096
		self.st_atime = time.time()
		self.st_mtime = self.st_atime
		self.st_ctime = self.st_atime
		self.st_blocks = 1
		self.st_blksize = 4096
		self.st_rdev = 500
		self.st_flags = 0
	def __repr__(self):
		return "mode<%d> nlink<%d> size<%d> time<%s> block<%d>" % ( self.st_mode, self.st_nlink, self.st_size, self.st_mtime, self.st_blocks)


class FileDetail:
	def __init__(self, detail):
		self.detail = detail
		self.perm = 0
		self.size = 4096
		self.isFolder = False
		self.isLink = False
		self.mtime = time.time()
		self.hardlink = 2
		self.user = ""
		self.group = ""
		self.name = ""
	def __repr__(self):
		return "name<%s> foler<%s> user<%s> mtime<%s> perm<%d>" % (self.name, self.isFolder and "TRUE" or "FALSE", self.user, time.strftime('%Y %m %d %H:%M',self.mtime),self.perm)

	def permStringToNumber(self,s):
		if(len(s) != 3):
			raise Exception("permission string should be 3 characters")
		r = 0
		for n in s:
			if n == 'r':
				r = r | 0x04
			elif n == 'w':
				r = r | 0x02
			elif n =='x':
				r = r | 0x01
			elif n == '-':
				pass
			else:
				raise Exception("permission string should only have 'rwx-'")
		return r

	def getPerm(self, perm):
		userPerm = self.permStringToNumber(perm[1:4]) << 6
		groupPerm = self.permStringToNumber(perm[4:7]) << 3
		otherPerm = self.permStringToNumber(perm[7:10])
		return userPerm | groupPerm | otherPerm

	def getmtime(self,sects):
		if(len(sects) != 3):
			raise Exception("time should consist of 3 strings")
		timestr = ' '.join(sects) + " " + str(time.localtime().tm_year)
		try:
			return time.mktime(time.strptime(timestr,'%b %d %H:%M %Y'))
		except:
			timestr = ' '.join(sects)
			return time.mktime(time.strptime(timestr,"%b %d %Y"))
	
	def parse(self):
		#drwxrwxr-x 9 roy roy 4096 Nov  9 21:06 hongquanproject
		sects = self.detail.split(None,8)
		if( sects < 9 ):
			return False
		t = sects
		sects = []
		for sect in t:
			s = sect.strip()
			if ( len(s) <= 0 ): continue
			sects.append(s)

		if( len(sects) < 9):
			return False

		perm = sects[0].strip()
		if(len(perm) != 10 ):
			return False
		
		self.isFolder = perm[0] == 'd'
		self.isLink = perm[0] == 'l'

		try:
			self.perm = self.getPerm(perm)
		except:
			return False
		try:
			self.mtime = self.getmtime(sects[5:8])
		except:
			print "***************"
			print sects[5:8]
			traceback.print_stack()
			return False
		self.hardlink = int(sects[1])
		self.user = sects[2]
		self.group = sects[3]
		self.size = int(sects[4])
		self.name = decode_(sects[8].strip()).encode('utf8')
		return True

class FtpSession:
	connection = None
	ftpop = None
	def __init__(self,conn,ftpChl):
		self.connection = conn
		self.ftpop = ftpChl

	def release(self):
		if not self.connection:
			return
		self.connection.close()
		self.ftpop.voidresp()
		self.connection = None

	def read(self,maxbytes = 4*1024):
		if( not self.connection ):
			return 0
		buf=""
		def recvfile():
			while True:
				r = self.connection.recv(maxbytes)
				if len(r) <= 0: return buf
				buf = buf + r
				maxbytes = maxbytes - len(r)
				if(maxbytes <= 0 ):
					break
			return buf
		#return recvfile()
		return self.connection.recv(maxbytes)

	def write(self,data):
		if not self.connection:
			return 0
		return self.connection.sendall(data)


class FtpChannel(ftplib.FTP):
	def __init__(self):
		ftplib.FTP.__init__(self)

	def makeFileTransfer(self, cmd, rest):
		self.voidcmd('TYPE I')
		cmd = cmd.decode('utf-8').encode(codec)
		return self.transfercmd( cmd , rest )

	def makeCmdTransfer( self, cmd ):
		self.voidcmd('TYPE A')
		cmd = cmd.decode('utf-8').encode(codec)
		return self.transfercmd( cmd )

	def getfile(self, remotepath, offset = None ):
		cmd = 'RETR ' + remotepath
		rest = None
		conn = self.makeFileTransfer( cmd, offset )
		sess = FtpSession(conn,self)
		return sess

	def parsePermission(self, perm):
		sects = perm.split(' ')
		if( len(sects) != 9):
			return 

	def lsdir(self,path):
		lines = []
		cmd = 'LIST %s' % (path)
		cmd = cmd.decode('utf-8').encode(codec)
		self.retrlines( cmd , lambda x:lines.append(x))
		r = []
		for line in lines:
			if len(line) <= 0: continue
			fd = FileDetail(line)
			if not fd.parse():
				continue
			if fd.isLink:
				continue
			r.append(fd)
		return r

	def statdir(self, path):
		p,c = os.path.split(path)
		lines = []
		cmd = 'LIST %s' %(p)
		cmd = cmd.decode('utf-8').encode(codec)
		self.retrlines( cmd , lambda x: lines.append(x) )
		for line in lines:
			fd = FileDetail(line)
			if not fd.parse():
				continue
			if fd.isLink:
				continue

			if fd.name == c:
				st = FtpStat()
				st.st_size = fd.size
				st.st_atime = st.st_ctime = st.st_mtime = fd.mtime
				st.st_mode = fd.perm | (fd.isFolder and stat.S_IFDIR or stat.S_IFREG)
				st.st_blocks = fd.size/4096 + ( (fd.size%4096) > 0 and 1 or 0 )
				st.st_nlink = fd.hardlink
				return st
		return None

	def putfile(self, remotepath, offset = None):
		cmd = 'STOR ' + remotepath
		rest = None
		conn = self.makeFileTransfer(cmd, offset)
		sess = FtpSession(conn,self)
		return sess

class FtpFS(fuse.Fuse):
	def __init__(self,url,*args,**kw):
		fuse.Fuse.__init__(self,*args,**kw)
		u = urlparse.urlsplit(url)
		self.op = FtpChannel()
		self.op.connect(u.hostname,u.port,100)
		self.op.login(u.username,u.password)

	def getattr(self,path):
		print " getattr >>>>>>", path
		if(path=='/'):
			return FtpStat()
		st = self.op.statdir(path)
		if st != None :
			return st
		else:
			return -errno.ENOENT

	def igetxattr(self,path,name,size):
		val = name.swapcase()+'@'+path
		if size == 0:
			return len(val)
		return val

	
	def access(self,path,perm):
		# I am not sure if 0 is OK
		return 0

	def readlink(self,path):
		print " readlink>>>>>>", path
		return -1

	def mknod(self,path,node,rdev):
		print(" mknod>>>>>>>",path)
		return -1

	def mkdir(self,path,mode):
		print("mkdir>>>>>>>",path)
		self.op.mkd(path)
		return 0

	def readdir(self,path,offset):
		print(" readdir >>>>>>>",path)
		dirs = self.op.lsdir(path)
		direntries = []
		for folder in dirs:
			direntries.append( fuse.Direntry(folder.name))
		return direntries;

	def unlink(self,path):
		print("unlink>>>>>>>>>>>")
		self.op.delete(path)
		return 0

	def symlink(self,target,name):
		print("symlink>>>>>>>>")
		return -1

	def rename(self,old,new):
		print("rename>>>>>>",old,new)
		self.op.rename(old,new)
		return 0

	def link(self,target,name):
		print("link>>>>>>>>>",target,name)
		return -1

	def open(self,path,flags):
		print("open>>>>>>>",path)
		return 0

	def create(self,path,flags,mode):
		print("create>>>>>>",path)
		sess=self.op.putfile(path)
		if(sess):
			sess.write("")
			sess.release()
		else:
			return -1

	def utimens(self,path,tm_acc,tm_mod):
		#do nothing
		return 0

	def read(self,path,length,offset,fh=None):
		print("read>>>>>>>",path)
		sess=self.op.getfile(path,offset)
		print("got file>>>")
		r = sess.read(length)
		print("read>>>")
		sess.release()
		print("return>>>")
		return r

	def write(self,path,buf,offset,fh=None):
		print("write>>>>>>>>",path,len(buf),offset)
		sess = self.op.putfile(path,offset)
		sess.write(buf)
		sess.release()
		return len(buf)

	def fgetattr(self,path,fh=None):
		print("fgetattr>>>>>>",path)
		return self.getattr(path)

	def chown(self,path,uid,gid):
		return 0

	def chmod(self,path,mode):
		return 0

	def truncate(self,path,offset):
		return 0

	def utime(self,tm):
		return 0

	def ftruncate(self,path,length,fh=None):
		print("ftruncate>>>>>",path)
		return 0

	def flush(self,path,fh=None):
		print("flush>>>>>>>",path)
		return 0

	def release(self,path,fh=None):
		print("release>>>>>>>>",path)
		return 0

	def fsync(self,path,fdatasync,fh=None):
		print("fsync>>>>>>>>>>",path)
		return 0

def testFS():
	usage='''    ftpfuse filesystem: mount a ftp server on a local filesystem
usage: ftpfuse.py ftpurl mountpoint'''
	if(len(sys.argv) < 2):
		print usage
		return
	server=FtpFS(sys.argv[1],version="%prog "+fuse.__version__,usage=usage)
	server.flags = 0
	server.multithreaded = 0
	server.parse(errex=1)
	server.main()

def testFileDetail():
	s='drwxrwxr-x 9 roy roy 4096 Nov  9 21:06 hongquanproject 1'
	f = FileDetail(s)
	f.parse()
	print f

if __name__=="__main__":
	testFS()
	
