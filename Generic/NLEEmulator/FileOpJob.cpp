#include "FileOpJob.h"
#include <TimeUtil.h>
#include <sstream>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FLOG (log_)

FileOpJob::FileOpJob(NLE::Watchdog& watchdog, ZQ::common::Log& log)
:watchdog_(watchdog), log_(log), watchId_(0), fd_(-1), readMode_(true), cursor_(0), startTime_(0), fid_("") {
}

FileOpJob::~FileOpJob() {
    if(fd_ != -1) {
        _close(fd_);
        fd_ = -1;
    }
}
bool FileOpJob::setup(const OPITEMS ops, int watchId, std::string fid) {
    watchId_ = watchId;
	fid_     = fid;
    ops_.clear();
    lastError_.clear();
    if(ops.size() < 2) {
        lastError_ = "Invalid job: atleast 2 operation required('open','close')";
        return false;
    } else {
        if(ops.begin()->opID != opCreate || ops.rbegin()->opID != opClose) {
            lastError_ = "Invalid job: the first op and the last op must be 'open' and 'close'";
            return false;
        }
        if(ops.begin()->filepath.empty()) {
            lastError_ = "invalid file path";
            return false;
        }
        filePath_ = ops.begin()->filepath;
        FileOperate jobType = opRead;
/*      for(size_t i = 1; i < ops.size() - 1; ++i)
		{
            const OPITEM& op = ops[i];
            if(op.opID != jobType) {
                if(i == 1) { // the first operation
                    jobType = op.opID;
                } else {
                    // report the error
                    lastError_ = "Invalid job: 'read' and 'write' operations can't be mixed in one job";
                    return false;
                }
            }
        }*/
        readMode_ = jobType == opRead;

        ops_ = ops;
        cursor_ = 0; // the next operation index
        startTime_ = 0;
        return true;;
    }
}
void FileOpJob::teardown(JobResult& result) {
    if(fd_ != -1) {
        _close(fd_);
        fd_ = -1;
    }
    
    result.success = false;
    result.filePath = filePath_;
    result.errorMsg.clear();
    result.errorOp.opID = opCreate;
    result.errorOp.stampMs = 0;
    result.errorOp.opLen = 0;
    result.errorOp.offset = 0;
    if(ops_.empty()) {
        result.success = false;
        result.errorMsg = "Invalid job: empty operation list";
    } else {
        if(cursor_ < ops_.size()) {
            result.success = false;
            result.errorMsg = lastError_;
            result.errorOp = ops_[cursor_];
        } else {
            result.success = true;
            result.errorMsg.clear();
        }
    }
    cursor_ = 0;
    ops_.clear();
    // generate the report
}
void FileOpJob::start(int64 startTime) {
    if(!ops_.empty()) {
        startTime_ = startTime;
        int watchTime = startTime + ops_[0].stampMs - ZQ::common::now();
        if(watchTime < 0) {
            watchTime = 0;
        }
        watchdog_.watch(watchId_, watchTime);
    } // else: not care
}

bool FileOpJob::finished(int64& finishTimeHint) {
    finishTimeHint = 0;
    if(ops_.empty()) {
        // invalid job, but still count
        return true;
    } else {
        if(cursor_ >= ops_.size()) {
            return true;
        } else {
            if(!lastError_.empty()) {
                return true;
            } else {
                finishTimeHint = startTime_ + ops_.rbegin()->stampMs + 10;
                return false;
            }
        }
    }
}

