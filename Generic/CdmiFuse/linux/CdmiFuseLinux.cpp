#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
#include <errno.h>
#include <assert.h>
#include <sstream>
#include <fstream>
#include <FileLog.h>
#include <strHelper.h>
#include "../CDMIHttpClient.h"
#include "CdmiFuseLinux.h"
#include "CryptoAlgm.h"
#include "FuseMirror.h"
#define _CUSTOM_TYPE_DEFINED_
#include "../sdk/AquaClient.h"
#include "../cachelayer.h"

using namespace XOR_Media::AquaClient;
using namespace CacheLayer;

CdmiFuseFS gFs;

static const std::string fs_name = "cdmifs";

int CdmiFuseFS::cdmi_getattr(const char* path, struct stat *st)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->getattr( path, st );
}

int CdmiFuseFS::cdmi_readlink (const char * path, char *buf, size_t size)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->readlink( path, buf, size );
}

int CdmiFuseFS::cdmi_mkdir(const char *path, mode_t mode)
{
 	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->mkdir( path, mode );
}

int CdmiFuseFS::cdmi_unlink(const char * path)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->unlink( path );
}

int CdmiFuseFS::cdmi_rmdir(const char * path )
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->rmdir( path );
}

int CdmiFuseFS::cdmi_symlink(const char * path, const char * sym )
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->symlink( path, sym);
}

int CdmiFuseFS::cdmi_rename(const char * path, const char * to)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->rename( path, to );
}

int CdmiFuseFS::cdmi_link(const char * path, const char * sym)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->link( path, sym );
}

int CdmiFuseFS::cdmi_chmod(const char * path, mode_t mode)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->chmod( path, mode );
}

int CdmiFuseFS::cdmi_chown(const char * path, uid_t u, gid_t g)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->chown( path,u,g );
}

int CdmiFuseFS::cdmi_truncate(const char * path, off_t offset)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->truncate( path, offset );
}


int CdmiFuseFS::cdmi_open(const char * path, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	int rc = op->open( path, info );
	return rc;
}

int CdmiFuseFS::cdmi_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path );
	assert( op != 0 );
	return op->read( path, buf, size, offset, info );
}

int CdmiFuseFS::cdmi_write(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path);
	assert( op != 0 );
	return op->write( path, buf, size, offset, info );
}

int CdmiFuseFS::cdmi_statfs(const char * path, struct statvfs * vfs)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op!=0 );
	return op->statfs( path, vfs );
}

int CdmiFuseFS::cdmi_flush(const char * path, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path );
	assert( op != 0 );
	return op->flush(path,info);
}

int CdmiFuseFS::cdmi_release(const char * path, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh );
	if(op)
	{
		op->release( path, info);
	}
	return 0;// FIXME: is this OK?
}

int CdmiFuseFS::cdmi_fsync(const char * path, int fh, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh );
	if(!op)
	{
		op = gFs.getOp(fh, path );
	}
	assert(op);
	return op->fsync( path, fh );
}

int CdmiFuseFS::cdmi_setxattr(const char * path, const char * key, const char * value, size_t size, int flag)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->setxattr( path, key, value, size, flag );
}

int CdmiFuseFS::cdmi_getxattr(const char * path, const char * key, char *value, size_t size)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->getxattr( path, key, value, size );
}

int CdmiFuseFS::cdmi_listxattr(const char * path, char * buf, size_t size)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->listxattr( path, buf, size );
}

int CdmiFuseFS::cdmi_removexattr(const char * path, const char * key)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->removexattr( path, key );
}

int CdmiFuseFS::cdmi_opendir(const char *path, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getMainOp(path);
	assert( op != 0 );
	int rc = op->opendir( path, info );
	return rc;
}

int CdmiFuseFS::cdmi_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path );
	assert( op != 0 );
	return op->readdir( path, buf, filler, offset, info );
}

int CdmiFuseFS::cdmi_releasedir(const char * path, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh );
	if(op)
	{
		op->releasedir( path, info);
	}
	return 0;
}

int CdmiFuseFS::cdmi_fsyncdir(const char * path, int fh, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh);
	if(!op)
	{
		op = gFs.getOp( fh, path );
	}
	if(!op)
	{
		return -EINVAL;
	}
	return op->fsyncdir( path, fh, info );
}



int CdmiFuseFS::cdmi_access(const char *path, int mode)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->access( path, mode );
}

int CdmiFuseFS::cdmi_create(const char * path, mode_t mode, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getMainOp(path);
	assert( op != 0 );
	int rc = op->create( path, mode, info );
	return rc;
}

int CdmiFuseFS::cdmi_ftruncate(const char * path, off_t offset, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path );
	assert( op != 0 );
	return op->ftruncate( path, offset, info );
}

int CdmiFuseFS::cdmi_fgetattr(const char * path, struct stat * st, struct fuse_file_info * info)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path );
	assert( op != 0 );
	return op->fgetattr( path, st, info );
}

int CdmiFuseFS::cdmi_lock(const char * path, struct fuse_file_info * info, int cmd, struct flock * lk)
{
	CdmiFuseOpLinux* op = gFs.getOp( info->fh, path );
	assert( op != 0 );
	return op->lock( path,cmd, lk, info );
}

int CdmiFuseFS::cdmi_utimens(const char * path, const struct timespec tv[2])
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->utimens(path,tv);
}

int CdmiFuseFS::cdmi_bmap(const char *, size_t blocksize, uint64_t *idx)
{
	return -1; //FIXME: not implemented yet
}

int CdmiFuseFS::cdmi_ioctl(const char * path, int cmd, void *arg, struct fuse_file_info * info, unsigned int flag, void *data)
{
	CdmiFuseOpLinux* op = gFs.getMainOp( path );
	assert( op != 0 );
	return op->ioctl(path, cmd, arg, info, flag, data);
}

int CdmiFuseFS::cdmi_poll(const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp)
{
	return -1;//FIXME: not implemented yet
}

