#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <sys/resource.h>
#include <FileLog.h>
#include <strHelper.h>

#define _FILE_OFFSET_BITS 64
#include "CdmiFuseLinux.h"
#include <fuse/fuse.h>
#include <sys/xattr.h>
#include <dirent.h>
#include "FuseMirror.h"
#include "../cachelayer.h"
#include <FileLog.h>

using namespace CacheLayer;

size_t sleepFactor = 1;
bool  enableCache = true;
bool  foreground = false;


class MirrorCacheTank : public DataTank {
public:
	MirrorCacheTank( ZQ::common::NativeThreadPool& readPool, ZQ::common::NativeThreadPool& writePool, ZQ::common::Log& logger, DataTankConf& tankConf)
		:DataTank(readPool, writePool, logger, tankConf) {
	}
	virtual bool isSuccess( int err , size_t* size) const	{
		return err == 0 || err == 200;
	}
	virtual	int directWrite( const std::string& filename, 
										const char* buf, 
										unsigned long long offset , size_t& size ) {
		waitAWhile( size );
		int rc = mirror_write_direct( filename.c_str(), buf, size, offset,0);
		size = rc;
		return size >= 0 ? 200: 500;
	}

	virtual int	directWrite( const std::string& filename, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal) {
		waitAWhile( sizeTotal );
		std::vector<CacheLayer::DataBuffer>::const_iterator it = bufs.begin();
		for(; it != bufs.end() ; it ++ ) {
			size_t size = it->size;
			if( directWrite( filename, it->buf, it->offset, size) != 200 )
				return 500;
		}
		return 200;
	}

	virtual int directRead( const std::string& filename, 
										char* buf, 
										unsigned long long offset, size_t& size ) {
		waitAWhile( size );
		int rc = mirror_read_direct( filename.c_str(), buf, size, offset , 0 );
		size = rc;
		return rc >=0 ? 200:500;
	}
	virtual	int	directRead( const std::string& filename, unsigned long long offset, 
			const std::vector<DataBuffer>& bufs, size_t& sizeTotal) { 
		waitAWhile( sizeTotal );
		struct iovec* iov = (struct iovec*)(malloc(sizeof(struct iovec)*bufs.size()));
		assert(iov!=NULL);
		std::vector<DataBuffer>::const_iterator it = bufs.begin();
		struct iovec* p = iov;
		while( it != bufs.end() ) {
			p->iov_base = it->buf;
			p->iov_len = it->size;
			p++;
			it++;
		}
		int rc = mirror_read_direct( filename.c_str(), iov, bufs.size(), offset, 0 );
		sizeTotal = rc;
		return rc >= 0 ? 200 : 500;
	}


private:
	void waitAWhile( size_t size ) {
		if( sleepFactor >= 10 )
			ZQ::common::delay(sleepFactor);
	}
};

MirrorCacheTank *cache = 0;
ZQ::common::NativeThreadPool *pool = 0;
//ZQ::common::Log logger(ZQ::common::Log::L_DEBUG);
ZQ::common::Log* logger = 0;

#define MLOG	(*logger)

static std::string toFolder;
static std::string confPath;

std::string newpath( const char * opPath )
{
	return toFolder + opPath;
}

std::string errstr(int err ) {
	char buf[512] = {0};
	strerror_r(err,buf,sizeof(buf));
	return std::string(buf);
}
inline std::string getErrStr(int err ) {
	return err == 0?"OK":errstr(errno);
}
#define RETERR() (err == 0 ? 0 : -errno)
int		mirror_getattr( const char* opPath, struct stat* st) {
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(mirror,"getattr() file:%s"),opPath);
	int err = stat( newpath(opPath).c_str(), st);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(mirror,"getattr() file:%s, got[%s]"),opPath,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_readlink( const char* opPath, char* linkpath, size_t bufSize ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"readlink(), file: %s"),opPath);
	int err= readlink( newpath(opPath).c_str(), linkpath, bufSize );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"readlink(), file: %s , got[%s]"),opPath,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_mkdir( const char* opPath, mode_t mode ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"mkdir(), folder: %s "),opPath);
	int err =  mkdir(newpath(opPath).c_str(), mode );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"mkdir(), folder: %s, got[%s] "),opPath,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_unlink( const char* opPath ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"unlink(), file: %s "),opPath);
	int err =  unlink( newpath(opPath).c_str() ) ;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"unlink(), file: %s, got[%s] "),opPath,getErrStr(err).c_str());
	return RETERR();
}

