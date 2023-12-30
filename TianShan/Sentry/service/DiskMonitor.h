
#ifndef __SENTRY_DISKMONITOR_H__
#define __SENTRY_DISKMONITOR_H__

#include "LogParserManager.h"
#include "SentryEnv.h"
#include "DiskIOPerf.h"
#include "NativeThread.h"
#include "FileLog.h"
#include "SystemUtils.h"
#include <string>

//Disk monitor base class
class DiskMonitorI : public ZQ::common::NativeThread
{
public:
	DiskMonitorI(ZQ::common::Log& log, DirectSender& directSender, int pollInterval, const std::string& waringTargets);
	virtual ~DiskMonitorI();
	void stop();
protected:
	virtual int run(void)=0;

protected:
	ZQ::common::Log& _log;
    ZQ::common::SysLog _syslog;
    DirectSender& _directSender;
	SYS::SingleObject _hQuit;//for monitor thread quit
	int	_pollInterval;
	std::string	_warningTargets;	

};

//Disk space monitor
class DiskSpaceMonitor : public DiskMonitorI
{
public:
	DiskSpaceMonitor(ZQTianShan::Sentry::SentryEnv& env, int pollInterval = 60000, int maxSkippedWarningCount = 10, const std::string& warningTargets = "");
	~DiskSpaceMonitor();
public:
	virtual int run();
	void add(const std::string& path, double freeWarning, double repeatStep);
private:
    void warn(const std::string& path, double freeRatio, double warningRatio, uint64 freeMB, uint64 totalMB);

private:
	struct DiskInfo {
        std::string path;
        double nextWarning;
        double firstWarning;
        double repeatStep;
        int skippedWarningCount;
        DiskInfo():nextWarning(0.0), firstWarning(0.0), repeatStep(0.0), skippedWarningCount(0){}
    };
    typedef std::vector<DiskInfo> DiskList;
    DiskList _diskList;
    int _maxSkippedWarningCount;
};

//Disk I/O busy monitor
class DiskIOMonitor : public DiskMonitorI
{
public:
	DiskIOMonitor(ZQTianShan::Sentry::SentryEnv& env, int monitorInterval, const std::string& warningTargets);
	virtual ~DiskIOMonitor();

public:
	virtual int run();
	void addMonitorItem(const std::string& devName, const double& busyWarning, const double& queueSizeWarning);

private:
	void sendWarning(const std::string& devName, const double& busyValue, const double& queueSize);

private:
	struct DiskIODev
	{
		std::string name;
		double busy;
		double queueSize;
		DiskIODev():busy(0.0),queueSize(0.0){}
	};
	
	std::map<std::string, struct DiskIODev> _ioDevs;
};


#endif //__SENTRY_DISKMONITOR_H__