void CdmiFuseFS::uninit_cdmi_fuse()
{
	if(mLogger)
	{
		(*mLogger)(ZQ::common::Log::L_INFO, "trying to stop CURLClientManager");
	}
	CdmiFuseOps::stopCURLenv();
	ZQ::common::MutexGuard gd( mLocker );
	CdmiFuseOpLinux* p = 0;
	OPMAP::iterator it = mOps.begin();
	for ( ; it != mOps.end(); it ++ )
	{
		p = it->second.ops;
		break;
	}
	if(p)
		delete p;
	mOps.clear();

	if( mLogger )
	{
		(*mLogger)(ZQ::common::Log::L_INFO, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> stopped <<<<<<<<<<<<<<<<<<<<<<<<<<< ");
		delete mLogger;
		mLogger = 0;
	}
}

bool CdmiFuseFS::init_cdmi_fuse() {
	{
		//change working directory
		chdir( mConfig.dump_path.c_str() );
		int fd = open("/proc/sys/kernel/core_pattern", O_WRONLY);
		if(fd != (-1)) {
			write(fd, "core.%e.%s.%t", 14);
			close(fd);
		}
	}

	mThdPool = new ZQ::common::NativeThreadPool( mConfig.curlThrdPoolSize  );	
	mLogger = new ZQ::common::FileLog( mConfig.logFilePath.c_str(),mConfig.logLevel, mConfig.logCount, mConfig.logSize );
	gFs.mLogger = mLogger;
	if(!mLogger)
	{
		ZQ::common::SysLog logger("CdmiFuse");
		logger(ZQ::common::Log::L_ERROR, "[cdmiFuse] failed to open log at [%s]",mConfig.logFilePath.c_str());
		return false;
	}
	CdmiFuseOps::startCURLenv();
	(*gFs.mLogger)(ZQ::common::Log::L_INFO, " ================================= starting ========================== ");
	(*gFs.mLogger)(ZQ::common::Log::L_INFO, "start fuse with: fuseSetting[%s] cdmiRoot[%s] userDomain[%s] homeContainer[%s] thdCount[%d] cache[%s] del_before_mv[%s] logflag[%x] dumpPath[%s]", 
			mConfig.fuse_setting.c_str(),
			mCdmiServerRoot.c_str(),
			mConfig.cdmiUserDomain.c_str(),
			mCdmiHomeContainer.c_str(),
			mConfig.curlThrdPoolSize,
			mConfig.fuseOpsConf.enableCache?"enable":"disable",
			mConfig.aqua_del_before_mv>=1?"enable":"disable",
			(unsigned int)mConfig.fuseOpsConf.fuseFlag,
			mConfig.dump_path.c_str() );

	mMainOp = new CdmiFuseOpLinux( *gFs.mLogger, *gFs.mThdPool, gFs.mCdmiServerRoot, mConfig.cdmiUserDomain, gFs.mCdmiHomeContainer, "", gFs, mConfig.fuseOpsConf, 0 );
	mMainOp->setTimeout(mConfig.connectTimeout, mConfig.operationTimeout, mConfig.retryTimeout);
	FileHandleInfo info;
	info.ops = mMainOp;
	info.disableCache = false;
	mOps[0] = info;
	(*gFs.mLogger)(ZQ::common::Log::L_INFO, "created CdmiFuseOpLinux, id[%lld], connTimeout[%d] operationTimeout[%d]",
			0LL, mConfig.connectTimeout, mConfig.operationTimeout );
	(*gFs.mLogger)(ZQ::common::Log::L_INFO, "creating folder[%s] at root",fs_name.c_str());

	int rcRootFolder = CdmiFuseFS::cdmi_mkdir(fs_name.c_str(), 0775);
	(*gFs.mLogger)(ZQ::common::Log::L_INFO, "created folder[%s] at root, [%s]",fs_name.c_str(), rcRootFolder == 0 ?"OK":"FAILED");

	return true;
}

void* CdmiFuseFS::cdmi_init(struct fuse_conn_info *conn)
{
	struct rlimit rl;
	rl.rlim_cur = RLIM_INFINITY;
	rl.rlim_max = RLIM_INFINITY;
	setrlimit( RLIMIT_CORE, &rl);
	chdir("/opt/cdmifuse/logs/crashdump");
	gFs.init_cdmi_fuse();
	return 0;
}

void CdmiFuseFS::cdmi_destroy(void *)
{
	gFs.uninit_cdmi_fuse();
}

bool CdmiFuseFS::initialize( const FuseConfig& conf )
{
	mConfig = conf;

	mCdmiServerRoot = mConfig.cdmiBaseUrl;
	mCdmiHomeContainer = mConfig.cdmiHomeContainer;
	mCdmiBindIp   = mConfig.curlBindIp;
	size_t thdSize = mConfig.curlThrdPoolSize;
	thdSize = MAX(3,thdSize);
	thdSize = MIN(200,thdSize);
	mConfig.curlThrdPoolSize = thdSize;

	return true;
}



void CdmiFuseFS::signal_caught( const char* sigstr, pid_t pid, uid_t uid) {
	if(!mLogger)
		return;
	(*mLogger)(ZQ::common::Log::L_WARNING, "caught a signal[%s] coming from process[%d] usr[%d]",
			sigstr,pid,uid);
}


CdmiFuseFS::CdmiFuseFS()
	:mThdPool(0),
	mMainOp(0),
	mLogger(0),
	mBaseOpId(1)
{
}

CdmiFuseFS::~CdmiFuseFS()
{
}

//void CdmiFuseFS::putDirCache( long long int id, const DIRENTVECTOR& cache )
//{
//	ZQ::common::MutexGuard gd( mLocker );
//	mDirCaches[id] = cache;
//}
//
//CdmiFuseFS::DIRENTVECTOR CdmiFuseFS::getDirCache( long long int id )
//{
//	ZQ::common::MutexGuard gd(mLocker);
//	std::map<long long int, DIRENTVECTOR>::const_iterator it = mDirCaches.find(id);
//	if( it == mDirCaches.end() )
//		return DIRENTVECTOR();
//	return it->second;
//}
//
//void CdmiFuseFS::removeDirCache( long long int id )
//{
//	ZQ::common::MutexGuard gd(mLocker);
//	mDirCaches.erase(id);
//}

int64 CdmiFuseFS::addFd(CdmiFuseOpLinux* op, bool disableCache, const char* pathname)
{
	ZQ::common::MutexGuard gd(mLocker);
	int64 fd = mBaseOpId ++;

	if (NULL == pathname)
		pathname = "";
	
	FileHandleInfo fhinfo;
	fhinfo.ops = op;
	fhinfo.disableCache = disableCache;
	fhinfo.pathname     = pathname;

	mOps[fd] = fhinfo;
#ifdef _DEBUG
	(*mLogger)(ZQ::common::Log::L_DEBUG, CLOGFMT(LinuxFuse, "added file handle[%lld]->path[%s]"), fd, pathname);
#endif // _DEBUG
	return fd;
}

void CdmiFuseFS::removeFd( int64 fd )
{
	ZQ::common::MutexGuard gd(mLocker);
	mOps.erase( fd );
#ifdef _DEBUG
	(*mLogger)(ZQ::common::Log::L_DEBUG, CLOGFMT(LinuxFuse, "fd[%lld] removed"), fd);
#endif // _DEBUG
}

std::string CdmiFuseFS::pathOfFd( int64 fd )
{
	ZQ::common::MutexGuard gd(mLocker);
	OPMAP::iterator it = mOps.find(fd);
	if (mOps.end() == it)
		return "";

	return it->second.pathname;
}

CdmiFuseOpLinux* CdmiFuseFS::getMainOp( const char * path )
{
	return mMainOp;
	/**
	long long int tid = 0;
	tid = - tid;
	CdmiFuseOpLinux* op = 0;
	ZQ::common::MutexGuard gd(mLocker);
	OPMAP::iterator it = mOps.find( tid );
	if( it == mOps.end()) 
	{
		op = new CdmiFuseOpLinux( *gFs.mLogger, *gFs.mThdPool, gFs.mCdmiServerRoot, gFs.mCdmiHomeContainer, path, gFs, mConfig.fuseOpsConf, tid );
		op->setTimeout(mConfig.connectTimeout, mConfig.operationTimeout, mConfig.retryTimeout);
		FileHandleInfo info;
		info.ops = op;
		info.disableCache = false;
		mOps[tid] = info;
		(*mLogger)(ZQ::common::Log::L_INFO, "created CdmiFuseOpLinux, id[%lld], connTimeout[%d] operationTimeout[%d]",
			tid, mConfig.connectTimeout, mConfig.operationTimeout );
	}
	else
	{
		op = it->second.ops;
	}
	assert( op != 0 );
	op->updateTarget( path );
	return op;
*/
}

CdmiFuseOpLinux* CdmiFuseFS::getOp( long long id, const char* path )
{
	CdmiFuseOpLinux* info = 0;
	ZQ::common::MutexGuard gd(mLocker);
	OPMAP::iterator it = mOps.find( id );
	if( it == mOps.end() )
	{
		return 0;
	}
	info = it->second.ops;
	if( path )
	{
		info->updateTarget( path );
	}
	return info;
}

bool CdmiFuseFS::isFileDisableCache( long long id ) {
	ZQ::common::MutexGuard gd(mLocker);
	OPMAP::iterator it = mOps.find(id);
	if( it == mOps.end() )
		return false;
	return it->second.disableCache;
}

void CdmiFuseFS::setDirty( long long id ) {
	ZQ::common::MutexGuard gd( mLocker );
	OPMAP::iterator it = mOps.find( id );
	if( it == mOps.end() )
		return;
	it->second.dirty = true;
	it->second.stampLastDirty = ZQ::common::now();
}

bool CdmiFuseFS::getDirty(long long id, bool bClear) 
{
	ZQ::common::MutexGuard gd(mLocker);
	OPMAP::iterator it = mOps.find( id );
	if( it == mOps.end() )
		return false;
	bool ret = it->second.dirty;
	if (bClear)
		it->second.dirty = false;

	return ret; 
}

bool CdmiFuseFS::hasEverDirty(long long id)
{
	ZQ::common::MutexGuard gd(mLocker);
	OPMAP::iterator it = mOps.find( id );
	if( it == mOps.end() )
		return false;

	return (it->second.stampLastDirty > 0);
}

/////////////////////////////////////////////////////////////////
//metadata operation


/////////////////////////////////////////////////////////////////
//CdmiFuseOpLinux
CdmiFuseOpLinux::CdmiFuseOpLinux( ZQ::common::Log& logger, 
		ZQ::common::NativeThreadPool& pool, 
		const std::string& cdmiRootUrl,
		const std::string& userDomain,
		const std::string& homeContainer,
		const std::string& opPath,
		CdmiFuseFS& fs,
		const FuseOpsConf& conf,
		long long id)
	:CdmiFuseOps(logger, pool, cdmiRootUrl, userDomain, homeContainer,conf.fuseFlag,conf,fs.getBindIp()),
	mOpId(id),
	mFs(fs)
{
}

CdmiFuseOpLinux::~CdmiFuseOpLinux()
{
}

#undef LOGFMT
// #define	LOGFMT(x,y)	"%s\\%08X\\FuseOpLinux[%s]\t"y,opPath,(unsigned int)pthread_self(),#x 
#define	LOGFMT(x, _FMT)	       FLOGFMT(LinuxFuse, x, "path[%s] " _FMT), opPath
#define MLOG _log

bool CdmiFuseOpLinux::isTargetAFolderCdmi( const char* opPath) 
{
	bool ret = false;
	struct stat st; memset(&st, 0, sizeof(st) );
	if( getattr( opPath, &st) )
		return false;
	ret = (st.st_mode & S_IFDIR) != 0;
	return ret;
}

bool CdmiFuseOpLinux::isTargetAFolder( const char* opPath, const char* target ) const
{
	const char* p = 0;
	size_t len = 0;

	if( target && target[0] != 0 )
	{
		p = target;
		len = strlen(target);
	}
	else
	{
		p = opPath;
		len = strlen(opPath);
	}
	if( !p || len <= 0 )
		return false;
	return p[len-1] == '/';
}

int	CdmiFuseOpLinux::cdmiCodeToSysCode( CdmiRetCode code ) const
{
	switch(code)
	{
	case cdmirc_OK:
	case cdmirc_Created:
	case cdmirc_Accepted:
	case cdmirc_PartialContent:
	case cdmirc_Found:
	case cdmirc_NoContent:
		return 0;

	case cdmirc_BadRequest:
		return -EINVAL;

		case cdmirc_Unauthorized:
		case cdmirc_Forbidden:
			return -EPERM;

	case cdmirc_NotFound:
		return -ENOENT;
	
	case cdmirc_NotAcceptable:
		return -EPERM;

	case cdmirc_Conflict:
		return -EEXIST;
		
	case cdmirc_ServerError:
		return -EIO;

	default:
	   return -EPERM;	
	}
	return 0;
}

std::string CdmiFuseOpLinux::toUrl( const std::string& url, bool bContainer )
{
	//step 1, check if this url is prefix by mountpoint
	static std::string mountpoint = mFs.config().fuse_mountpoint;
	std::string::size_type pos = url.find(mountpoint);
	std::string result;
	if( pos == 0 ) {//ignore any space leading url ?
		result = pathToUri( url.substr(mountpoint.length() + 1 ) ) ;
	} else {	
		result = pathToUri(url);
	}
	if( bContainer && result.length() > 0 && result.at(result.length() - 1) != '/')
		result = result + '/';
	return result;
}

int CdmiFuseOpLinux::getattr( const char* opPath, struct stat *st )
{
	fuse_context* ctx = fuse_get_context();
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(getattr, "caller[%d]"), (int)ctx->pid);
	StopWatch ts;

	CdmiFuseOps::FileInfo info;
	CdmiFuseOps::FileInfo_reset(info);

	CdmiFuseOps::CdmiRetCode code = getFileInfo(toUrl(opPath), info, false );
	if( !CdmiRet_SUCC(code))
	{
		MLOG(ZQ::common::Log::L_ERROR,LOGFMT(getattr, "failed due to [%s(%d)], caller[%d]"),
				cdmiRetStr(code), code, (int)ctx->pid);
		return cdmiCodeToSysCode(code);
	}

	bool	bFolder = ( info.filestat.st_mode & S_IFDIR  );
	// info.filestat.st_uid = getuid();
	// info.filestat.st_gid = getgid();
	// info.filestat.st_mode |= S_IRUSR|S_IWUSR;
	info.filestat.st_mode |= bFolder ? (S_IFDIR|S_IXUSR) : S_IFREG;
	info.filestat.st_nlink = bFolder ? 2: 1;
	info.filestat.st_blksize = 4096;
	info.filestat.st_blocks = (info.filestat.st_size / info.filestat.st_blksize) + (info.filestat.st_size % info.filestat.st_blksize ? 1: 0);

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(getattr, "attr gotten for caller[%d]: type[%s], took [%lld]us, stat: %s"),
			(int)ctx->pid, bFolder ? "folder":"file", ts.stop(), dumpFileStat(info.filestat).c_str());
	*st = info.filestat;
	return 0;
}

