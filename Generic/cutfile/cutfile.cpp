#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
class LargeFileReader {
public:
    LargeFileReader():fd_(-1) {}
    ~LargeFileReader() {
        close();
    }
    bool open(const char* name) {
        close();
        fd_ = ::open(name, _O_BINARY | _O_RDONLY);
        return -1 != fd_;
    }
    int64 length() {
        return _filelengthi64(fd_);
    }
    int read(char* buf, int64 pos, int count) {
        if(-1 == _lseeki64(fd_, pos, SEEK_SET)) {
            return -1;
        }
        return ::read(fd_, buf, count);
    }
    int64 seek(int64 pos) {
        return _lseeki64(fd_, pos, SEEK_SET);
    }
    void close() {
        if(-1 != fd_) {
            ::close(fd_);
            fd_ = -1;
        }
    }
public:
    int fd_;
};
#else // ZQ_OS_LINUX
#define _FILE_OFFSET_BITS  64
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
class LargeFileReader {
public:
    LargeFileReader():fd_(-1) {}
    ~LargeFileReader() {
        close();
    }
    bool open(const char* name) {
        close();
        fd_ = ::open(name, O_LARGEFILE | O_RDONLY);
        return -1 != fd_;
    }
    int64 length() {
        struct stat st;
        if(0 == fstat(fd_, &st)) {
            return st.st_size;
        } else {
            return -1;
        }
    }
    int read(char* buf, int64 pos, int count) {
        if(-1 == lseek(fd_, pos, SEEK_SET)) {
            return -1;
        }
        return ::read(fd_, buf, count);
    }
    int64 seek(int64 pos) {
        return lseek(fd_, pos, SEEK_SET);
    }
    void close() {
        if(-1 != fd_) {
            ::close(fd_);
            fd_ = -1;
        }
    }
public:
    int fd_;
};
#endif

int main(int argc, char* argv[]) {
    const char* name1 = NULL;
    const char* name2 = NULL;
    int64 begpos = 0;
    int64 len = 0;
    if(argc < 3) {
        printf("usage: cutfile <origfile> <targetfile> [begin_offset [length]]\n");
        return 0;
    }
    name1 = argv[1];
    name2 = argv[2];
    if(argc > 3) {
        begpos = _atoi64(argv[3]);
    }

    if(argc > 4) {
        len = _atoi64(argv[4]);
    }
    LargeFileReader f1;
    if(!f1.open(name1)) {
        printf("*ERROR* Can't open file %s\n", name1);
        return 0;
    }
    // tune the byte range
    int64 flen = f1.length();
	if (len<=0)
		len = flen - begpos;
    int64 endpos = begpos + len;
    if(begpos < 0 || begpos > flen || len <= 0 || endpos > flen) {
        printf("*ERROR* Requested range [%lld, %lld) can't be served. Valid range is [0, %lld)\n", begpos, endpos, flen);
        return 0;
    }

    FILE* f2 = fopen(name2, "wb");
    if(NULL == f2) {
        printf("*ERROR* Can't open file %s\n", name2);
        return 0;
    }

    char buf[4096];
    f1.seek(begpos);
    // copy
    int64 nowpos = begpos;
    while(nowpos < endpos) {
        int writesize = 0;
        if(endpos - nowpos > 4096) {
            writesize = 4096;
        } else {
            writesize = endpos - nowpos;
        }
        nowpos += writesize;
        if(-1 != ::read(f1.fd_, buf, writesize)) {
            ::fwrite(buf, 1, writesize, f2);
        } else {
            ::fclose(f2);
            printf("*ERROR* Can't read file %s at position %lld\n", name1, nowpos);
            return 0;
        }
    }
    printf("*DONE* Write %s [%lld, %lld) --> %s\n", name1, begpos, endpos, name2);
    ::fclose(f2);
    return 0;
}
