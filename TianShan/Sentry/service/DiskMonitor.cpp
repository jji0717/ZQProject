
#include <boost/thread.hpp>
#include "DiskMonitor.h"
#include "LogPaserManagement.h"

DiskMonitorI::DiskMonitorI(ZQ::common::Log& log, DirectSender& directSender, int pollInterval, const std::string& waringTargets)
	: _log(log), _syslog("Sentry", ZQ::common::Log::L_DEBUG), _directSender(directSender), 
	_pollInterval(pollInterval), _warningTargets(waringTargets)
{
}

DiskMonitorI::~DiskMonitorI()
{
}

void DiskMonitorI::stop()
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskIOMonitorI, "signaled semaphore to stop  monitor thread"));
	_hQuit.signal();
}

////////////////////////////
///class DiskSpaceMonitor
///////////////////////////
DiskSpaceMonitor::DiskSpaceMonitor(ZQTianShan::Sentry::SentryEnv& env, int pollInterval, int maxSkippedWarningCount, const std::string& warningTargets)
	:DiskMonitorI(env._log, env._logParserManagement->directSender(), pollInterval, warningTargets)
{
	if(pollInterval <= 0)       
        _pollInterval = 60000; // 1 min

    if(maxSkippedWarningCount >= 0) 
        _maxSkippedWarningCount = maxSkippedWarningCount;
	else
        _maxSkippedWarningCount = 10;
}

DiskSpaceMonitor::~DiskSpaceMonitor()
{
}

void DiskSpaceMonitor::add(const std::string& path, double freeWarning, double repeatStep) 
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DiskSpaceMonitor, "Add monitoring: path(%s), freeWarning(%.1f%%), repeatStep(%.1f%%)"), path.c_str(), freeWarning * 100, repeatStep * 100);
    if(freeWarning < 0 || repeatStep < 0) {
        return;
    }
    DiskInfo disk;
    disk.path = path;
    disk.firstWarning = freeWarning;
    disk.repeatStep = repeatStep;
    disk.nextWarning = freeWarning;
    disk.skippedWarningCount = 0;
    _diskList.push_back(disk);
}

int DiskSpaceMonitor::run() 
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskSpaceMonitor, "Enter the monitoring thread"));
    bool bContinue = true;
    while(bContinue)
	{
        for(DiskList::iterator it = _diskList.begin(); it != _diskList.end(); it++)
		{
            uint64 avail = 0;
            uint64 total = 0;
            uint64 free = 0;

            if(!FS::getDiskSpace(it->path, avail, total, free))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(DiskSpaceMonitor, "Can't fetch the disk space info of %s"), it->path.c_str());
				continue;
			}

			if (total == 0)
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(DiskSpaceMonitor, "Total space of %s is 0"), it->path.c_str());
				continue;
			}

			double freeRatio = (double) MIN(free,avail) / (double) total;//  (double) free / (double) total;

			if (it->firstWarning < freeRatio) // hasn't reached the warning line
			{
				// 0.0----next-----base(------1.0)
				// reset the counters
				it->nextWarning = it->firstWarning;
				it->skippedWarningCount = 0;
				continue;
			} 

			//start here, has already reached the free space warning line
			// determine if it is necessary to trigger the warning event

			bool bWarn = true;
			
			do {
				// case 1. first hit after service starts
				if (it->nextWarning > it->firstWarning)
				{
					it->nextWarning = it->firstWarning; // stop entering this case again
					break; // leave bWarn = true;
				}

				// adjust the value nextWarning
				if(it->nextWarning < 0.0)
					it->nextWarning = 0.0;
			
				// case 2, hit but hasn't reached the step size and max yield rounds
				if(it->nextWarning < freeRatio && freeRatio <= it->firstWarning && it->skippedWarningCount < _maxSkippedWarningCount)
				{
					// 0.0----next(-----base)------1.0
					it->skippedWarningCount++;
					bWarn = false;
					break;
				} 

				// case 3, hit and necessary to trigger the warning
				// (0.0----next)-----base------1.0

			} while (0);

			if (bWarn)
			{
				warn(it->path, freeRatio, it->firstWarning, free / (1024 * 1024), total / (1024 * 1024));
				it->skippedWarningCount = 0;
				it->nextWarning = freeRatio - it->repeatStep;
			}
		}

        switch(_hQuit.wait(_pollInterval))	
		{
        case SYS::SingleObject::SIGNALED: // quit the polling
            bContinue = false;
			_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskSpaceMonitor, "Requested to stop the polling"));
            break;
        case SYS::SingleObject::TIMEDOUT: // continue the monitoring
            break;
        default: // something bad happened, quit the pooling
            bContinue = false;
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(DiskSpaceMonitor, "Unknown state of polling controller"));
            break;
        }
		
    }

    // leave the monitoring thread
    _log(ZQ::common::Log::L_INFO, CLOGFMT(DiskSpaceMonitor, "Leave the monitoring thread"));
    return 0;
}

