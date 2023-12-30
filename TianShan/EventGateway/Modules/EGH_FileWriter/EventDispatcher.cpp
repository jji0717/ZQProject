#include "EventDispatcher.h"
#include "strHelper.h"
#include "FileSystemOp.h"
#include <glob.h>
#include <sstream>

extern ZQ::common::Config::Loader<EventGateway::FileWriter::FileWriterConfig> fwConfig;

__BEGIN_FILE_WRITER

WorkerThread::WorkerThread(const std::string& path, ZQ::common::Log& logger)
:_path(path), _fd(0), _quit(false), _log(logger) {
}

WorkerThread::~WorkerThread() {
    if(_fd) {
        fclose(_fd);
        _fd = 0;
    }
}

bool WorkerThread::init() {
    std::vector<std::string> elem = ZQ::common::stringHelper::rsplit(_path, FNSEPC, 1);
    char bname[512];
    if(elem.size() == 2) {
        _dname = elem.at(0);
	strcpy(bname, elem[1].c_str());
        // bname = elem.at(1).c_str();
    }
    else if(elem.size() == 1) {
	strcpy(bname, elem[0].c_str());
        // bname = elem.at(0).c_str();

        char buff[255];
        char* p = getcwd(buff, 255);
        if(p) {
            _dname = p;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    char* p = strrchr(bname, '.');
    _ext = (p+1);

    *p = '\0';
    _fname = bname;

    _log(ZQ::common::Log::L_DEBUG, "dir:[%s] basename:[%s] ext:[%s]", _dname.c_str(), _fname.c_str(), _ext.c_str());

    if(!FS::createDirectory(_dname)) {
        _log(ZQ::common::Log::L_ERROR, 
            "failed to create path  [%s]: [%d] [%s]\n", 
            elem.at(0).c_str(), SYS::getLastErr(), SYS::getErrorMessage().c_str());
         return false;
    }

    if(!createMainFile()) {
        return false;
    }

    struct stat st;
    if(!stat(_path.c_str(), &st)) {
        if(st.st_size >= fwConfig.rotateSize) {
            rotate();
            _size = 0;
        }
        _size = st.st_size;
    }

    return true;
}

void WorkerThread::stop() {
    fflush(_fd);

    _quit = true;
    _handle.signal();
    waitHandle(5000);
}

bool WorkerThread::createMainFile() {
	 if(_fd) {
        fclose(_fd);
        _fd = 0;
    }
    _fd = fopen(_path.c_str(), "a");
    if(!_fd) {
        _log(ZQ::common::Log::L_ERROR, 
                "failed to open file [%s]: [%d] [%s]\n", 
                _path.c_str(), SYS::getLastErr(), SYS::getErrorMessage().c_str());
        return false;
    }
    _log(ZQ::common::Log::L_DEBUG, "created main file [%s]", _path.c_str());

    return true;
}

void WorkerThread::rotate() {
    int32 rotateCount = fwConfig.rotateCount;
    int32 rotateStart = fwConfig.rotateStart;

    std::ostringstream pattern;
    pattern << _dname << '/' << _fname << ".*." << _ext;

    int i = 0;
    glob_t globResult;
    int rc = glob(pattern.str().c_str(), 0, 0, &globResult);

    if(!rc) {
        _log(ZQ::common::Log::L_DEBUG, "rotate count: %d found: %ld", rotateCount, globResult.gl_pathc);
    }
    else {
        _log(ZQ::common::Log::L_DEBUG, "no history file (%s) found", pattern.str().c_str());
    }

    if (!rc && globResult.gl_pathc > 0 && globResult.gl_pathc >= (size_t)rotateCount) {
        for (i = globResult.gl_pathc-1; i >= 0;  --i) {
            if (i >= rotateCount-2) {
                _log(ZQ::common::Log::L_DEBUG, "going to remove: %s", (globResult.gl_pathv)[i]);
                unlink((globResult.gl_pathv)[i]);
            }
        }
    }

    char* tmp = 0;
    char* oldName = (char*)alloca(255);
    char* newName = (char*)alloca(255);
    sprintf(oldName, "%s/%s.%d.%s",  _dname.c_str(), _fname.c_str(), rotateStart+rotateCount-2, _ext.c_str());
    for (i = rotateCount+rotateStart-2; i > 0; i--) {
        tmp = newName;
        newName = oldName;
        oldName = tmp;
        snprintf(oldName, 255, "%s/%s.%d.%s", _dname.c_str(), _fname.c_str(), i-1, _ext.c_str());

        if (rename(oldName, newName)) {
            if (errno == ENOENT) {
                _log(ZQ::common::Log::L_DEBUG, "old log %s does not exist", oldName);
            } else {
                _log(ZQ::common::Log::L_ERROR, "error renaming %s to %s: %s", oldName, newName, strerror(errno));
            }
        }
        else {
            _log(ZQ::common::Log::L_DEBUG, "renaming %s --> %s", oldName, newName);
        }
    }

    if(rotateCount > 1) {
        char* startName = (char*)alloca(255);
        sprintf(startName, "%s/%s.%d.%s", _dname.c_str(), _fname.c_str(), rotateStart, _ext.c_str());
        if(rename(_path.c_str(), startName)) {
            _log(ZQ::common::Log::L_DEBUG, "error renaming %s to %s: %s", oldName, newName, strerror(errno));
        }
        else {
            _log(ZQ::common::Log::L_DEBUG, "rename %s ---> %s", _path.c_str(), startName);
        }
    }
    else {
        _log(ZQ::common::Log::L_DEBUG, "going to remove %s", _path.c_str());
        unlink(_path.c_str());
    }

    createMainFile();
}

int WorkerThread::run() {
    _log(ZQ::common::Log::L_DEBUG, "WorkerThread [%04x]  enter(): [%s]", id(), _path.c_str());

    while(true) {
        SYS::SingleObject::STATE st = _handle.wait();
        if(st == SYS::SingleObject::SIGNALED) {
            if(_quit) {
                goto exit;
            }
            std::vector<std::string> tmp;
            {
                ZQ::common::MutexGuard guard(_lock);
                tmp.swap(_req);
            }
            std::vector<std::string>::iterator iter = tmp.begin();
            for(; iter != tmp.end(); ++iter) {
                std::string& line = *iter;
                line += '\n';

                if((_size + line.length()) >= (size_t)fwConfig.rotateSize) {
                    _log(ZQ::common::Log::L_INFO, "total bytes (%d), try to rotate (%s)", _size, _path.c_str());
                    rotate();
                    _size = 0;
                }

                size_t bytes = fwrite(line.data(), 1, line.length(), _fd);
                if(bytes < line.length()) {
                    if(ferror(_fd)) {
                        _log(ZQ::common::Log::L_ERROR, 
                                "failed to write (%s) to file(%s): [%d] %s\n", 
                                line.c_str(), _path.c_str(), SYS::getLastErr(), SYS::getErrorMessage().c_str());
                        clearerr(_fd);
                    }
                } /* write error */
                else {
                    _size += bytes;
                }
            }
        } /* signaled */
        else {
            continue;
        }

        fflush(_fd);
        if(_req.size() > 0) {
            _handle.signal();
        }
    }

exit:
    _log(ZQ::common::Log::L_DEBUG, "WorkerThread [%04x]  leave(): [%s]", id(), _path.c_str());

    return (0);
}

void WorkerThread::addLines(const Lines& lines) {
    ZQ::common::MutexGuard guard(_lock);

    Lines::const_iterator iter = lines.begin();
    for(; iter != lines.end(); ++iter) {
        _req.push_back(iter->text);
    }
    _handle.signal();
}

__END_FILE_WRITER

