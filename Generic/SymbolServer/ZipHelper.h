#ifndef __ZipHelper_H__
#define __ZipHelper_H__

#ifndef ZLIB_WINAPI
#define ZLIB_WINAPI
#endif
#include <zip.h>
#include <unzip.h>
#include <string>
#include <vector>

namespace ZipHelper {
struct Error {
    int code; // error code
    std::string desc; // error description
    std::string action; // failed action

    Error():code(0){}
    void clear() {
        code = 0;
        desc.clear();
        action.clear();
    }
};
class Output {
public:
    virtual int write(const char* data, size_t len) = 0;
};

class RawFileOutput:public Output {
public:
    RawFileOutput();
    ~RawFileOutput();
    bool open(const char* path);
    virtual int write(const char* data, size_t len);
    void close();
private:
    FILE* file_;
};
class ZipFileOutput:public Output {
public:
    ZipFileOutput();
    bool open(const char* path);
    // open new file in the zip file
    bool openInZip(const char* name);
    virtual int write(const char* data, size_t len);
    // close the current file in the zip
    void closeInZip();
    void close();
private:
    zipFile file_;
    bool openedInZip_;
};

class GzipFileOutput:public Output {
public:
    GzipFileOutput();
    ~GzipFileOutput();
    bool open(const char* path);
    virtual int write(const char* data, size_t len);
    void close();
private:
    gzFile file_;
};

typedef std::vector<std::string> FileList;
class UnzipFile {
public:
    UnzipFile();
    bool open(const char* path);
    bool list(FileList& files);
    bool extract(const char* name, Output& out);
    bool close();
    const Error& getLastError();
private:
    unzFile file_;
    Error lastError_;
};
bool extractFileFromZipFile(const char* zipFilePath, const char* fileName, Output& out, Error& e);
bool listFilesInZipFile(const char* zipFilePath, FileList& files, Error& e);
} // namespace ZipHelper
#endif
