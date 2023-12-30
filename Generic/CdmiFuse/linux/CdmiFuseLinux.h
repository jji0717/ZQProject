#ifndef __cdmi_fuse_implementation_linux_version_header_file__
#define __cdmi_fuse_implementation_linux_version_header_file__

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fcntl.h>
#include "../CdmiFuseOps.h"

#define CDMIFUSE_IOCTL_CODE_CACHE_SETTING 51
#define CDMIFUSE_CMD_CACHE_SETTING _IOW(CDMIFUSE_IOCTL_BASECODE, CDMIFUSE_IOCTL_CODE_CACHE_SETTING, char)

class CdmiFuseFS;

class CdmiFuseOpLinux : public CdmiFuseOps
{
public:
	CdmiFuseOpLinux( ZQ::common::Log& log, 
			ZQ::common::NativeThreadPool& pool, 
			const std::string& cdmiRootUrl,
			const std::string& userDomain,
			const std::string& homeContainer,
			const std::string& opPath, 
			CdmiFuseFS& fs,
			const FuseOpsConf& opsConf,
		    long long id = -1);

	virtual ~CdmiFuseOpLinux();

	long long id() const { return mOpId; }

	void	updateTarget( const std::string& target )	{	}

	int		getattr( const char* opPath, struct stat* st);
	int		readlink( const char* opPath, char* linkpath, size_t bufSize );
	int		mkdir( const char* opPath, mode_t mode );
	int		unlink( const char* opPath );
	int		rmdir( const char* opPath );
	int		symlink(const char* opPath, const char* target);
	int		rename( const char* opPath, const char* to );
	int		link( const char* opPath, const char* target );// can we support this ?
	int		chmod( const char* opPath, mode_t mode );
	int		chown( const char* opPath,uid_t uid, gid_t gid);
	int		truncate( const char* opPath, off_t offset );
	int		open( const char* opPath, struct fuse_file_info* info );
	int		read( const char* opPath, char* buf, size_t bufSize, off_t offset, struct fuse_file_info* info = 0);
	int		write( const char* opPath, const char* buf, size_t size, off_t offset, struct fuse_file_info* info = 0);
	int		statfs( const char* opPath, struct statvfs* st);
	int		flush( const char* opPath, struct fuse_file_info* info = 0);
	int		release( const char* opPath, struct fuse_file_info* info = 0 );
	int		fsync( const char* opPath, int fd, struct fuse_file_info *info = 0 );
	int		setxattr( const char* opPath, const char* name, const char* value, size_t size, int flag );
	int		getxattr( const char* opPath, const char* name, char* value, size_t bufSize);
	int		listxattr( const char* opPath, char* list, size_t size );
	int		removexattr( const char* opPath, const char* name );
	int		opendir( const char* opPath, struct fuse_file_info * info);
	int		readdir( const char* opPath, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* info = 0 );
	int		releasedir( const char* opPath, struct fuse_file_info* info = 0 );
	int		fsyncdir( const char* opPath, int fd, struct fuse_file_info* info = 0 );
	int		access( const char* opPath, int mode );
	int		create( const char* opPath, mode_t mode, struct fuse_file_info* info);
	int		ftruncate( const char* opPath, off_t offset, struct fuse_file_info* info = 0 );
	int		fgetattr( const char* opPath, struct stat* st, struct fuse_file_info* info = 0);
	int		lock( const char* opPath, int cmd, struct flock* lk, struct fuse_file_info* info = 0 );
	int		utimens( const char* opPath, const struct timespec tv[2]);
	int		ioctl( const char* opPath, int cmd, void* arg, struct fuse_file_info* info,unsigned int flag, void* data );

private:
	void	opvTestOnCache( const char* opPath );
private:
	std::string dumpFileStat( struct stat& st ) const;
	bool	isTargetAFolder( const char* opPath, const char* target = 0 ) const;
	bool	isTargetAFolderCdmi( const char* opPath );
	int		cdmiCodeToSysCode( CdmiRetCode code ) const;
	std::string toUrl( const std::string& url, bool bContainer = false );

	int		cdmi_clone( const char* opPath, const std::string& copyTo, bool fastClone = false );
	int		cdmi_status_clone( const char* opPath , unsigned char& percent , unsigned short& status);
	int		cdmi_throughput(const char* opPath, uint32 kbSize);
	int		cdmi_cacheSetting( bool enable );

private:
	long long			mOpId;
	CdmiFuseFS&			mFs;
};

struct FuseConfig
{
public:
	FuseConfig();
	~FuseConfig();

	bool		loadConfig( const std::string& configPath,bool foreground );
	//fuse argument
	std::string		fuse_setting;
	std::string		fuse_mountpoint;

	std::string		dump_path;
	
	//log file config
	std::string		logFilePath;
	int32			logLevel;
	int32			logSize;
	int32			logCount;
	int32			logFlushInterval;

	int32			flagDisableCache;

	FuseOpsConf		fuseOpsConf;

	//cdmi server config
	std::string		cdmiBaseUrl;
	std::string		cdmiHomeContainer;
	std::string		cdmiUserDomain;
	//curl client config
	int32			curlThrdPoolSize;
	std::string		curlBindIp;

	int32			useMirrorFuse;
	std::string		mirrorFolder;
	int32			sleepFactor;

	int32			connectTimeout;
	int32			operationTimeout;
	int32			retryTimeout;

	int32			aqua_del_before_mv;


	//for OPV test	
	int32			opvTestCacheByKeyword;
	std::string		opvTestCacheOnKeyword;
	std::string		opvTestCacheOffKeyword;
	//