std::string  CdmiFuseOpLinux::dumpFileStat( struct stat& st ) const
{
	std::ostringstream oss;
	oss<<"mode["<< std::oct <<st.st_mode<< "] size["<< std::dec<<st.st_size<<"] blocks["<<st.st_blocks<<"] time["<<
		st.st_atime<<"/"<<st.st_mtime<<"/"<<st.st_ctime<<"]";
	return oss.str();
}

int CdmiFuseOpLinux::readlink( const char* opPath, char* linkPath, size_t bufSize )
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(readlink, "get link content for [%s]"), opPath );
	StopWatch ts;

	Json::Value val;
	std::string refLoc;
	CdmiRetCode code = cdmi_ReadDataObject( val, toUrl(opPath), refLoc );	
	if( !CdmiRet_SUCC( code) )
	{
		return cdmiCodeToSysCode( code );
	}
	//TODO: some additional action must be made to distinguish the reference location is 
	//      at another site
	size_t copySize = std::min( bufSize - 1, refLoc.length());
	strncpy( linkPath, refLoc.c_str(), copySize );
	return copySize > 0 ? 0 : -1;
}

int CdmiFuseOpLinux::mkdir( const char* opPath, mode_t mode )
{
	MLOG(ZQ::common::Log::L_DEBUG,LOGFMT(mkdir, "creating folder [%s]"), opPath);
	StopWatch ts;

	if (0 == fs_name.compare(opPath))
	{
		MLOG(ZQ::common::Log::L_WARNING, LOGFMT(mkdir, "sub-folder[%s] matches FS type, faked a SUCC"), opPath);
		return cdmiCodeToSysCode( cdmirc_Created );
	}

	CdmiRetCode retCode = nonCdmi_CreateContainer( toUrl(opPath) );

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(mkdir, "nonCdmi_CreateContainer return [%s] with url [%s], took[%lld]us"),
			cdmiRetStr(retCode), opPath, ts.stop() );

	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::unlink( const char* opPath)
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(unlink, "delete file %s"), opPath );
	
	StopWatch ts;

	CdmiRetCode ret = nonCdmi_DeleteDataObject( toUrl(opPath) );
	if (opPath && CdmiRet_SUCC(ret))
	{
		uncacheFileInfo(opPath);
		uncacheChild(opPath);
/*
		// uncache the children of parent
		const char* rpos = strrchr(opPath, '/');
		if (NULL != rpos)
		{
			std::string parent = std::string(opPath, 0, (size_t)(rpos-opPath));
			uncacheChildren(parent.c_str());
			MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(unlink, "delete() file %s, uncached children of dir[%s]"), opPath, parent.c_str());
		}
*/
	}

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(unlink, "delete file %s, got %s, took [%lld]us"),
		opPath, CdmiFuseOps::cdmiRetStr(ret), ts.stop());

	return cdmiCodeToSysCode( ret );
}

int CdmiFuseOpLinux::rmdir( const char* opPath )
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(unlink, "delete folder %s"), opPath );
	
	StopWatch ts;

	CdmiRetCode retCode = nonCdmi_DeleteContainer( toUrl(opPath) );

	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(unlink, "delete folder %s, got %s, took [%lld]us"),
		opPath, CdmiFuseOps::cdmiRetStr( retCode ), ts.stop(	) );

	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::symlink( const char* opPath, const char* target )
{
	if(!target || !target[0] )
		return -EINVAL;//invalid parameter, treat as not exist
	//FIXME: not implemented yet
	MLOG(ZQ::common::Log::L_DEBUG,LOGFMT(symlink, "link [%s] as [%s]"), opPath, target);
	return -EPERM;
}

int CdmiFuseOpLinux::link( const char* opPath, const char* target )
{
	if( !target || !target[0] )
		return -EINVAL;
	//FIXME: not implemented yet
	return -EPERM;
}

int CdmiFuseOpLinux::chmod( const char* opPath, mode_t mode )
{
	std::string newOwner = userString();
	MLOG(ZQ::common::Log::L_DEBUG,LOGFMT(chmod, "chmod() path[%s] changing to mode[%o] owner[%s] per emailfmt[%c]"), opPath, mode, newOwner.c_str(), (_mdOwnerInEmailFmt?'T':'F'));
	mode_t mask = 0x7;
	const char* role[3] = {"EVERYONE@", "AUTHENTICATED@", "OWNER@"};
	Json::Value metadata;
	Json::Value acl;
	for( size_t i = 0; i < 3; i ++ ) {
		uint32 aceFlags = 0;
		mode_t tmp = (mode >> i) & mask;
		if( tmp & 0x01 )
			aceFlags |= CDMI_ACE_EXECUTE;
		if( tmp & 0x02 )
			aceFlags |= CDMI_ACE_WRITE_OBJECT;
		if( tmp & 0x04 )
			aceFlags |= CDMI_ACE_READ_OBJECT;
		Json::Value val;
		val["acetype"] = "0x00000000";
		val["identifier"] = role[i];
		val["aceflags"] = "0x00000083";

		char buf[32]; sprintf(buf, "0x%x",aceFlags);
		val["acemask"] = buf;
		acl.append(val);
	}

	metadata["cdmi_acl"] = acl;
	metadata["cdmi_owner"] = newOwner;

	std::string location;
	CdmiRetCode cdmirc = cdmi_UpdateDataObjectEx(location, toUrl(opPath)+"?metadata", metadata, 0, std::string(""), -1);
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(chmod, "chmod() path[%s] changed to mode[%o], new owner[%s] got [%s]"), opPath, mode, _username.c_str(), cdmiRetStr(cdmirc) );
	
	if (opPath && CdmiRet_SUCC(cdmirc))
		uncacheFileInfo(opPath);

	return cdmiCodeToSysCode( cdmirc );
}

int CdmiFuseOpLinux::chown( const char* opPath, uid_t uid, gid_t gid )
{
	std::string user, group;
	// get user name from uid
	struct passwd* pwd = getpwuid( uid );
	if( pwd && pwd->pw_name )
	{
		user = pwd->pw_name;
	}
	// get group name from gid
	struct group* grp = getgrgid( gid );
	if( grp && grp->gr_name )
	{
		group = grp->gr_name;
	}
	
	MLOG(ZQ::common::Log::L_DEBUG,LOGFMT(chmodi, "change mode owner file [%s] to [%s/%s], actually nothing has been done"),
			opPath, user.c_str(), group.c_str() );
	return 0;
}

