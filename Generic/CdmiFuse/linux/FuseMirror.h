#ifndef __fuse_mirror_header_file__
#define __fuse_mirror_header_file__

#include <string>
#include <sys/uio.h>
#include <fuse/fuse.h>

int		mirror_getattr( const char* opPath, struct stat* st) ;
	

int		mirror_readlink( const char* opPath, char* linkpath, size_t bufSize ) ;
	

int		mirror_mkdir( const char* opPath, mode_t mode ) ;
	

int		mirror_unlink( const char* opPath ) ;
	


int		mirror_rmdir( const char* opPath ) ;
	


int		mirror_symlink(const char* opPath, const char* target) ;
	

int		mirror_rename( const char* opPath, const char* to ) ;
	

int		mirror_link( const char* opPath, const char* target ) ;
	


int		mirror_chmod( const char* opPath, mode_t mode ) ;
	


int		mirror_chown( const char* opPath,uid_t uid, gid_t gid) ;
	


int		mirror_truncate( const char* opPath, off_t offset ) ;
	

int		mirror_open( const char* opPath, struct fuse_file_info* info ) ;

int		mirror_read( const char* opPath, char* buf, size_t bufSize, off_t offset, struct fuse_file_info* info ) ;
int		mirror_read_direct( const char* opPath, char* buf, size_t bufSize, off_t offset, struct fuse_file_info* info ) ;
int		mirror_read_direct( const char* opPath, struct iovec* iov, int iovcount, off_t offset, struct fuse_file_info* info ) ;
	

int		mirror_write( const char* opPath, const char* buf, size_t size, off_t offset, struct fuse_file_info* info ) ;
int		mirror_write_direct( const char* opPath, const char* buf, size_t size, off_t offset, struct fuse_file_info* info ) ;
	


int		mirror_statfs( const char* opPath, struct statvfs* st) ;
	


int		mirror_flush( const char* opPath, struct fuse_file_info* info ) ;
	

int		mirror_release( const char* opPath, struct fuse_file_info* info  ) ;
	

int		mirror_fsync( const char* opPath, int fd, struct fuse_file_info *info  ) ;
	

int		mirror_setxattr( const char* opPath, const char* name, const char* value, size_t size, int flag ) ;
	

int		mirror_getxattr( const char* opPath, const char* name, char* value, size_t bufSize) ;
	

int		mirror_listxattr( const char* opPath, char* list, size_t size ) ;
	

int		mirror_removexattr( const char* opPath, const char* name ) ;
	

int		mirror_opendir( const char* opPath, struct fuse_file_info * info) ;

int		mirror_readdir( const char* opPath, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* info  ) ;

int		mirror_releasedir( const char* opPath, struct fuse_file_info* info  ) ;
	

int		mirror_fsyncdir( const char* opPath, int fd, struct fuse_file_info* info  ) ;
	

int		mirror_access( const char* opPath, int mode ) ;
	

int		mirror_create( const char* opPath, mode_t mode, struct fuse_file_info* info) ;

int		mirror_ftruncate( const char* opPath, off_t offset, struct fuse_file_info* info  ) ;
	

int		mirror_fgetattr( const char* opPath, struct stat* st, struct fuse_file_info* info ) ;
	

int		mirror_lock( const char* opPath, struct fuse_file_info* info, int cmd, struct flock* lk ) ;
	

int		mirror_utimens( const char* opPath, const struct timespec tv[2]) ;
	
void*	mirror_init(struct fuse_conn_info *conn);

void	mirror_destroy(void *);

void	setDest( const std::string& folder );
void	setConfPath( const std::string& confPath );
void	setForeground();

#endif//__fuse_mirror_header_file__

