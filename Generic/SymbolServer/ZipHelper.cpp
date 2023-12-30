#include "ZipHelper.h"
namespace ZipHelper {
static const char* showZipError(int e) {
    static const char* ok = "OK";
    static const char* parameterError = "Parameter error";
    static const char* badZipFile = "Bad zip file";
    static const char* internalError = "Internal error";
    static const char* unknown = "Unknown error";
    switch(e) {
    case ZIP_OK:
        return ok;
    case ZIP_ERRNO:
        return strerror(errno);
    case ZIP_PARAMERROR:
        return parameterError;
    case ZIP_BADZIPFILE:
        return badZipFile;
    case ZIP_INTERNALERROR:
        return internalError;
    default:
        return unknown;
    }
}

static const char* showUnzipError(int e) {
    static const char* ok = "OK";
    static const char* endOfList = "End of list";
    static const char* parameterError = "Parameter error";
    static const char* badZipFile = "Bad zip file";
    static const char* internalError = "Internal error";
    static const char* crcError = "CRC error";
    static const char* unknown = "Unknown error";
    switch(e) {
    case UNZ_OK:
        return ok;
    case UNZ_END_OF_LIST_OF_FILE:
        return endOfList;
    case UNZ_ERRNO:
        return strerror(errno);
    case UNZ_PARAMERROR:
        return parameterError;
    case UNZ_BADZIPFILE:
        return badZipFile;
    case UNZ_INTERNALERROR:
        return internalError;
    case UNZ_CRCERROR:
        return crcError;
    default:
        return unknown;
    }
}

bool extractFileFromZipFile(const char* zipFilePath, const char* fileName, Output& out, Error& e) {
    e.clear();

    unzFile uf = unzOpen(zipFilePath);
    if(uf) {
        int stat = unzLocateFile(uf, fileName, 0);
        if(stat == UNZ_OK) {
            stat = unzOpenCurrentFile(uf);
            if(stat == UNZ_OK) {
                char buf[4096];
                size_t len = 4096;
                int nRead = 0;
                while((nRead = unzReadCurrentFile(uf, buf, len)) > 0 ) {
                    int nWrite = out.write(buf, nRead);
                    if(nWrite < nRead) {
                        // write error
                        e.code = -1;
                        e.desc = "Output error";
                        e.action = "Output.write";
                        break;
                    }
                }
                if(0 != e.code) { // write error
                } else if (nRead == UNZ_EOF) {
                    e.clear();
                } else { // read failed
                    e.code = nRead;
                    e.desc = showUnzipError(e.code);
                    e.action = "unzReadCurrentFile";
                }
                stat = unzCloseCurrentFile(uf);
                if(stat != UNZ_OK && 0 == e.code) {
                    e.code = stat;
                    e.desc = showUnzipError(stat);
                    e.action = "unzCloseCurrentFile";
                }
            } else { // open file in zip failed
                e.code = stat;
                e.desc = showUnzipError(stat);
                e.action = "unzOpenCurrentFile";
            }
        } else { // not care the UNZ_END_OF_LIST_OF_FILE
            e.code = stat;
            e.desc = showUnzipError(stat);
            e.action = "unzLocateFile";
        }
        stat = unzClose(uf);
        if(UNZ_OK != stat && 0 == e.code) {
            e.code = stat;
            e.desc = showUnzipError(stat);
            e.action = "unzClose";
        }
    } else { // fail to open zip file
        e.code = -1;
        e.desc = "Invalid zip file";
        e.action = "unzOpen";
    }
    return e.code == 0;
}
bool listFilesInZipFile(const char* zipFilePath, FileList& files, Error& e) {
    e.clear();
    files.clear();

    unzFile uf = unzOpen(zipFilePath);
    if(uf) {
        int stat = unzGoToFirstFile(uf);
        while(stat == UNZ_OK) {
            char name[1280];
            unz_file_info info;
            stat = unzGetCurrentFileInfo(uf, &info, name, 1280, NULL, 0, NULL, 0);
            if(stat == UNZ_OK) {
                files.push_back(name);
            } else {
                e.code = stat;
                e.desc = showUnzipError(stat);
                e.action = "unzGetCurrentFileInfo";
                break;
            }
            stat = unzGoToNextFile(uf);
        }
        if(UNZ_END_OF_LIST_OF_FILE != stat && 0 == e.code) {
            e.code = stat;
            e.desc = showUnzipError(stat);
            e.action = files.empty() ? "unzGoToFirstFile" : "unzGoToNextFile";
        }
        stat = unzClose(uf);
        if(UNZ_OK != stat && 0 == e.code) {
            e.code = stat;
            e.desc = showUnzipError(stat);
            e.action = "unzClose";
        }
    } else { // fail to open zip file
        e.code = -1;
        e.desc = "Invalid zip file";
        e.action = "unzOpen";
    }
    return e.code == 0;
}
UnzipFile::UnzipFile():file_(NULL){}
bool UnzipFile::open(const char* path) {
    close();
    file_ = unzOpen(path);
    if(file_) {
        lastError_.clear();
        return true;
    } else {
        lastError_.code = -1;
        lastError_.desc = "Invalid zip file";
        lastError_.action = "unzOpen";
        return false;
    }
}
bool UnzipFile::list(FileList& files) {
    if(NULL == file_) {
        lastError_.code = -1;
        lastError_.desc = "File not open";
        lastError_.action = "list";
        return false;
    }
    lastError_.clear();
    files.clear();

    int stat = unzGoToFirstFile(file_);
    while(stat == UNZ_OK) {
        char name[1280];
        unz_file_info info;
        stat = unzGetCurrentFileInfo(file_, &info, name, 1280, NULL, 0, NULL, 0);
        if(stat == UNZ_OK) {
            files.push_back(name);
        } else {
            lastError_.code = stat;
            lastError_.desc = showUnzipError(stat);
            lastError_.action = "unzGetCurrentFileInfo";
            break;
        }
        stat = unzGoToNextFile(file_);
    }
    if(UNZ_END_OF_LIST_OF_FILE != stat && 0 == lastError_.code) {
        lastError_.code = stat;
        lastError_.desc = showUnzipError(stat);
        lastError_.action = files.empty() ? "unzGoToFirstFile" : "unzGoToNextFile";
    }
    return 0 == lastError_.code;
}
bool UnzipFile::extract(const char* name, Output& out) {
    if(NULL == file_) {
        lastError_.code = -1;
        lastError_.desc = "File not open";
        lastError_.action = "extract";
        return false;
    }
    lastError_.clear();
    int stat = unzLocateFile(file_, name, 0);
    if(stat == UNZ_OK) {
        stat = unzOpenCurrentFile(file_);
        if(stat == UNZ_OK) {
            char buf[4096];
            size_t len = 4096;
            int nRead = 0;
            while((nRead = unzReadCurrentFile(file_, buf, len)) > 0 ) {
                int nWrite = out.write(buf, nRead);
                if(nWrite < nRead) {
                    // write error
                    lastError_.code = -1;
                    lastError_.desc = "Output error";
                    lastError_.action = "Output.write";
                    break;
                }
            }
            if(0 != lastError_.code) { // write error
            } else if (nRead == UNZ_EOF) {
                lastError_.clear();
            } else { // read failed
                lastError_.code = nRead;
                lastError_.desc = showUnzipError(lastError_.code);
                lastError_.action = "unzReadCurrentFile";
            }
            stat = unzCloseCurrentFile(file_);
            if(stat != UNZ_OK && 0 == lastError_.code) {
                lastError_.code = stat;
                lastError_.desc = showUnzipError(stat);
                lastError_.action = "unzCloseCurrentFile";
            }
        } else { // open file in zip failed
            lastError_.code = stat;
            lastError_.desc = showUnzipError(stat);
            lastError_.action = "unzOpenCurrentFile";
        }
    } else { // not care the UNZ_END_OF_LIST_OF_FILE
        lastError_.code = stat;
        lastError_.desc = showUnzipError(stat);
        lastError_.action = "unzLocateFile";
    }
    return 0 == lastError_.code;
}
bool UnzipFile::close() {
    lastError_.clear();
    if(file_) {
        int stat = unzClose(file_);
        if(UNZ_OK != stat) {
            lastError_.code = stat;
            lastError_.desc = showUnzipError(stat);
            lastError_.action = "unzClose";
        }
        file_ = NULL;
    }
    return 0 == lastError_.code;
}

const Error& UnzipFile::getLastError() {
    return lastError_;
}

RawFileOutput::RawFileOutput():file_(NULL){}
RawFileOutput::~RawFileOutput() {
    close();
}
bool RawFileOutput::open(const char* path) {
    close();
    file_ = fopen(path, "wb");
    return NULL != file_;
}
int RawFileOutput::write(const char* data, size_t len) {
    if(file_)
        return fwrite(data, 1, len, file_);
    else
        return 0;
}
void RawFileOutput::close() {
    if(file_) {
        fclose(file_);
        file_ = NULL;
    }
}
ZipFileOutput::ZipFileOutput():file_(NULL), openedInZip_(false){}
bool ZipFileOutput::open(const char* path) {
    close();
    file_ = zipOpen (path, APPEND_STATUS_CREATE);
    return NULL != file_;
}
bool ZipFileOutput::openInZip(const char* name) {
    closeInZip();
    openedInZip_ = (NULL != file_ && ZIP_OK == zipOpenNewFileInZip(file_, name, NULL, NULL, 0, NULL, 0, "", Z_DEFLATED, Z_DEFAULT_COMPRESSION));
    return openedInZip_;
}
int ZipFileOutput::write(const char* data, size_t len) {
    if(openedInZip_ && ZIP_OK == zipWriteInFileInZip(file_, data, len)) {
        return len;
    } else {
        return 0;
    }
}
void ZipFileOutput::closeInZip() {
    if(openedInZip_) {
        zipCloseFileInZip(file_);
        openedInZip_ = false;
    }
}
void ZipFileOutput::close() {
    closeInZip();
    if(file_) {
        zipClose(file_, "");
        file_ = NULL;
    }
}

GzipFileOutput::GzipFileOutput():file_(NULL){}
GzipFileOutput::~GzipFileOutput() {
    close();
}
bool GzipFileOutput::open(const char* path) {
    close();
    file_ = gzopen(path, "wb");
    return NULL != file_;
}
int GzipFileOutput::write(const char* data, size_t len) {
    if(file_)
        return gzwrite(file_, data, len);
    else
        return 0;
}
void GzipFileOutput::close() {
    if(file_) {
        gzclose(file_);
        file_ = NULL;
    }
}

} // namespace ZipHelper
