#include "LocateSparseTsHeader.h"
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
    void close() {
        if(-1 != fd_) {
            ::close(fd_);
            fd_ = -1;
        }
    }
private:
    int fd_;
};
#else // ZQ_OS_LINUX
#define _FILE_OFFSET_BITS  64
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    void close() {
        if(-1 != fd_) {
            ::close(fd_);
            fd_ = -1;
        }
    }
private:
    int fd_;
};
#endif

// all packets are valid after the first valid one
static const int findFirstValidPacket(const char* startPacket, int nCount, int packetLen, char leadByte) {
    if(nCount <= 0) {
        return -1;
    }
#define IsValid(n) (*(startPacket + ((n) * packetLen)) == leadByte)
    // speed-dial
    if(IsValid(0)) { // all valid
        return 0;
    }
    if( 1 == nCount || !IsValid(nCount - 1)) { // all invalid
        return -1;
    } // nCount >= 2 now

    // search the first valid packet by taking dichotomy
    int i0 = 0; // last invalid one
    int i1 = nCount - 1; // first valid one
    while(i0 + 1 < i1) {
        // narrow the [i0..i1] by check the middle one
        int i = (i1 - i0) / 2 + i0;
        if(IsValid(i)) {
            i1 = i;
        } else {
            i0 = i;
        }
    }
    return i1;
}

int64 locateSparseTsHeader(const char* filename, int64 startOffset, int64 readBufferSize, int64 reservedTailSize, int64 packetSize, char packetLeadByte) {
    LargeFileReader f;
    if(!f.open(filename)) {
        return SPARSE_E_BADFILE;
    }
    int64 fileSize = f.length();
    if(fileSize < 0) {
        return SPARSE_E_IOFAULT;
    }
    // check the parameters
    if(startOffset < 0) {
        startOffset = fileSize + startOffset;
        if(startOffset < 0) {
            return SPARSE_E_INVAL;
        }
    }
    int64 endOffset = fileSize - reservedTailSize;
    if(startOffset > endOffset || endOffset > fileSize) {
        return SPARSE_E_INVAL;
    }
    // now: startOffset <= endOffset <= fileSize
    if(endOffset == startOffset) { // zero length checking field
        return endOffset;
    }
    int64 firstBlockSize = 0;
    int64 lastBlockSize = 0;
    int64 blockCount = 0;
    // align the block from the beginning of the file
    if(endOffset - startOffset > readBufferSize) {
        lastBlockSize = endOffset % readBufferSize;
        firstBlockSize = (endOffset - startOffset - lastBlockSize) % readBufferSize;
        blockCount = (endOffset - startOffset - firstBlockSize - lastBlockSize) / readBufferSize + 2;
        if(firstBlockSize == 0) {
            firstBlockSize = readBufferSize;
            blockCount--;
        }
        if(lastBlockSize == 0) {
            lastBlockSize = readBufferSize;
            blockCount--;
        }
    } else { // only one block
        firstBlockSize = endOffset - startOffset;
        lastBlockSize = firstBlockSize;
        blockCount = 1;
    }
    // prepare the buffer
    char* bufp = (char*)malloc(readBufferSize);
    if(NULL == bufp) {
        return SPARSE_E_NOMEM;
    }

    int64 firstHeaderOffset = -1;
    int i0 = -1;
    int i1 = blockCount;
    while(i0 + 1 < i1) {
        int i = (i1 - i0) / 2 + i0;
        // load block data from file
        int64 blockBeginOffset = 0;
        int64 blockSize = 0;
        if(i == 0) { // first block
            blockBeginOffset = startOffset;
            blockSize = firstBlockSize;
        } else if (i == blockCount -1) { // last block
            blockBeginOffset = endOffset - lastBlockSize;
            blockSize = lastBlockSize;
        } else {
            blockBeginOffset = startOffset + firstBlockSize + (i - 1) * readBufferSize;
            blockSize = readBufferSize;
        }
        if(blockSize != f.read(bufp, blockBeginOffset, blockSize)) {
            free(bufp);
            return SPARSE_E_IOFAULT;
        }
        // align the packet from endOffset
        int64 firstPacketOffset = (endOffset - blockBeginOffset) % packetSize;
        int64 alignedBlockSize = blockSize - firstPacketOffset;
        int64 packetCount = alignedBlockSize / packetSize + (alignedBlockSize % packetSize == 0 ? 0 : 1);
        int iValid = findFirstValidPacket(bufp + firstPacketOffset, packetCount, packetSize, packetLeadByte);
        if(iValid == 0) {
            i1 = i;
        } else if (iValid == -1) {
            i0 = i;
        } else { // got
            firstHeaderOffset = blockBeginOffset + firstPacketOffset + iValid * packetSize;
            break;
        }
    }
    if(firstHeaderOffset == -1) {
        if (i1 == blockCount) { // all invalid
            firstHeaderOffset = endOffset;
        } else { // i0 == 0
            firstHeaderOffset = startOffset + (endOffset - startOffset) % packetSize;
        }
    }
    free(bufp);
    return firstHeaderOffset;
}