int		mirror_rmdir( const char* opPath ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"rmdir(), folder: %s "),opPath);
	int err= rmdir( newpath(opPath).c_str() );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"rmdir(), folder: %s, got[%s] "),opPath,getErrStr(err).c_str());
	return RETERR();
}

int		mirror_symlink(const char* opPath, const char* target) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"symlink(), file: %s-%s "),opPath,target);
	int err = symlink( newpath(opPath).c_str(), newpath(target).c_str());
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"symlink(), file: %s-%s, got[%s] "),opPath,target,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_rename( const char* opPath, const char* target ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"rename(), file: %s-%s "),opPath,target);
	if(enableCache) {
		cache->validate_writebuffer( newpath(target) );
		cache->validate_readcache( newpath(target) );
	}
	int err =  rename( newpath(opPath).c_str(), newpath(target).c_str() );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"rename(), file: %s-%s, got[%s] "),opPath,target,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_link( const char* opPath, const char* target ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"link(), file: %s-%s "),opPath,target);
	int err =  link( newpath(opPath).c_str(), newpath(target).c_str() );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"link(), file: %s-%s ,got[%s]"),opPath,target,getErrStr(err).c_str());
	return RETERR();
}

int		mirror_chmod( const char* opPath, mode_t mode ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"chmod(), file: %s to %d "),opPath, mode);
	int err = chmod( newpath(opPath).c_str(), mode );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"chmod(), file: %s to %d, got[%s] "),opPath, mode, getErrStr(err).c_str() );
	return RETERR();
}

int		mirror_chown( const char* opPath,uid_t uid, gid_t gid) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"chown(), file: %s/%d/%d "),opPath,uid,gid);
	int err= chown( newpath(opPath).c_str(), uid, gid );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"chown(), file: %s/%d/%d ,got[%s]"),opPath,uid,gid,getErrStr(err).c_str());
	return RETERR();
}

int		mirror_truncate( const char* opPath, off_t offset ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"truncate(), file: %s to %ld "),opPath, offset);
	int err =  truncate( newpath(opPath).c_str(), offset );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"truncate(), file: %s to %ld ,got[%s]"),opPath, offset, getErrStr(err).c_str());
	return RETERR();
}
int	mirror_open( const char* opPath, struct fuse_file_info* info ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"open(), file: %s "),opPath);
	int fid = open( newpath(opPath).c_str(), info->flags );
	if( fid >= 0 )
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"open(), file: %s, got[%s] "),opPath,"OK");
		info->fh = fid;
		return 0;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"open(), file: %s, got[%s] "),opPath,errstr(errno).c_str());
		return -errno;
	}
}

int		mirror_read( const char* opPath, char* buf, size_t bufSize, off_t offset, struct fuse_file_info* info ) {
	StopWatch ts;ts.start();
	fuse_context* ctx = fuse_get_context();
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"read(), file: %s offset:%ld size:%d, cache[%s] "),
			opPath,offset,(int)bufSize, enableCache?"true":"false");
	int gotSize = 0;
	if(!enableCache) {
		gotSize =  mirror_read_direct( opPath, buf, bufSize, offset, info);
	} else { 
	unsigned int size = bufSize;
	if( 0 == cache->cacheRead( newpath(opPath) , buf, (unsigned long long)offset, size) )
		gotSize =  (int)size;
	else
		gotSize = -errno;
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"read(), file: %s pid[%d] offset:%ld size:%d,got [%d] , time cost[%lld]us "),
			opPath,(int)ctx->pid, offset,(int)bufSize,gotSize, ts.stop());
	return gotSize;
}

int		mirror_read_direct( const char* opPath, struct iovec* iov, int iovcount, off_t offset, struct fuse_file_info* info )  {
	FILE* f = fopen(opPath,"r+");
	if(!f)
		return -2;
	lseek(fileno(f),offset,SEEK_SET);
	ssize_t rc = readv(fileno(f),iov,iovcount);
	fclose(f);
	return rc;
}

int		mirror_read_direct( const char* opPath, char* buf, size_t bufSize, off_t offset, struct fuse_file_info* info ) {
	if(info) {
		lseek(info->fh,offset,SEEK_SET);
		return (int)read(info->fh,buf,bufSize);
	} else 	{
		FILE* f = fopen(opPath,"r+");
		if(!f)
			return -2;
		fseek(f,offset,SEEK_SET);
		size_t rc = fread(buf,1,bufSize,f);
		fclose(f);
		return rc;
	}
}