int CdmiFuseOpLinux::truncate( const char* opPath, off_t offset )
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(truncate, "truncate target [%s] at [%ld]"), opPath, offset);
	StopWatch ts;

	Json::Value val;
	std::string location;

	CdmiRetCode retCode = nonCdmi_UpdateDataObject( toUrl( opPath), location, "", offset, 0, 0, offset );
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(truncate, "truncate [%s] at [%ld], got [%s], took [%lld]us"),
			opPath, offset, cdmiRetStr(retCode), ts.stop() );
	if (opPath && CdmiRet_SUCC(retCode))
		uncacheFileInfo(opPath);
	
	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::rename( const char* opPath, const char* to )
{
	if( !to || !to[0] )
		return -EINVAL;
	StopWatch ts;

	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(rename, "renaming from [%s] to [%s]"), opPath, to );
	bool isFolder = isTargetAFolderCdmi( opPath );
	std::string destUrl = toUrl(to, isFolder );
	std::string srcUrl = toUrl( opPath, isFolder );
	
	if( mFs.config().aqua_del_before_mv >= 1 ) {
		unlink( to );
		MLOG(ZQ::common::Log::L_INFO, LOGFMT(rename, "deleted the target [%s] before renaming"), to );
	}
		
	Json::Value result;
	CdmiRetCode retCode = cdmirc_ServerError;
	if( isFolder )
	{
		retCode = cdmi_CreateContainer( result, destUrl, Properties(), "", "", "", "",srcUrl );
	}
	else
	{
		retCode = cdmi_CreateDataObject( result, destUrl, "", Properties(), "", StrList(), "", "", "", "",srcUrl );
	}
	
	std::string parents;

	if (opPath && CdmiRet_SUCC(retCode))
	{
		uncacheFileInfo(opPath);
		uncacheChild(opPath);
/*
		// uncache the children of parent
		const char* rpos = strrchr(opPath, '/');
		if (NULL != rpos)
		{
			std::string parent = std::string(opPath, 0, (size_t)(rpos-opPath));
			uncacheChildren(parent.c_str());
			parents = parent + ",";
		}
*/
	}

	if (to && CdmiRet_SUCC(retCode))
	{
		uncacheFileInfo(to);
		cacheChild(to);

		/*
		// uncache the entire children of destination parent
		const char* rpos = strrchr(to, '/');
		if (NULL != rpos)
		{
			std::string parent = std::string(to, 0, (size_t)(rpos-to));
			uncacheChild(to);
			parents += parent;
		}
		*/
	}

	MLOG(ZQ::common::Log::L_INFO,LOGFMT(rename, "renamed from[%s] to [%s] isFolder[%s], got [%s], uncached children of dir[%s], took [%lld]us"),
			srcUrl.c_str(), destUrl.c_str(), isFolder ? "true":"false", cdmiRetStr(retCode), parents.c_str(), ts.stop() );
	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::open( const char* opPath, struct fuse_file_info* info )
{
	if(!info)
		return -1;
	
	int flag = info->flags; // ??

	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(open, "opening file with flag[0x%x]"), (unsigned int)flag );
	
	StopWatch ts;
	std::string location;
	Json::Value val;
	int sysCode = -1;

	CdmiFuseOps::FileInfo fi;
	CdmiFuseOps::FileInfo_reset(fi);

	std::string pathname = toUrl(opPath);
	CdmiRetCode retCode = getFileInfo(pathname, fi, false ); // cdmi_ReadDataObject( val, toUrl( opPath ) + "?metadata", location );
	if( CdmiRet_SUCC( retCode ) )
	{
		if( flag & O_TRUNC )
		{
			MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(open, "target[%s] exists, truncate to 0 bytes"),
					opPath);
			sysCode= truncate(opPath, 0);
		}
		else sysCode = 0;
	}
	else if( retCode == cdmirc_NotFound )
	{
		MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(open, "target [%s] does not exist, creating it"), opPath );
		retCode = nonCdmi_CreateDataObject( toUrl( opPath), "" );
		sysCode = cdmiCodeToSysCode( retCode );
		if (CdmiRet_SUCC(retCode))
			cacheChild(opPath);
	}

	bool disableCache = (flag & mFs.config().flagDisableCache) != 0 ;
	
	if( sysCode == 0 )
	{
		//FIXME: add this to FS so that successor can invoke the right object instance
		info->fh = mFs.addFd(this, disableCache, pathname.c_str());
		CacheTank* tank = getCacheTank(opPath);
		assert( tank != NULL );
		CacheLayer::ReadCache& rCache = tank->getReadCache();
		if( rCache.cacheExistFor(opPath)) {
			info->keep_cache = true;
		} else {
			info->keep_cache = false;
		}
	}

	fuse_context* ctx = fuse_get_context();
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(open, "caller[%d] open file, got [%s] took [%lld]us: keep_cache[%c] fd[%lld]"),
			(int)ctx->pid, cdmiRetStr(retCode), ts.stop(), info->keep_cache ? 'T':'F', info->fh);
	return sysCode;
}

void CdmiFuseOpLinux::opvTestOnCache( const char* path ) {
	if( gFs.config().opvTestCacheByKeyword == 0 )
		return;
	static std::string cacheOn = gFs.config().opvTestCacheOnKeyword;
	static std::string cacheOff = gFs.config().opvTestCacheOffKeyword;
	if( !cacheOn.empty() && strstr( path, cacheOn.c_str() ) != 0 ) {
		setCache(true);
	} else if( !cacheOff.empty() && strstr( path, cacheOff.c_str() ) != 0 ) {
		setCache(false);
	}
}

int CdmiFuseOpLinux::read( const char* opPath, char* buf, size_t bufSize, off_t offset, struct fuse_file_info *info )
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(read, "reading offset[%ld + %zu]bytes by fd[%llu]"),
			offset, bufSize, info->fh);
	StopWatch ts;
	opvTestOnCache( opPath );

	std::string contentType,location;
	uint32 bufLen = (uint32)bufSize;
	bool disableCache = mFs.isFileDisableCache( info->fh );


	CdmiRetCode retCode = nonCdmi_ReadDataObject( toUrl(opPath), contentType, location, offset, bufLen, buf, disableCache );
	if( retCode == cdmirc_InvalidRange )
	{
		retCode = cdmirc_OK;
		bufLen = 0;
	}

	ZQ::common::CRC32 crc32;
	crc32.update(buf, bufLen);

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(read, "has read offset[%ld +%u]bytes by fd[%llu]: retCode [%s], took [%lld]us crc32[0x%08X]"),
			offset, bufLen, info->fh, cdmiRetStr(retCode), ts.stop(), crc32.get());
	
	if( CdmiRet_SUCC(retCode ) )
		return bufLen;

	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::write( const char* opPath, const char* buf, size_t size, off_t offset, struct fuse_file_info *info )
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(write, "writing offset[%ld +%zu]bytes by fd[%llu]"),
			offset, size, info->fh);
	StopWatch ts;
	opvTestOnCache(opPath);
	bool disableCache = mFs.isFileDisableCache( info->fh );
	mFs.setDirty( info->fh );
	std::string location;
	CdmiRetCode	retCode = nonCdmi_UpdateDataObject( toUrl( opPath), location, "application/octet-stream", offset, size, buf, -1, false, disableCache);

	MLOG(ZQ::common::Log::L_INFO,LOGFMT(write, "wrote offset[%ld +%zu]bytes: fd[%llu], retCode[%s], took [%lld]us"),
			offset, size, info->fh, cdmiRetStr(retCode), ts.stop());
	if (opPath && CdmiRet_SUCC(retCode))
		uncacheFileInfo(opPath);

	if( !CdmiRet_SUCC(retCode) )
		return cdmiCodeToSysCode( retCode );

	return size;
}

int CdmiFuseOpLinux::statfs(const char* opPath, struct statvfs * st )
{
	if(!st)	return -1;
	int64 freeBytes = 10LL * 1024 * 1024 * 1024;
	int64 totalBytes = 10LL * 1024 * 1024 * 1024;
	
	getDiskSpace(freeBytes, totalBytes);

	st->f_bsize		= 512; // 4096;
	st->f_frsize	= 512;
	st->f_blocks	= totalBytes/st->f_bsize;
	st->f_bfree		= freeBytes /st->f_bsize;
	st->f_bavail	= freeBytes /st->f_bsize;
	st->f_files		= 10 * 1024 * 1024;
	st->f_ffree		= 9 * 1024 * 1024;
	st->f_namemax	= 1 * 1024;

	MLOG(ZQ::common::Log::L_INFO,LOGFMT(statfs, "got[%ld] free bytes, [%ld] total bytes"),
			freeBytes,totalBytes);
	return 0;
}

int CdmiFuseOpLinux::flush( const char* opPath, struct fuse_file_info *info )
{
	CdmiRetCode ret = cdmirc_OK;
	fuse_context* ctx = fuse_get_context();
	if( info  &&  mFs.getAndClearDirty( info->fh ) )
	{
		MLOG(ZQ::common::Log::L_INFO, LOGFMT(flush, "caller[%d] fd[%d] dirty, flushing data"), (int)ctx->pid, (int)info->fh);
		ret = flushdata(toUrl(opPath));
	}

	MLOG(CdmiRet_SUCC(ret) ? ZQ::common::Log::L_INFO : ZQ::common::Log::L_ERROR, LOGFMT(flush, "caller[%d] fd[%d] got[%s(%d)]"), (int)ctx->pid, int(info? info->fh :-1), cdmiRetStr(ret), ret);
	return cdmiCodeToSysCode( ret);
}

int CdmiFuseOpLinux::release(  const char* opPath, struct fuse_file_info* info )
{
	bool hasEverDirty = false;
	if( info )
	{
		hasEverDirty = mFs.hasEverDirty(info->fh);
		mFs.removeFd( info->fh );
	}

	fuse_context* ctx = fuse_get_context();
	
	if (hasEverDirty)
		cdmi_IndicateClose(toUrl(opPath));

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(release, "caller[%d] fd[%d] released, indicated-server[%c]"), (int)ctx->pid, int(info? info->fh :-1), (hasEverDirty?'T':'F'));
	return 0;
}

int CdmiFuseOpLinux::fsync( const char* opPath, int fd, struct fuse_file_info *info )
{
	return flush(opPath, info);
}

int CdmiFuseOpLinux::setxattr( const char* opPath, const char* name, const char* value, size_t size, int flag )
{
	//FIXME: not implemented yet
	return 0;
}

int CdmiFuseOpLinux::getxattr( const char* opPath, const char* name, char* value, size_t bufSize )
{
	//FIXME: not implemented yet
	return 0;
}

int CdmiFuseOpLinux::listxattr( const char* opPath, char* list, size_t size )
{
	//FIXME: not implemented yet
	return 0;
}

int CdmiFuseOpLinux::removexattr( const char* opPath, const char* name )
{
	//FIXME: not implemented yet
	return 0;
}

int CdmiFuseOpLinux::opendir(const char* opPath, struct fuse_file_info* info )
{
	fuse_context* ctx = fuse_get_context();
	std::string dirPath = toUrl(opPath, true);
	CdmiRetCode retCode = CdmiFuseOps::openDir(dirPath);
	if (!CdmiRet_SUCC(retCode))
	{
		MLOG(ZQ::common::Log::L_ERROR, LOGFMT(opendir, "failed to opendir for caller[%d] due to [%s(%d)]"), (int)ctx->pid, cdmiRetStr(retCode), retCode);
		return cdmiCodeToSysCode( retCode );
	}

	info->fh = mFs.addFd(this, false, dirPath.c_str());
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(opendir, "dir opened: caller[%d] fd[%lld]"), (int)ctx->pid, info->fh);
	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::readdir( const char* opPath, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* info )
{
	fuse_context* ctx = fuse_get_context();
	std::string dirPath = mFs.pathOfFd(info->fh);
	// dirPath = toUrl(dirPath, true);

	StopWatch ts;
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(readdir, "reading dir[%s]"), dirPath.c_str());
	DirChildren dc;
	CdmiRetCode retCode = CdmiFuseOps::readDir(dirPath, dc);
	if (!CdmiRet_SUCC(retCode))
	{
		MLOG(ZQ::common::Log::L_ERROR, LOGFMT(readdir, "failed for caller[%d] due to [%s(%d)]"), (int)ctx->pid, cdmiRetStr(retCode), retCode);
		return cdmiCodeToSysCode( retCode );
	}

	size_t count =0;
	for(std::vector<std::string>::const_iterator it = dc._children.begin(); it < dc._children.end(); it ++ )
	{
		if (1 == filler(buf, it->c_str(), 0, 0))
		{
			MLOG(ZQ::common::Log::L_WARNING, LOGFMT(readdir, "buffer insufficient, break"));
			break;
		}

		count ++;
	}

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(readdir, "got [%zu] entries, took [%lld]us"), count, ts.stop());
	return 0;
}