	int				fuse_argc;
	char**			fuse_argv;
private:
	void	readConfig( const std::string& key, const std::string& defaultValue, std::string& value );
	void	readConfig( const std::string& key, int32 defaultValue, int32& value );
	void	readConfig( const std::string& key, size_t defaultValue, size_t& value );
	void	readConfig( const std::string& key, int64 defaultValue, int64& value );
	void	readConfig( const std::string& key, double defaultValue, double& value );
	int64	parseInteger( const Json::Value& val ) const;

	bool	readAllConfig( bool foreground);
	bool	parseFuseSetting( bool foreground);
	void	clear();
	int32	readSize( const std::string& str ) const;

	Json::Value		confRoot;
	std::vector<std::string> fuse_setting_vec;
};

typedef struct _FileHandleInfo {
	CdmiFuseOpLinux*		ops;
	bool					disableCache;
	bool 					dirty;
	int64                   stampLastDirty;
	std::string             pathname;

	_FileHandleInfo() :ops(0),disableCache(false),dirty(false), stampLastDirty(0) {}

} FileHandleInfo;

class CdmiFuseFS
{
public:
	CdmiFuseFS();
	virtual ~CdmiFuseFS();

public:

	const FuseConfig& config() const{ return mConfig; }

	int64       addFd(CdmiFuseOpLinux* op, bool disableCache, const char* pathname);
	void		removeFd( int64 id );
	std::string pathOfFd( int64 fd );

	CdmiFuseOpLinux* getMainOp( const char* path );
	CdmiFuseOpLinux* getOp( long long id , const char* path = 0 );
	bool			 isFileDisableCache( long long id );
	void			 setDirty( long long id );
	bool 			 getDirty(long long id, bool bClear =false);
	bool 			 hasEverDirty(long long id);

#define getAndClearDirty(_FD)  getDirty(_FD, true)

	//void	putDirCache( long long int , const DIRENTVECTOR& cache );
	//DIRENTVECTOR getDirCache( long long int id );
	//void	removeDirCache( long long int id );

	bool	initialize( const FuseConfig& conf );

	bool	init_cdmi_fuse();
	void	uninit_cdmi_fuse();
	void	signal_caught( const char* sigstr, pid_t pid, uid_t uid);

public:

	static int cdmi_getattr(const char* path, struct stat *st);
	static int cdmi_readlink (const char *, char *, size_t);
	static int cdmi_mknod(const char *, mode_t, dev_t);
	static int cdmi_mkdir(const char *path, mode_t);
	static int cdmi_unlink(const char * path);
	static int cdmi_rmdir(const char * path );
	static int cdmi_symlink(const char *, const char *);
	static int cdmi_rename(const char * from, const char * to);
	static int cdmi_link(const char *, const char *);
	static int cdmi_chmod(const char *, mode_t);
	static int cdmi_chown(const char *, uid_t, gid_t);
	static int cdmi_truncate(const char *, off_t);
	static int cdmi_open(const char * path, struct fuse_file_info * info);
	static int cdmi_read(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * info);
	static int cdmi_write(const char * path, const char * buf, size_t size, off_t offset, struct fuse_file_info * info);
	static int cdmi_statfs(const char * path, struct statvfs *);
	static int cdmi_flush(const char *, struct fuse_file_info *);
	static int cdmi_release(const char *, struct fuse_file_info * info);
	static int cdmi_fsync(const char *, int, struct fuse_file_info *);
	static int cdmi_setxattr(const char *, const char *, const char *, size_t, int);
	static int cdmi_getxattr(const char *, const char *, char *, size_t);
	static int cdmi_listxattr(const char *, char *, size_t);
	static int cdmi_removexattr(const char *, const char *);
	static int cdmi_opendir(const char *path, struct fuse_file_info *);
	static int cdmi_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t, struct fuse_file_info *);
	static int cdmi_releasedir(const char *, struct fuse_file_info *);
	static int cdmi_fsyncdir(const char *, int, struct fuse_file_info *);
	static void *cdmi_init(struct fuse_conn_info *conn);
	static void cdmi_destroy(void *);
	static int cdmi_access(const char *path, int);
	static int cdmi_create(const char * path, mode_t, struct fuse_file_info * info);
	static int cdmi_ftruncate(const char *, off_t, struct fuse_file_info *);
	static int cdmi_fgetattr(const char *, struct stat *, struct fuse_file_info *);
	static int cdmi_lock(const char *, struct fuse_file_info *, int cmd, struct flock *);
	static int cdmi_utimens(const char *, const struct timespec tv[2]);
	static int cdmi_bmap(const char *, size_t blocksize, uint64_t *idx);
	static int cdmi_ioctl(const char *, int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data);
	static int cdmi_poll(const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp);

	std::string  getBindIp(){ return mCdmiBindIp; }

private:
	std::string						mMountPoint; //maybe no neccessary here
	ZQ::common::NativeThreadPool	*mThdPool;
	std::string						mCdmiServerRoot; //server root url
	std::string						mCdmiHomeContainer;
	std::string                     mCdmiBindIp;
	CdmiFuseOpLinux*				mMainOp;

	FuseConfig						mConfig;

	typedef std::map< int64, FileHandleInfo>	OPMAP;
	OPMAP							mOps;
	
	ZQ::common::Log*				mLogger;

	long long						mBaseOpId;
	ZQ::common::Mutex				mLocker;

	//DIRENTVECTOR					mDirentCache;
	//std::map<long long int, DIRENTVECTOR > mDirCaches;
};

#endif//__cdmi_fuse_implementation_linux_version_header_file__
//
// vim:ts=4:sw=4:noexpandtab:nu:

