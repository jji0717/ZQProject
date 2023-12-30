#ifndef __ZQ_NLE_FileOpJob_H__
#define __ZQ_NLE_FileOpJob_H__
#include "NLEUtils.h"
#include "NLEDefine.h"

// this class is designed for Emulator only
// and there is no thread-protection in the calls
//
class FileOpJob :public NLE::TimeoutObject {
public:

    explicit FileOpJob(NLE::Watchdog& watchdog, ZQ::common::Log& log);
    virtual ~FileOpJob();
    bool setup(const OPITEMS ops, int watchId, std::string fid);
    struct JobResult {
        bool success;
        std::string filePath;
        OPITEM errorOp;
        std::string errorMsg;
    };
    void teardown(JobResult & result);
    void start(int64 startTime);
    bool finished (int64& finishTimeHint);
    std::string getLastError() const {
        return lastError_;
    }
public:
    virtual void onTimer();
private:
    bool execute(const OPITEM op);
private:
	ZQ::common::Log& log_;
    NLE::Watchdog& watchdog_;
    int watchId_;
	std::string fid_;
    int fd_;
    std::string filePath_;
    std::vector<char> buf;
    bool readMode_;
    OPITEMS ops_;
    size_t cursor_;
    int64 startTime_;

    std::string lastError_;
};

typedef boost::shared_ptr<FileOpJob> FileOpJobPtr;

class Emulator {
public:
    Emulator(ZQ::common::Log& log, NLE::Watchdog& watchdog);
    // util function

	bool setup(FileInfos& fileinfos, const std::string& rootPath, std::string iterator);
    void start();
    void waitForAllJobsDone();
private:
    struct Requirement {
        std::string path;
        int64 len;
    };
    typedef std::list<Requirement> Requirements;
    static void fixup(FileInfos& fileinfos, const std::string& root, Requirements& requirements);
    bool prepare(const Requirements& requirements, Requirement& failedRequirement, std::string& error);
private:
    ZQ::common::Log& log_;
    NLE::Watchdog& watchdog_;
    struct JobItem {
        FileOpJobPtr job;
        std::string fid;
        int watchId;
        int state; // 1 for success, -1 for failure, 0 for running
    };
    typedef std::vector<JobItem> Jobs;
    Jobs jobs_;

	std::string iterator_;
};

#endif