/*
int CdmiFuseOpLinux::opendir(const char* opPath, struct fuse_file_info* info )
{
	fuse_context* ctx = fuse_get_context();

	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(opendir, "trying to open folder, caller[%d]"), (int)ctx->pid);
	DirChildren dcOld = getCachedChildren(dirPath);

	// step 1. read the container
	CdmiRetCode retCode = cdmirc_OK;
	int latencyReadContainer =0;
	std::string thisObjId;

	int64 stampStart = ZQ::common::now();
	uint32 yieldsec = 1;
	if (_fuseOpsConf.attrCache_childrenTTLsec >0)
	{
		yieldsec = _fuseOpsConf.attrCache_childrenTTLsec /60;
		if (yieldsec < 1)
			yieldsec =1;
		else if (yieldsec > 30)
			yieldsec = 30;
	}

	if (dcOld.stampLast > stampStart - yieldsec*1000)
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "readDir() container[%s] has been recently read within %dsec, skip reading"),  dirPath.c_str(), yieldsec);
	else
	{
		StopWatch ts;
		Json::Value result;
		std::string curPath = toUrl( opPath,true );
		CdmiRetCode retCode = cdmi_ReadContainer( result, curPath );
		if( !CdmiRet_SUCC ( retCode) )
		{
			MLOG(ZQ::common::Log::L_ERROR, LOGFMT(opendir, "failed to opendir for caller[%d] due to [%s(%d)]"), (int)ctx->pid, cdmiRetStr(retCode), retCode);
			return cdmiCodeToSysCode( retCode );
		}
	}

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(opendir, "cdmi query got[%s], caller[%d], took[%lld]us"), cdmiRetStr(retCode), (int)ctx->pid, ts.stop());
	std::string thisObjId;
	if (result.isMember("objectID"))
		thisObjId = result["objectID"].asString();

	//convert json value into dirent cache
#define BATCH_ATTR_READ

#ifdef BATCH_ATTR_READ
#	define DEFAULT_BATCH_NO   (2)
#	define MAX_FILE_PER_BATCH (1000)
#	define MIN_FILE_PER_BATCH (50)
#endif
	
	int readerId=0;
	int readOffset=0;
	StrList filesOfBatch;

	DirChildren dcOld = getCachedChildren(opPath);
	DirChildren dcNew;
	int cReaders =0;

	if ( result.isMember("children"))
	{
		Json::Value& children = result["children"];//should never fail
		size_t filesPerBatch = children.size() / (DEFAULT_BATCH_NO +1);
		if (filesPerBatch >MAX_FILE_PER_BATCH)
			filesPerBatch = MAX_FILE_PER_BATCH;
		if (filesPerBatch <MIN_FILE_PER_BATCH)
			filesPerBatch = MIN_FILE_PER_BATCH;

		do {
			if (!children.isArray())
				break;

			if (children.size() == dcOld._children.size()) // the result looks like as same as the previous info
			{
				MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(opendir, "the cached children(%d) looks good to take, skipping ChildReaders"), dcOld._children.size());
				break;
			}

			if ((_slowThrdPool.pendingRequestSize() >_slowThrdPool.size() *9) && dcOld._children.size() >0)
			{
				MLOG(ZQ::common::Log::L_WARNING, LOGFMT(opendir, "too many ChildReaders queued %d, force to taking the cached children[%d]"), _slowThrdPool.pendingRequestSize(), dcOld._children.size());
				break;
			}

			MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(opendir, "path[%s] ignoring cached children(%d): expired or miss-matched the count(%d) of this query"), curPath.c_str(), dcOld._children.size(), children.size());

			for( size_t i = 0 ; i < children.size(); i ++ )
			{
				const Json::Value& v = children[i];
				if(!v.isString())
				{	
					MLOG(ZQ::common::Log::L_WARNING, LOGFMT(opendir, "got a non-string child entry, ignored"));
					continue;
				}

				std::string name = v.asString();
				if(name.length() > 0 && name.at(name.length()-1) == '/')
					name = name.substr(0,name.length()-1);
				// dirCache.push_back( name );
				dcNew._children.push_back(name);

				if(0 == _fuseOpsConf.attrCache_byList)
					continue;

				std::string fullpath = curPath + name;
				filesOfBatch.push_back( fullpath );
				if (filesOfBatch.size() > filesPerBatch )
				{
					//issue read ahead for file attribute cache
					ChildReader* pReader = new ChildReader(*this, ++readerId, thisObjId, opPath, filesOfBatch, 
						"", readOffset, readOffset + filesOfBatch.size() -1);
					pReader->start(); cReaders++;
					readOffset += filesOfBatch.size();
					filesOfBatch.clear();
				}
			}

			if (0 == _fuseOpsConf.attrCache_byList)
			{
				// only take the new children list but skip ChildReaders
				MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(opendir, "path[%s] skipping ChildReaders for %d children per attrCache_byList[0]"), curPath.c_str(), dcNew._children.size());
				break;
			}
			
			// the last ChildReader if necessary
			if (filesOfBatch.size() >0)
			{
				ChildReader* pReader = new ChildReader(*this, ++readerId, thisObjId, opPath, filesOfBatch, 
					"", readOffset, readOffset + filesOfBatch.size() -1);

				pReader->start(); cReaders++;
				readOffset += filesOfBatch.size();
				filesOfBatch.clear();
			}

		} while(0);
	}

	info->fh = mFs.addFd(this, false, opPath);
	bool bCached =false;
	size_t cNewChildren = dcNew._children.size();
	
	if (cNewChildren >0)
	{
		cacheChildren(opPath, dcNew);
		// mFs.putDirCache( info->fh, dcNew._children );
		bCached = true;
	}
	else cNewChildren = dcOld._children.size();
	// else mFs.putDirCache( info->fh, dcOld._children );
	
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(opendir, "dir opened, fd[%lld] children newly cached[%c] size[%d], %d readers issued"), info->fh, bCached?'T':'F', cNewChildren, cReaders);
	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::readdir( const char* opPath, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* info )
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(readdir, "reading dir content"));
	StopWatch ts;

	size_t count = 0;
	DirChildren children = getCachedChildren(mFs.pathOfFd(info->fh));
	for(std::vector<std::string>::const_iterator it = children._children.begin(); it < children._children.end(); it ++ )
	{
		if (1 == filler(buf, it->c_str(), 0, 0))
		{
			MLOG(ZQ::common::Log::L_WARNING, LOGFMT(readdir, "buffer insufficient, break"));
			break;
		}

		count ++;
	}

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(readdir, "got [%zu] entries, took [%lld]us"), count, ts.stop());
	return 0;
}
*/

int CdmiFuseOpLinux::releasedir( const char* opPath, struct fuse_file_info *info )
{
	// mFs.removeDirCache( info->fh );
	//if (opPath)
	//	uncacheChildren(opPath);
	fuse_context* ctx = fuse_get_context();
	mFs.removeFd(info->fh);
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(releasedir, "caller[%d] fd[%lld] removed"), (int)ctx->pid, info->fh);
	return 0;
}

int CdmiFuseOpLinux::fsyncdir( const char* opPath, int fd, struct fuse_file_info *info )
{
	return 0;
}

int CdmiFuseOpLinux::access( const char* opPath, int mode  )
{
	//FIXME: not implemented
	return 0;
}

int CdmiFuseOpLinux::create( const char* opPath, mode_t mode, struct fuse_file_info* info )
{
	return open(opPath, info );
}

int CdmiFuseOpLinux::ftruncate( const char* opPath, off_t offset, struct fuse_file_info *info )
{
	return truncate( opPath,offset);	
}

int CdmiFuseOpLinux::fgetattr( const char* opPath,struct stat*st, struct fuse_file_info *info )
{
	return getattr( opPath, st );
}

int CdmiFuseOpLinux::lock(  const char* opPath,int cmd, struct flock* lk, struct fuse_file_info* info )
{
	//not supported
	return -1;
}

int CdmiFuseOpLinux::utimens( const char* opPath, const struct timespec tv[2] )
{
	//do nothing
	return 0;
}