void DiskSpaceMonitor::warn(const std::string& path, double freeRatio, double warningRatio, uint64 freeMB, uint64 totalMB) {
    char buf[32];
    sprintf(buf, "%.2f", freeRatio * 100);
    _syslog(ZQ::common::Log::L_WARNING, "%s%% free space left in %s", buf, path.c_str());
    if(_warningTargets.empty()) {
        return;
    }
    Properties properties;
    properties["path"] = path;
    properties["freePercent"] = buf;
    sprintf(buf, "%.0f", warningRatio * 100);
    properties["warningPercent"] = buf;
    sprintf(buf, "%lld", freeMB);
    properties["freeMB"] = buf;
    sprintf(buf, "%lld", totalMB);
    properties["totalMB"] = buf;

    _directSender.send("DiskState", "FreeSpaceLow", 1, properties, _warningTargets);
	
}


///////////////////////////
///class DiskIOMonitor
///////////////////////////
DiskIOMonitor::DiskIOMonitor(ZQTianShan::Sentry::SentryEnv& env, int pollInterval, const std::string& warningTargets)
	: DiskMonitorI(env._log,  env._logParserManagement->directSender(), pollInterval, warningTargets)
{
	if(pollInterval <= 0)	
		_pollInterval = 30000;
}

DiskIOMonitor::~DiskIOMonitor()
{
}

int DiskIOMonitor::run()
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskIOMonitor, "Enter DiskIOMonitor thread"));
    bool bContinue = true;
	//windows OS monitor the logical device 
#ifdef ZQ_OS_MSWIN	
	ZQ::common::DiskIOPerf perf(true);
#else
	ZQ::common::DiskIOPerf perf(false);
#endif
	ZQ::common::DiskIOPerf::IoStatCounters iosP,iosC, iosW;
	ZQ::common::DiskIOPerf::DiskPerfData perfDatas;
	while(bContinue)
	{
		if(_ioDevs.size() > 0)
		{
			if(iosW.size() > 0)
				iosP = iosW;
			else
				iosP = perf.pollIoStats();
		}
		
		//wait interval
		SYS::SingleObject::STATE status = _hQuit.wait(_pollInterval);
		switch(status)
		{
		case SYS::SingleObject::SIGNALED://quit
			bContinue = false;
			_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskIOMonitor, "Requested to stop the disk io monitor poll"));
            break;		
        case SYS::SingleObject::TIMEDOUT: // continue the monitoring
            break;
        default: // something bad happened, quit the pooling
            bContinue = false;
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(DiskIOMonitor, "Unknown state of disk io monitor poll"));
            break;
		}

		if(_ioDevs.size() > 0)
		{
			iosC = perf.pollIoStats();
			iosW = iosC;

			perfDatas = perf.computeDiskStats(iosP,iosC);
			std::map<std::string, struct DiskIODev>::iterator it;
			for(it = _ioDevs.begin(); it != _ioDevs.end(); it++)
			{
				ZQ::common::DiskIOPerf::DiskPerfData::iterator itR;
				for(itR = perfDatas.begin(); itR != perfDatas.end(); itR++)
				{
					//name is eque and have value greater the warning values
					if(stricmp(it->first.c_str(), itR->dev_name) == 0 && (itR->util >= it->second.busy || itR->avgQueSz >= it->second.queueSize))//send waring
						sendWarning(it->first, itR->util, itR->avgQueSz);				
				}
			}
		}		
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskIOMonitor, "Leave DiskIOMonitor thread"));
    return 0;
}

void DiskIOMonitor::addMonitorItem(const std::string& devName, const double& busyWarning, const double& queueSizeWarning)
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(DiskIOMonitor,"add monitor item name[%s] warning value busy[%.2f] queueSize[%.2f]"), devName.c_str(), busyWarning, queueSizeWarning);
	if(devName.length() == 0 || busyWarning <= 0.0 || queueSizeWarning <= 0.0)
	{
		return;
	}
	struct DiskIODev devinfo;
	devinfo.name = devName;
	devinfo.busy = busyWarning;
	devinfo.queueSize = queueSizeWarning;
	_ioDevs.insert(std::pair<std::string, struct DiskIODev>(devName, devinfo));
}

void DiskIOMonitor::sendWarning(const std::string& devName, const double& busyValue, const double& queueSize)
{
	_syslog(ZQ::common::Log::L_WARNING, "device [%s] disk io busy [%.2f%%] queue size[%.2f]", devName.c_str(), busyValue, queueSize);
    if(_warningTargets.empty()) {
        return;
    }

	char buf[10] = {0};
	sprintf(buf, "%.2f", busyValue);
	Properties properties;
    properties["device"] = devName;
    properties["busyPercent"] = buf;
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%.2f", queueSize);
	properties["pendingIOs"] = buf;
    _directSender.send("DiskState", "DiskBusy", 2, properties, _warningTargets);
}

