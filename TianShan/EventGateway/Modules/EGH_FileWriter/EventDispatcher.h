#ifndef __EVENT_DISPATCHER__
#define __EVENT_DISPATCHER__

#include "NativeThread.h"
#include "SystemUtils.h"
#include "FileWriterConfig.h"
#include "Locks.h"

__BEGIN_FILE_WRITER


class WorkerThread : public ZQ::common::NativeThread {

public:
    WorkerThread(const std::string&, ZQ::common::Log&);
    virtual ~WorkerThread(); 

    virtual bool init();
    virtual int run();

public:
    void addLines(const Lines&);
    std::string getPath() const { return _path; }

    void stop();
    void rotate();

private:
    bool createMainFile();

private:
    std::string _path;
    FILE* _fd;
    bool _quit;

    ZQ::common::Log& _log;

    ZQ::common::Mutex _lock;
    std::vector<std::string> _req;

    SYS::SingleObject _handle;
private:
    std::string _fname;
    std::string _dname;
    std::string _ext;
    size_t _size;
};


__END_FILE_WRITER

#endif