int	CdmiFuseOpLinux::ioctl( const char* opPath, int cmd, void* arg, struct fuse_file_info* info,unsigned int flag, void* data )
{
	if( _IOC_TYPE(cmd) !=  CDMIFUSE_IOCTL_BASECODE )
	{
//		struct fuse_context* ctx = 	fuse_get_context();
//		MLOG(ZQ::common::Log::L_WARNING,LOGFMT(ioctl, "comes a unknown cmd [%d] [%d] from caller[%d]"),cmd, _IOC_TYPE(cmd),
//				ctx->pid);
		return -ENOTTY;
	}
	
	size_t paraSize = _IOC_SIZE(cmd);
	int cmdNo = _IOC_NR(cmd);
	
	if(!data || paraSize <= 0 )
	{
		MLOG(ZQ::common::Log::L_WARNING,LOGFMT(ioctl, "no data parameter passed in cdmifuse_ioctl"));
		return -EINVAL;
	}

	switch( cmdNo ) {
		case ( CDMIFUSE_IOCTL_CODE_CLONE_FILE ): 
			{//full clone or fast clone
				if( paraSize < sizeof(CDMIFUSE_CLONE_REQ ) ) {
					MLOG(ZQ::common::Log::L_WARNING,LOGFMT(ioctl, "CDMIFUSE_CLONE_REQ cmd with incompleted data passed in"));
					return -EINVAL;
				}

				CDMIFUSE_CLONE_REQ* req = reinterpret_cast<CDMIFUSE_CLONE_REQ*>(data);
				bool fastClone = (req->cloneFlags & 0x1) != 0;
				std::string filename;
				filename.assign(req->destPath,CDMIFUSE_CLONE_MAX_PATH);
				MLOG(ZQ::common::Log::L_INFO, LOGFMT(ioctl, "%s clone of file[%s]"), fastClone ? "fast ":"", filename.c_str());
				return cdmi_clone( opPath, filename, fastClone );
			}

			break;

		case CDMIFUSE_IOCTL_CODE_STATUS_QEURY:
			{//query clone status
				if( paraSize < sizeof(CDMIFUSE_CLONE_STATUS)) {
					MLOG(ZQ::common::Log::L_WARNING,LOGFMT(ioctl, "CDMIFUSE_CMD_CLONE_STATUS cmd with incompleted data passed in"));
					return -EINVAL;
				}

				CDMIFUSE_CLONE_STATUS* status = reinterpret_cast<CDMIFUSE_CLONE_STATUS*>(data);
				MLOG(ZQ::common::Log::L_DEBUG,LOGFMT(ioctl, "query clone status"));				
				return cdmi_status_clone(opPath, status->percentCompleted, status->status);
			}
			break;

		case CDMIFUSE_IOCTL_CODE_QOS:
			{//QOS 
				if( paraSize < sizeof( CDMIFUSE_QoS_PARAMS ) ) {
					MLOG(ZQ::common::Log::L_WARNING,LOGFMT(ioctl, "CDMIFUSE_QoS_PARAMS cmd with incompleted data passed in"));
					return -EINVAL;
				}
				// refer to CDMI v1.0.2.pdf 16.4 metadata cdmi_throughput
				CDMIFUSE_QoS_PARAMS* qos = reinterpret_cast<CDMIFUSE_QoS_PARAMS*>(data);
				MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(ioctl, "QoS get the throughputKBps[%d]"), qos->throughputKBps);
				return cdmi_throughput(opPath, qos->throughputKBps);
				//return -ENOSYS;
			}
			break;

		case CDMIFUSE_IOCTL_CODE_CACHE_SETTING:
			{
				char* p = reinterpret_cast<char*>(data);
				MLOG(ZQ::common::Log::L_DEBUG,LOGFMT(ioctl, "cache setting: %s"), *p == 0 ?"disable":"enable");
				return cdmi_cacheSetting( *p != 0 );
			}
			break;

		default:
			{
				MLOG(ZQ::common::Log::L_WARNING,LOGFMT(ioctl, "matched magic number but unknown command code [%d]"),cmd);
				return -ENOTTY;
				break;
			}
	}

	return 0;
}

int CdmiFuseOpLinux::cdmi_cacheSetting( bool enable ) {
	setCache(enable);
	return 0;
}

int CdmiFuseOpLinux::cdmi_throughput(const char* opPath, uint32 kbSize)
{
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(cdmi_throughput, "setting limitation [%d]Kbps"), kbSize);
	//std::string mimetype = "text/plain";
	Json::Value result;
	Properties metadata;
	uint32 mParaSize = kbSize * 1024;
	char buffer[32];
	memset(buffer, '\0', 32);
	snprintf(buffer, 32, "%d", mParaSize);
	metadata["cdmi_throughput"] = std::string(buffer);
	std::string url = toUrl( std::string(opPath));
	CdmiRetCode retCode = cdmi_CreateDataObject(result, url, "", metadata);
	if( !CdmiRet_SUCC(retCode) )
		MLOG(ZQ::common::Log::L_ERROR, LOGFMT(cdmi_throughput, "cdmi_CreateDataObject failed with error[%s(%d)]"), cdmiRetStr(retCode), retCode);
	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::cdmi_clone( const char* opPath, const std::string& copyTo, bool fastClone ) {
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(cdmi_clone, "copy %s to %s"),opPath, copyTo.c_str() );
	StopWatch ts;
	Json::Value result;
	std::string srcPath = toUrl( opPath );
	if(srcPath.length() >0 && srcPath.at(0) != '/')
		srcPath = std::string("/") + srcPath;
	CdmiRetCode retCode = cdmi_CreateDataObject( result, toUrl(copyTo), "", Properties(), "", StrList(), "", "", "", srcPath, "", "", "", fastClone);
	MLOG(ZQ::common::Log::L_INFO, LOGFMT(cdmi_clone, "cdmi_CreateDataObject %s from %s to %s, got [%s], took [%lld]us"),
			fastClone ? "fast-clone":"copy", opPath, copyTo.c_str(), cdmiRetStr( retCode), ts.stop() )	;
	return cdmiCodeToSysCode( retCode );
}

int CdmiFuseOpLinux::cdmi_status_clone( const char* opPath, unsigned char& percent, unsigned short& status ) {
	percent = 0;
	status = CDMIFUSE_CLONE_STATUS_ERROR;
	std::string strStatus;
	MLOG(ZQ::common::Log::L_DEBUG, LOGFMT(cdmi_status_clone, "query status of file"));
	StopWatch ts;
	Json::Value result;
	std::string location;
	std::string url = toUrl( std::string(opPath)) + "?completionStatus;metadata:percentComplete";
	CdmiRetCode retCode = cdmi_ReadDataObject(result, url, location  );
	if( CdmiRet_SUCC(retCode) ) {
		if(!result.isMember("metadata")) {
			MLOG(ZQ::common::Log::L_ERROR, LOGFMT(cdmi_status_clone, "bad status response, no 'metadata' is found"));
			return -EIO;
		}
		Json::Value& md = result["metadata"];
		if(!md.isMember("percentComplete")) {
			MLOG(ZQ::common::Log::L_WARNING, LOGFMT(cdmi_status_clone, "incompleted status response, no 'percentComplete' is found, take percentComplete=0"));
			percent = 0;
		} else {
			const Json::Value& pc = md["percentComplete"];
			if( pc.isIntegral()) {
				percent = pc.asInt();
			} else if( pc.isString()  ){
				std::string strPc = pc.asString();
				percent = strPc.length() > 0 ? atoi(strPc.c_str()):0;
			} else {
				MLOG(ZQ::common::Log::L_WARNING, LOGFMT(cdmi_status_clone, "bad status response, 'percentComplete' found but with illegal type"));
				percent = 0;
			}
		}

		if(result.isMember("completionStatus")) {
			strStatus = result["completionStatus"].asString();
			if( strcasecmp( strStatus.c_str(), "Processing") == 0 )  {
				status = CDMIFUSE_CLONE_STATUS_PROCESSING;
			} else if( 0 ==strcasecmp( strStatus.c_str(), "Complete") ) {
				status = CDMIFUSE_CLONE_STATUS_COMPLETED;
				if (percent <=0)
					percent = 100;
			}
		}
		if (percent >100)
			percent = 100;
	}

	MLOG(ZQ::common::Log::L_INFO, LOGFMT(cdmi_status_clone, "clone status gotten [%d%%/%s], cdmiReturn[%s], took[%lld]us"),
			percent, strStatus.c_str(), cdmiRetStr(retCode), ts.stop());
	return cdmiCodeToSysCode( retCode );
}

FuseConfig::FuseConfig()
	:logLevel(6),
	logCount(5),
	logFlushInterval(2000),
	curlThrdPoolSize(10),
	useMirrorFuse(0),
	fuse_argc(0),
	fuse_argv(0)
{
}

FuseConfig::~FuseConfig()
{
//	clear();
}

void	FuseConfig::clear()
{
	if( fuse_argc && fuse_argv )
	{
		delete[] fuse_argv;
		fuse_argc = 0;
		fuse_argv = 0;
	}
}

bool FuseConfig::parseFuseSetting(bool foreground )
{
	// "-o nonempty -o allow_other -o hard_remove -o no_remote_lock -o uid=0 -o gid=0 -o big_writes -o attr_timeout=5 -o direct_io /mnt/fuse"
	// the following are enabled by default according to tests
	// nonempty, hard_remove, big_writes, attr_timeout=5
	if (std::string::npos == fuse_setting.find("nonempty"))
		fuse_setting += " -o nonempty";
	if (std::string::npos == fuse_setting.find("hard_remove"))
		fuse_setting += " -o hard_remove";
	if (std::string::npos == fuse_setting.find("no_remote_lock"))
		fuse_setting += " -o no_remote_lock";
	if (std::string::npos == fuse_setting.find("big_writes"))
		fuse_setting += " -o big_writes";
	if (std::string::npos == fuse_setting.find("attr_timeout"))
		fuse_setting += " -o attr_timeout=5";
	// if (std::string::npos == fuse_setting.find("direct_io"))
	// 	fuse_setting += " -o direct_io";

	clear();
	fuse_setting_vec.clear();
	ZQ::common::stringHelper::SplitString(fuse_setting, fuse_setting_vec, " ", " \t\v\r\n");
	if( fuse_setting_vec.empty() )
		return false;

	if (foreground)
	{
		fuse_argc++;
		fuse_setting_vec.insert(fuse_setting_vec.begin(), "-f");
	}

	fuse_setting_vec.insert(fuse_setting_vec.begin(), fs_name);

	fuse_argc = fuse_setting_vec.size();

	fuse_argv = new char*[fuse_argc];
	int i = 0;
	while (i<fuse_argc)
	{
		fuse_argv[i] = (char*)fuse_setting_vec[i].c_str();
		if( ( i + 1 ) == fuse_argc ) {
			fuse_mountpoint = fuse_setting_vec[i];
		}
		i++;
	}

	return true;
}

void	FuseConfig::readConfig( const std::string& key, const std::string& defaultValue, std::string& value )
{
	value = defaultValue;
	if( !confRoot.isMember(key) )
		return;
	value = confRoot[key].asString();
}