int		mirror_write( const char* opPath, const char* buf, size_t bufSize, off_t offset, struct fuse_file_info* info ) {
	StopWatch ts;ts.start();
	fuse_context* ctx = fuse_get_context();
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"write(), file: %s offset:%ld size:%d cache[%s]"),
			opPath,offset,(int)bufSize, enableCache ? "true":"false");
	int gotSize = 0;
	if( !enableCache ) {
		gotSize =  mirror_write_direct(opPath, buf, bufSize, offset, info);
	} else {
		unsigned int size = bufSize;
		if( 0 == cache->cacheWrite( newpath(opPath) , buf, (unsigned long long)offset, size ) )
			gotSize =  (int)size;
		else
			gotSize = -errno;
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"write(), file: %s pid[%d] offset:%ld size:%d, got[%d] took[%lld]us"),
			opPath,(int)ctx->pid, offset,(int)bufSize,gotSize, ts.stop());
	return gotSize;

}

int		mirror_write_direct( const char* opPath, const char* buf, size_t size, off_t offset, struct fuse_file_info* info ) {
 	if(info) {
		lseek(info->fh,offset,SEEK_SET);
		return (int)write(info->fh,buf,size);
	} else {
		FILE* f = fopen(opPath,"r+");
		if(!f)
			return -2;
		fseek(f,offset,SEEK_SET);
		size_t rc = fwrite(buf,1,size,f);
		fclose(f);
		return rc;
	}
}

int		mirror_statfs( const char* opPath, struct statvfs* st) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"statfs(), file: %s "),opPath);
	int err = statvfs( newpath(opPath).c_str(), st);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"statfs(), file: %s, got[%s]"),opPath,getErrStr(err).c_str());
	return RETERR();
}

int		mirror_flush( const char* opPath, struct fuse_file_info* info ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"flush(), file: %s "),opPath);
	int err = 0;
	if ( enableCache ) {
		err = cache->validate_writebuffer(newpath(opPath).c_str() );
	} else {
	}
	if( (err >= 200 && err < 300) || err == 0  )
		err = 0 ;
	else 
		err = -1;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"flush(), file: %s, got[%s] "),opPath,"OK");
	return err;
}
int		mirror_release( const char* opPath, struct fuse_file_info* info  ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"release(), file: %s "),opPath);
	int err = 0;
	if( info) {
		err = close(info->fh );
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"release(), file: %s, got [%s] "),opPath, getErrStr(err).c_str());
	return RETERR();
}
int		mirror_fsync( const char* opPath, int fd, struct fuse_file_info *info  ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"fsync(), file: %s "),opPath);
	if( !enableCache ) {
		int err = fsync(fd);
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"fsync(), file: %s,got[%s] "),opPath, getErrStr(err).c_str());
		return RETERR();
	} else {
		cache->validate_writebuffer( newpath(opPath) );
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"fsync(), file: %s,got[%s] "),opPath,"OK");
		return 0;
	}
}
int		mirror_setxattr( const char* opPath, const char* name, const char* value, size_t size, int flag ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"setxattr(), file: %s, key:%s"),opPath,name);
	int err = setxattr( newpath(opPath).c_str(),name,value,size,flag);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"setxattr(), file: %s, key:%s, got[%s]"),opPath,name,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_getxattr( const char* opPath, const char* name, char* value, size_t bufSize) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"getxattr(), file: %s key:%s "),opPath,name);
	int err =  getxattr( newpath(opPath).c_str(), name, value, bufSize );
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"getxattr(), file: %s key:%s "),opPath,name);
	return RETERR();
}
int		mirror_listxattr( const char* opPath, char* list, size_t size ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"listxattr(), file: %s "),opPath);
	int err = listxattr( newpath(opPath).c_str(), list, size) ;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"listxattr(), file: %s, got[%s] "),opPath, getErrStr(err).c_str());
	return RETERR();
}
int		mirror_removexattr( const char* opPath, const char* name ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"removexattr(), file: %s key:%s "),opPath,name);
	int err = removexattr( newpath(opPath).c_str(), name );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"removexattr(), file: %s key:%s "),opPath,name);
	return RETERR();
}
int		mirror_opendir( const char* opPath, struct fuse_file_info * info) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"opendir(), file: %s "),opPath);
	DIR* d = opendir( newpath(opPath).c_str() );
	if( d)
	{
		info->fh = reinterpret_cast<uint64_t>(d);
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"opendir(), file: %s, got[%s] "),opPath,"OK");
		return 0;
	}
	else {
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"opendir(), file: %s, got[%s] "),opPath, errstr(errno).c_str());
		return -errno;
	}
}