void FileOpJob::onTimer() {
    while(cursor_ < ops_.size()) {
        const OPITEM& op = ops_[cursor_];
        if(op.stampMs + startTime_ - ZQ::common::now() <= 10) {
            // execute the next operation
            if(execute(op)) {
                ++cursor_;
                continue;
            } else { // fail
                return;
            }
        } else {
            // book the next continue time
            int watchTime = startTime_ + op.stampMs - ZQ::common::now();
            if(watchTime < 0) {
                watchTime = 0;
            }
            watchdog_.watch(watchId_, watchTime);
            return;
        }
    }
    // the operation sequence is clear
}
bool FileOpJob::execute(const OPITEM op) {
	int64 t1 = ZQ::common::now();
    switch(op.opID) {
    case opCreate:
        {
            if(fd_ != -1) {
                lastError_ = "already be opened";
                return false;
            }

 /*           if(readMode_) {
                fd_ = _open(op.filepath.c_str(), _O_BINARY|_O_RDONLY);
            } else {
                fd_ = _open(op.filepath.c_str(), _O_BINARY|_O_CREAT|_O_WRONLY, _S_IWRITE);
            }*/
			fd_ = _open(op.filepath.c_str(), _O_BINARY|_O_CREAT, _S_IWRITE | _S_IREAD);
			FLOG(ZQ::common::Log::L_INFO, "WatchedID: %d, FID: %s, open file %s took %dms",
				watchId_, fid_.c_str(), op.filepath.c_str(), ZQ::common::now() - t1);
            if(fd_ != -1) {
                return true;
            } else {
                lastError_ = strerror(errno);
                return false;
            }
        }
        break;
    case opRead:
        {
            if(fd_ == -1) {
                lastError_ = "file is not open";
                return false;
            }
            if(_lseeki64(fd_, op.offset, SEEK_SET) < 0) {
                // record the error information
                lastError_ = strerror(errno);
                return false;
            }
			FLOG(ZQ::common::Log::L_INFO, "WatchedID: %d, FID: %s,(read)seek file offset[%lld] took %dms", 
				watchId_, fid_.c_str(), op.offset, ZQ::common::now() - t1);

			t1 = ZQ::common::now();
            buf.resize(op.opLen);
            int nRead = _read(fd_, &buf[0], op.opLen);

			FLOG(ZQ::common::Log::L_INFO, "WatchedID: %d, FID: %s, read file length[%lld] took %dms", 
				watchId_, fid_.c_str(), op.opLen,  ZQ::common::now() - t1);

            if(nRead < 0) {
                // record the error information
                lastError_ = strerror(errno);
                return false;
            } else if(nRead == 0) {
                lastError_ = "EOF";
                return false;
            } else if(nRead != op.opLen) {
                lastError_ = "read length not match the operation length";
                return false;
            } else {
                // just discard the content
                return true;
            }
        }
        break;
    case opWrite:
        {
            if(fd_ == -1) {
                lastError_ = "file is not open";
                return false;
            }
            if(_lseeki64(fd_, op.offset, SEEK_SET) < 0) {
                // record the error information
                lastError_ = strerror(errno);
                return false;
            }
			FLOG(ZQ::common::Log::L_INFO, "WatchedID: %d, FID: %s,(write)seek file offset[%lld] took %dms", 
				watchId_, fid_.c_str(), op.offset, ZQ::common::now() - t1);
			t1 = ZQ::common::now();
            buf.resize(op.opLen, '~');
            int nWrite = _write(fd_, &buf[0], op.opLen);
			
			FLOG(ZQ::common::Log::L_INFO, "WatchedID: %d, FID: %s, write file length[%lld] took %dms", 
				watchId_, fid_.c_str(), op.opLen, ZQ::common::now() - t1);

            if(nWrite < 0) {
                // record the error information
                lastError_ = strerror(errno);
            } else if(op.opLen != nWrite) {
                lastError_ = "write length not match the operation length";
                return false;
            } else {
                return true;
            }
        }
        break;
    case opClose:
        {
            if(fd_ == -1) {
                lastError_ = "file is not open";
                return false;
            }
			FLOG(ZQ::common::Log::L_INFO, "WatchedID: %d, FID: %s, closed file took %dms", 
				watchId_, fid_.c_str(), ZQ::common::now() - t1);

            _close(fd_);
            fd_ = -1;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

Emulator::Emulator(ZQ::common::Log& log, NLE::Watchdog& watchdog)
:log_(log), watchdog_(watchdog) {

}

// util function
void Emulator::fixup(FileInfos& fileinfos, const std::string& root, Requirements& requirements) {
    std::string rootDir = root;
    if(!root.empty() && *root.rbegin() != '\\' && *root.rbegin() != '/') {
        rootDir += FNSEPS;
    }

    requirements.clear();
    int64 timeOffset = -1;
    for(FileInfos::iterator itF = fileinfos.begin(); itF != fileinfos.end(); ++itF) {
        Requirement r;
        r.len = 0;
        for(OPITEMS::iterator itOp = itF->opitems.begin(); itOp != itF->opitems.end(); ++itOp) {
            if(timeOffset < 0) {
                timeOffset = itOp->stampMs;
            }
            itOp->stampMs -= timeOffset;
            if(itOp->stampMs < 0) {
                // warning here
                itOp->stampMs = 0;
            }
            if(!itOp->filepath.empty()) {
                std::replace(itOp->filepath.begin(), itOp->filepath.end(), '\\', '_');
                std::replace(itOp->filepath.begin(), itOp->filepath.end(), '/', '_');
                itOp->filepath = rootDir + itOp->filepath;
                r.path = itOp->filepath;
            }
            int64 fileLen = itOp->opLen + itOp->offset;
            if(fileLen > r.len) {
                r.len = fileLen;
            }
        }
        if(!r.path.empty() && r.len > 0) {
            requirements.push_back(r);
        }
    }
}
bool Emulator::prepare(const Requirements& requirements, Requirement& failedRequirement, std::string& error) {
    error.clear();
    failedRequirement.path.clear();
    failedRequirement.len = 0;
    char junk[4096];
    memset(junk, '^', 4096);
    for(Requirements::const_iterator it = requirements.begin(); it != requirements.end(); ++it) {
        if(!it->path.empty() && it->len > 0) {
            int fd = _open(it->path.c_str(), _O_BINARY|_O_CREAT|_O_WRONLY, _S_IWRITE);
            if(fd == -1) {
                failedRequirement = (*it);
                error = strerror(errno);
                return false;
            }
            if(-1 == _lseeki64(fd, 0, SEEK_END)) {
                failedRequirement = (*it);
                error = strerror(errno);
                _close(fd);
                return false;
            }
            int64 nSize = _telli64(fd);
            if(nSize == -1) {
                failedRequirement = (*it);
                error = strerror(errno);
                _close(fd);
                return false;
            }
            while(nSize < it->len) {
                int nWrite = _write(fd, junk, 4096);
                if(nWrite == -1) {
                    failedRequirement = (*it);
                    error = strerror(errno);
                    _close(fd);
                    return false;
                } else {
                    nSize += nWrite;
                }
            }
            _close(fd);
        }
    }
    return true;
}
bool Emulator::setup(FileInfos& fileinfos, const std::string& rootPath, std::string iterator) {
    jobs_.clear();
	iterator_ = iterator;
    Requirements rs;
    fixup(fileinfos, rootPath, rs);
    Requirement r;
    std::string err;
    if(!prepare(rs, r, err)) {
		log_(ZQ::common::Log::L_ERROR, CLOGFMT(NLESimulation, "[%s]Failed to prepare the test file. path(%s), len(%lld), error(%s)"), iterator_.c_str(), r.path.c_str(), r.len, err.c_str());
        return false;
    }
    for(FileInfos::const_iterator it = fileinfos.begin(); it != fileinfos.end(); ++it) {
        FileOpJobPtr job(new FileOpJob(watchdog_, log_));
        int watchId = watchdog_.add(job);
        if(job->setup(it->opitems, watchId, it->fid)) {
            // save the job object and the related info
            JobItem jitem;
            jitem.job = job;
            jitem.fid = it->fid;
            jitem.watchId = watchId;
            jitem.state = 0;
            jobs_.push_back(jitem);
        } else {
            log_(ZQ::common::Log::L_WARNING, CLOGFMT(NLESimulation, "[%s]Failed to create job %s. error(%s)"), iterator_.c_str(), it->fid.c_str(), job->getLastError().c_str());
            watchdog_.remove(watchId);
        }
    }
    return true;
}
void Emulator::start() {
    log_(ZQ::common::Log::L_INFO, CLOGFMT(NLESimulation, "[%s]Start the emulation: jobs(%d)"), iterator_.c_str(), jobs_.size());
    int64 nowTime = ZQ::common::now();
    for(Jobs::iterator it = jobs_.begin(); it != jobs_.end(); ++it) {
        it->job->start(nowTime);
    }
}
// false for timeout
void Emulator::waitForAllJobsDone() {
    while(true) {
        int64 finishTimeMin = 0;
        size_t finishedCount = 0;
        size_t successCount = 0;
        for(Jobs::iterator it = jobs_.begin(); it != jobs_.end(); ++it) {
            if(it->state != 0) {
                ++finishedCount;
                if(it->state == 1) ++successCount;
                continue;
            }

            int64 finishHint = 0;
            if(it->job->finished(finishHint)) {
                ++finishedCount;
                FileOpJob::JobResult result;
                it->job->teardown(result);
                // print the report
                if(result.success) {
                    log_(ZQ::common::Log::L_DEBUG, CLOGFMT(NLESimulation, "[%s]Job %s finished successfully."), iterator_.c_str(), it->fid.c_str());
                    it->state = 1;
                    ++successCount;
                } else {
                    log_(ZQ::common::Log::L_DEBUG, CLOGFMT(NLESimulation, "[%s]Job %s failed. operation(%d) path(%s) length(%lld) offset(%lld) error(%s)"),
						iterator_.c_str(), it->fid.c_str(), result.errorOp.opID, result.filePath.c_str(), result.errorOp.opLen, result.errorOp.offset, result.errorMsg.c_str());
                    it->state = -1;
                }
            } else {
                finishTimeMin = finishTimeMin == 0 || finishHint < finishTimeMin ? finishHint:finishTimeMin;
            }
        }
        printf("[%s]*progress*  success:%4d  finished:%4d  total:%4d\n", iterator_.c_str(), successCount, finishedCount, jobs_.size());
        if(finishedCount < jobs_.size()) {
            int waitTime = finishTimeMin - ZQ::common::now();
            if(waitTime < 1000) {
                waitTime = 1000;
            }
            if(waitTime > 3000) {
                waitTime = 3000;
            }
            Sleep(waitTime);
        } else { // all jobs done
            log_(ZQ::common::Log::L_INFO, CLOGFMT(NLESimulation, "[%s]All jobs done. success(%d) total(%d)"),iterator_.c_str(), successCount, jobs_.size());
            printf("[%s]All jobs done. success(%d) total(%d)\n", iterator_.c_str(),successCount, jobs_.size());
            return;
        }
    }
}