void	FuseConfig::readConfig( const std::string& key, int32 defaultValue, int32& value )
{
	value = defaultValue;
	if( !confRoot.isMember(key) )
		return;
	value = (int32)parseInteger( confRoot[key] );
}

void FuseConfig::readConfig( const std::string& key, size_t defaultValue, size_t& value )
{
	value = defaultValue;
	if( !confRoot.isMember(key) )
		return;
	value = (size_t) parseInteger( confRoot[key] );
}

void	FuseConfig::readConfig( const std::string& key, int64 defaultValue, int64& value )
{
	value = defaultValue;
	if( !confRoot.isMember(key) )
		return;
	value = parseInteger( confRoot[key] );
}

void	FuseConfig::readConfig( const std::string& key, double defaultValue, double& value )
{
	value = defaultValue;
	if( !confRoot.isMember(key) )
		return;
	value = confRoot[key].asDouble();
}

int64	FuseConfig::parseInteger( const  Json::Value& val ) const  {
	if( val.isIntegral() ) {
		return val.asInt64();
	} else if (val.isString() ) {
		std::string strtmp = val.asString();
		std::string::size_type pos = strtmp.find("0x");
		if( pos == std::string::npos ) {
			return atoll( strtmp.c_str());
		} else {
			return strtoll( strtmp.substr( pos + 2 ).c_str(), NULL, 16 );
		}
	}

	return int64(-1);
}

int32 FuseConfig::readSize( const std::string& str ) const {
	if(str.empty())
		return 0;
	int32 factor = 1;
	int32 result = atoi(str.c_str());
	switch(str.at(str.length()-1))
	{
		case 'b':
		case 'B':
			factor = 1; break;
		case 'k':
		case 'K':
			factor = 1024;break;
		case 'm':
		case 'M':
			factor = 1024 * 1024; break;
		case 'g':
		case 'G':
			factor = 1024 * 1024 * 1024; break;
		default:
			factor = 1; break;
	}
	result *= factor;
	return result;
}

std::string normalizeDumpPath( const std::string& path ) {
	std::string dumpPath = "/";
	if(path.empty()){
		return dumpPath;
	}

	struct stat st;
	if(lstat(path.c_str(),&st) != 0 ) {
		if( mkdir(path.c_str(),0644) != 0 ){
			return dumpPath;
		}
	} else if( !S_ISDIR(st.st_mode) ) {
		return dumpPath;
	}
	dumpPath = path;
	return dumpPath;
}

bool	FuseConfig::readAllConfig( bool foreground )
{
	std::string tmpBufferSize;
	std::string tmpWriteBufferSize;
	std::string tmpLogSize;
	std::string tmpMaxWriteLength;
	std::string tmpMinWriteLength;

	readConfig("dump_path",  "", dump_path);

	readConfig("log_path",   "", logFilePath);
	readConfig("log_level",   6, logLevel);
	readConfig("log_count",   5, logCount);
	readConfig("log_size", "10m", tmpLogSize);


	readConfig("mirror_fuse",  0, useMirrorFuse);
	readConfig("mirror_folder", "",mirrorFolder);
	readConfig("mirror_sleepFactor",1000,sleepFactor);

	readConfig("cache",1,fuseOpsConf.enableCache);
	readConfig("cache_buffers",100,fuseOpsConf.tankConf.readBlockCount);
	readConfig("cache_buffers_forwrite",50,fuseOpsConf.tankConf.writeBlockCount);
	readConfig("cache_buffersize", "128k",tmpBufferSize);
	readConfig("cache_buffersize_forwrite", "2m",tmpWriteBufferSize);
	readConfig("cache_logFlag",0,fuseOpsConf.tankConf.logFlag);
	readConfig("cache_flushThreads",10,fuseOpsConf.tankConf.flushThreadPoolSize);
	readConfig("cache_forceFlushInterval",5000,fuseOpsConf.tankConf.bufferInvalidationInterval);
	readConfig("cache_readBuffer_maxLinger",300000,fuseOpsConf.tankConf.cacheInvalidationInterval);
	readConfig("cache_readAheadMax",32,fuseOpsConf.tankConf.readAheadCount);
	readConfig("cache_readAheadTrigger",4,fuseOpsConf.tankConf.readAheadThreshold);
	readConfig("cache_readAhead_powerBase",4,fuseOpsConf.tankConf.readAheadIncreamentLogBase);
	readConfig("cache_readAhead_recognitions",100, fuseOpsConf.tankConf.mergableArrayMaxItemSize);
	readConfig("cache_readAfterFlushDirty",1, fuseOpsConf.tankConf.readAfterFlushDirty);
	readConfig("cache_writeSegments_max",10,fuseOpsConf.tankConf.maxWriteQueueMergeItemCount);
	readConfig("cache_writeLength_max", "4m",tmpMaxWriteLength);
	readConfig("cache_writeLength_min", "16k",tmpMinWriteLength);
	readConfig("cache_writeQueue_idle",50,fuseOpsConf.tankConf.writeQueueIdleInMs );
	readConfig("cache_partitions",10,fuseOpsConf.tankConf.partitionCount);
	readConfig("disable_filecache_flag",0,flagDisableCache);
	readConfig("cache_writeThreadsOfYield",0,fuseOpsConf.tankConf.writeThreadsOfYield);
	readConfig("cache_writeYieldMax",1000,fuseOpsConf.tankConf.writeYieldMax);
	readConfig("cache_writeYieldMin",20,fuseOpsConf.tankConf.writeYieldMin);
	readConfig("cache_writeYieldWinSize",3,fuseOpsConf.tankConf.writeAvgWinSizeForYield);
	readConfig("cache_writeBufferOfYield",50,fuseOpsConf.tankConf.writeBufferCountOfYield);
	readConfig("attrCache_TTL",         CACHED_FILE_INFO_TTL_SEC, fuseOpsConf.attrCache_TTLsec);
	readConfig("attrCache_childrenTTL", CACHED_FILE_INFO_TTL_SEC, fuseOpsConf.attrCache_childrenTTLsec);
	readConfig("attrCache_size", 10 * 1000, fuseOpsConf.attrCache_size);
	readConfig("attrCache_byList", 1, fuseOpsConf.attrCache_byList);

	fuseOpsConf.tankConf.mergableArrayMaxItemSize = MIN( fuseOpsConf.tankConf.mergableArrayMaxItemSize, 1000);
	fuseOpsConf.tankConf.maxWriteQueueMergeItemCount = MIN( fuseOpsConf.tankConf.maxWriteQueueMergeItemCount, 20 );

	fuseOpsConf.tankConf.writeBufferCountOfYield = MIN(100, MAX(0,fuseOpsConf.tankConf.writeBufferCountOfYield));
	
	if( fuseOpsConf.tankConf.readAheadIncreamentLogBase < 2 )
		fuseOpsConf.tankConf.readAheadIncreamentLogBase = 2;

	if( fuseOpsConf.tankConf.readBlockCount < 2 )	fuseOpsConf.tankConf.readBlockCount = 2;
	if( fuseOpsConf.tankConf.readBlockCount > 1000* 1000 ) fuseOpsConf.tankConf.readBlockCount = 1000 * 1000;

	if( fuseOpsConf.tankConf.writeBlockCount < 2 )	fuseOpsConf.tankConf.writeBlockCount = 2;
	if( fuseOpsConf.tankConf.writeBlockCount > 1000* 1000 ) fuseOpsConf.tankConf.writeBlockCount = 1000 * 1000;

	fuseOpsConf.tankConf.readCacheBlockSize = readSize( tmpBufferSize );
	fuseOpsConf.tankConf.writeBufferBlockSize = readSize( tmpWriteBufferSize );
	logSize = readSize(tmpLogSize);

	fuseOpsConf.tankConf.maxWriteQueueBufferCount = readSize( tmpMaxWriteLength)/fuseOpsConf.tankConf.writeBufferBlockSize;
	fuseOpsConf.tankConf.minWriteQueueBufferCount = readSize( tmpMinWriteLength)/fuseOpsConf.tankConf.writeBufferBlockSize;

	if( fuseOpsConf.tankConf.readCacheBlockSize < 4096 )
		fuseOpsConf.tankConf.readCacheBlockSize = 4096;

	// childrenTTL should not be more than fileInfoTTL, otherwise will trigger many indivial file query when list dir
	if (fuseOpsConf.attrCache_childrenTTLsec > fuseOpsConf.attrCache_TTLsec)
		fuseOpsConf.attrCache_childrenTTLsec = fuseOpsConf.attrCache_TTLsec;
	if(fuseOpsConf.attrCache_size < 1000 ) {
		fuseOpsConf.attrCache_size = 1000;
	}

	if( logSize < 100 * 1024)
		logSize = 100 * 1024;

	readConfig("cdmi_baseurl",	"",	cdmiBaseUrl);
	readConfig("cdmi_homecontainer", "",cdmiHomeContainer);
	readConfig("cdmi_userdomain", "",cdmiUserDomain);

	readConfig("curl_threads",10, curlThrdPoolSize);
	readConfig("curl_bindIp","", curlBindIp);

	readConfig("fuse_setting", "",fuse_setting);
	readConfig("curl_connectTimeout",2000,connectTimeout);
	readConfig("curl_operationTimeout",5000,operationTimeout);
	readConfig("curl_retryTimeout",10000,retryTimeout);
	readConfig("flags",16,fuseOpsConf.fuseFlag);

	readConfig("aqua_del_before_mv",0,aqua_del_before_mv);

	//for opv test
	readConfig("cacheByKeyword",0,opvTestCacheByKeyword);
	readConfig("cacheOnKeyword", "",opvTestCacheOnKeyword);
	readConfig("cacheOffKeyword", "",opvTestCacheOffKeyword);

	if( opvTestCacheByKeyword >= 1)
		fuseOpsConf.enableCache = 1;

	if(!parseFuseSetting( foreground))
	{
		ZQ::common::SysLog logger("CdmiFuse");
		logger(ZQ::common::Log::L_ERROR, CLOGFMT(LinuxFuse, "no fuse setting is found"));
		return false;
	}

	if( logFilePath.empty() )
	{
		ZQ::common::SysLog logger("CdmiFuse");
		logger(ZQ::common::Log::L_ERROR, CLOGFMT(LinuxFuse, "config parsed but no log file path specified"));
		return false;
	}

	if(dump_path.empty()) {
		dump_path = logFilePath;
		char* strDumpPath = (char*)malloc( sizeof(char) * dump_path.length() + 20 );//for folder name crash
		strcpy(strDumpPath,dump_path.c_str());
		strDumpPath = dirname(strDumpPath);
		strcat(strDumpPath, "/crash");
		dump_path = strDumpPath;
		free(strDumpPath);
	}

	dump_path = normalizeDumpPath(dump_path);
	return true;
}