int		mirror_readdir( const char* opPath, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* info  ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"readdir(), file: %s "),opPath);
	struct dirent* d = 0;
	while((d=readdir(reinterpret_cast<DIR*>(info->fh))) != 0 )
	{
		if( filler(buf,d->d_name,0,0) == 1 )
			break;
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"readdir(), file: %s, got[%s] "),opPath,"OK");
	return 0;
}
int		mirror_releasedir( const char* opPath, struct fuse_file_info* info  ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"releasedir(), file: %s "),opPath);
	int err = closedir(reinterpret_cast<DIR*>(info->fh));
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"releasedir(), file: %s, got[%s] "),opPath, getErrStr(err).c_str());
	return RETERR();
}
int		mirror_fsyncdir( const char* opPath, int fd, struct fuse_file_info* info  ) {
	return	0;
}
int		mirror_access( const char* opPath, int mode ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"access(), file: %s mode: %d "),opPath,mode);
	int err = access( newpath(opPath).c_str(), mode);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"access(), file: %s mode:%d, got[%s] "),opPath,mode, getErrStr(err).c_str());
	return RETERR();
}
int		mirror_create( const char* opPath, mode_t mode, struct fuse_file_info* info) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"create(), file: %s mode:%d "),opPath,mode);
	int fd = creat( newpath(opPath).c_str(), mode);
	if(fd<0) {
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"create(), file: %s mode:%d, got[%s]"),opPath,mode, errstr(errno).c_str());
		return -errno;
	}
	info->fh = fd;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"create(), file: %s mode:%d, got[%s] "),opPath,mode,"OK");
	return 0;

}
int		mirror_ftruncate( const char* opPath, off_t offset, struct fuse_file_info* info  ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"ftruncate(), file: %s offset:%ld "),opPath,offset);
	int err = truncate( newpath(opPath).c_str(), offset);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"ftruncate(), file: %s offset:%ld, got[%s]"),opPath,offset,getErrStr(err).c_str());
	return RETERR();
}
int		mirror_fgetattr( const char* opPath, struct stat* st, struct fuse_file_info* info ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(mirror,"fgetattr(), file: %s "),opPath);
	int err =  stat( newpath(opPath).c_str(), st);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(mirror,"fgetattr(), file: %s, got[%s] "),opPath, getErrStr(err).c_str());
	return RETERR();
}
int		mirror_lock( const char* opPath, struct fuse_file_info* info  , int cmd, struct flock* lk) {
	return -1;
}
int		mirror_utimens( const char* opPath, const struct timespec tv[2]) {
	return 0;
}

void* mirror_init(struct fuse_conn_info *conn)
{
	struct rlimit rl;
	rl.rlim_cur = RLIM_INFINITY;
	rl.rlim_max = RLIM_INFINITY;
	setrlimit( RLIMIT_CORE, &rl);
	std::string logPath = "/opt/cdmifuse/logs/cdmi.log";
	int logLevel = 7;
	int logCount = 5;
	int logSize = 10 * 1024 * 1024;
	FuseConfig conf;
	if(!conf.loadConfig( confPath, foreground)) {
		ZQ::common::SysLog slogger("CdmiFuse");
		slogger(ZQ::common::Log::L_ERROR,"[cdmiFuse] failed to load conf[%s]", confPath.c_str());
		return 0;
	} else {
		logPath = conf.logFilePath;
		logLevel = conf.logLevel;
		logCount = conf.logCount;
		logSize  = conf.logSize;
		sleepFactor = conf.sleepFactor;
		enableCache = conf.fuseOpsConf.enableCache;
	}

	ZQ::common::FileLog* pLog = new ZQ::common::FileLog(logPath.c_str(),
														logLevel,logCount,logSize);
	logger = pLog;
	if( enableCache) {
		pool = new ZQ::common::NativeThreadPool();
		ZQ::common::NativeThreadPool*writePool = new ZQ::common::NativeThreadPool();

		cache = new MirrorCacheTank(*pool,*writePool, *pLog, conf.fuseOpsConf.tankConf);
		cache->createTank();
	}
	(*pLog)(ZQ::common::Log::L_INFO, "[cdmiFuse] load conf[%s], sleepFactor[%zu] cache[%s]",
				confPath.c_str(), sleepFactor, enableCache?"true":"false");

	return 0;
}

void mirror_destroy(void *)
{
	if(cache) {
		cache->destroyTank();
		delete cache;
	}
	if( pool ) {
		pool->stop();
		delete pool;
	}
}

void setDest( const std::string& folder )
{
	toFolder = folder;
	if(!toFolder.empty())
	{
		if(toFolder.at(toFolder.length()-1) == '/')
			toFolder = toFolder.substr(0,toFolder.length()-1);
	}
}

void    setConfPath( const std::string& path ) {
	confPath = path;
}

void	setForeground() {
	foreground = true;
}