bool FuseConfig::loadConfig( const std::string& configPath, bool foreground )
{
	Json::Reader reader;
	std::ifstream ifile;
	ifile.open( configPath.c_str() );
	if(!ifile.is_open())
	{//syslog here
		ZQ::common::SysLog logger("CdmiFuse");
		logger(ZQ::common::Log::L_ERROR, CLOGFMT(LinuxFuse, "failed to open config file [%s]"), configPath.c_str());
		return false;
	}

	if(!reader.parse(ifile,confRoot))
	{//syslog here
		ZQ::common::SysLog logger("CdmiFuse");
		logger(ZQ::common::Log::L_ERROR, CLOGFMT(LinuxFuse, "config file[%s] opened, but failed to parse: %s"),
			configPath.c_str(), reader.getFormatedErrorMessages().c_str() );
		return false;
	}

	return readAllConfig(foreground);
}

bool loadConfAndInit( FuseConfig& fConf, const char* confPath, struct fuse_operations& ops, int foreground )
{
	if( confPath == 0 || confPath[0] == 0 )
		confPath = "./cdmifuse.conf";
	if(!fConf.loadConfig( confPath, foreground >= 1) )
		return false;

	memset( &ops, 0, sizeof(ops) );
	if(fConf.useMirrorFuse >= 1 ) 
	{
		if(foreground>=1)
			setForeground();
		ops.getattr			=	mirror_getattr;
		ops.readlink		=	mirror_readlink;
		ops.mkdir			=	mirror_mkdir;
		ops.unlink			=	mirror_unlink;
		ops.rmdir			=	mirror_rmdir;
		ops.symlink			=	mirror_symlink;
		ops.rename			=	mirror_rename;
		ops.link			=	mirror_link;
		ops.chmod			=	mirror_chmod;
		ops.chown			=	mirror_chown;
		ops.truncate		=	mirror_truncate;
		ops.open			=	mirror_open;
		ops.read			=	mirror_read;
		ops.write			=	mirror_write;
		ops.statfs			=	mirror_statfs;
		ops.flush			=	mirror_flush;
		ops.release			=	mirror_release;
		ops.fsync			=	mirror_fsync;
		ops.setxattr		=	mirror_setxattr;
		ops.getxattr		=	mirror_getxattr;
		ops.listxattr		=	mirror_listxattr;
		ops.removexattr		=	mirror_removexattr;
		ops.opendir			=	mirror_opendir;
		ops.readdir			=	mirror_readdir;
		ops.releasedir		=	mirror_releasedir;
		ops.fsyncdir		=	mirror_fsyncdir;
		ops.init			=	mirror_init;
		ops.destroy			=	mirror_destroy;
		ops.access			=	mirror_access;
		ops.create			=	mirror_create;
		ops.ftruncate		=	mirror_ftruncate;
		ops.fgetattr		=	mirror_fgetattr;
		ops.lock			=	mirror_lock;
		ops.utimens			=	mirror_utimens;
		setDest(fConf.mirrorFolder);
		setConfPath( confPath );
	}
	else
	{
		ops.getattr			=	CdmiFuseFS::cdmi_getattr;
		ops.readlink		=	CdmiFuseFS::cdmi_readlink;
		ops.mkdir			=	CdmiFuseFS::cdmi_mkdir;
		ops.unlink			=	CdmiFuseFS::cdmi_unlink;
		ops.rmdir			=	CdmiFuseFS::cdmi_rmdir;
		ops.symlink			=	CdmiFuseFS::cdmi_symlink;
		ops.rename			=	CdmiFuseFS::cdmi_rename;
		ops.link			=	CdmiFuseFS::cdmi_link;
		ops.chmod			=	CdmiFuseFS::cdmi_chmod;
		ops.chown			=	CdmiFuseFS::cdmi_chown;
		ops.truncate		=	CdmiFuseFS::cdmi_truncate;
		ops.open			=	CdmiFuseFS::cdmi_open;
		ops.read			=	CdmiFuseFS::cdmi_read;
		ops.write			=	CdmiFuseFS::cdmi_write;
		ops.statfs			=	CdmiFuseFS::cdmi_statfs;
		ops.flush			=	CdmiFuseFS::cdmi_flush;
		ops.release			=	CdmiFuseFS::cdmi_release;
		ops.fsync			=	CdmiFuseFS::cdmi_fsync;
		ops.setxattr		=	CdmiFuseFS::cdmi_setxattr;
		ops.getxattr		=	CdmiFuseFS::cdmi_getxattr;
		ops.listxattr		=	CdmiFuseFS::cdmi_listxattr;
		ops.removexattr		=	CdmiFuseFS::cdmi_removexattr;
		ops.opendir			=	CdmiFuseFS::cdmi_opendir;
		ops.readdir			=	CdmiFuseFS::cdmi_readdir;
		ops.releasedir		=	CdmiFuseFS::cdmi_releasedir;
		ops.fsyncdir		=	CdmiFuseFS::cdmi_fsyncdir;
		ops.init			=	CdmiFuseFS::cdmi_init;
		ops.destroy			=	CdmiFuseFS::cdmi_destroy;
		ops.access			=	CdmiFuseFS::cdmi_access;
		ops.create			=	CdmiFuseFS::cdmi_create;
		ops.ftruncate		=	CdmiFuseFS::cdmi_ftruncate;
		ops.fgetattr		=	CdmiFuseFS::cdmi_fgetattr;
		ops.lock			=	CdmiFuseFS::cdmi_lock;
		ops.utimens			=	CdmiFuseFS::cdmi_utimens;
#if (FUSE_MAJOR_VERSION==2 && FUSE_MINOR_VERSION >=8) || FUSE_MAJOR_VERSION > 2
		ops.ioctl			=	CdmiFuseFS::cdmi_ioctl;
#endif
	}

	return true;
}

static void  signal_action(int signum, siginfo_t  *info, void *data) {
	exit(0);

	/*
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_handler=SIG_DFL;
	sigaction( signum, &act, 0);
	raise(signum);
	*/
}

int main( int argc, char* argv[] )
{
	int foreground = 0;
	if(argc>=3) {
		foreground = atoi(argv[2]);
	}
	struct fuse_operations	fuseOperation;
	FuseConfig fConfig;
	const char* confPath = 0;
	if( argc > 1)
		confPath = argv[1];

	if(!loadConfAndInit(fConfig,confPath,fuseOperation, foreground ))
		return -1;
	
	std::string umount_cmdline = "fusermount -u -z " + fConfig.fuse_mountpoint;
	system(umount_cmdline.c_str());

	if(!gFs.initialize( fConfig ))
		return -1;

	int rc = fuse_main( fConfig.fuse_argc, fConfig.fuse_argv, &fuseOperation,0);
	
	return rc;
}

/*
==================================================== 
fuse_main() arguments:
====================================================
http://blog.woralelandia.com/2012/07/16/fuse-mount-options/

general options:
-o opt,[opt...]        mount options
-h   --help            print help
-V   --version         print version

FUSE options:
-d   -o debug          enable debug output (implies -f)
-f                     foreground operation
-s                     disable multi-threaded operation

-o allow_other         allow access to other users
-o allow_root          allow access to root
-o nonempty            allow mounts over non-empty file/dir
-o default_permissions enable permission checking by kernel
-o fsname=NAME         set filesystem name
-o subtype=NAME        set filesystem type
-o large_read          issue large read requests (2.4 only)
-o max_read=N          set maximum size of read requests

-o hard_remove         immediate removal (don't hide files)
-o use_ino             let filesystem set inode numbers
-o readdir_ino         try to fill in d_ino in readdir
-o direct_io           use direct I/O
-o kernel_cache        cache files in kernel
-o [no]auto_cache      enable caching based on modification times (off)
-o umask=M             set file permissions (octal)
-o uid=N               set file owner
-o gid=N               set file group
-o entry_timeout=T     cache timeout for names (1.0s)
-o negative_timeout=T  cache timeout for deleted names (0.0s)
-o attr_timeout=T      cache timeout for attributes (1.0s)
-o ac_attr_timeout=T   auto cache timeout for attributes (attr_timeout)
-o intr                allow requests to be interrupted
-o intr_signal=NUM     signal to send on interrupt (10)
-o modules=M1[:M2...]  names of modules to push onto filesystem stack

-o max_write=N         set maximum size of write requests
-o max_readahead=N     set maximum readahead
-o async_read          perform reads asynchronously (default)
-o sync_read           perform reads synchronously
-o atomic_o_trunc      enable atomic open+truncate support
-o big_writes          enable larger than 4kB writes
-o no_remote_lock      disable remote file locking

Module options:

[iconv]
-o from_code=CHARSET   original encoding of file names (default: UTF-8)
-o to_code=CHARSET	    new encoding of the file names (default: UTF-8)

[subdir]
-o subdir=DIR	    prepend this directory to all paths (mandatory)
-o [no]rellinks	    transform absolute symlinks to relative
==================================================== */
// vim:ts=4:sw=4:noexpandtab:nu:

