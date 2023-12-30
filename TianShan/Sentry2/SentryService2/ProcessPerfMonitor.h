// ProcessPerfMonitor.h: interface for the ProcessPerfMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSPERFMONITOR_H__67C7A6A4_2FC5_4DDC_AAD5_FC105CC79066__INCLUDED_)
#define AFX_PROCESSPERFMONITOR_H__67C7A6A4_2FC5_4DDC_AAD5_FC105CC79066__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <windows.h>
#include <string>
#include <vector>
#pragma warning(disable: 4786)
namespace ProcessPerfData
{
struct RawData
{
    std::wstring    imageW;
    DWORD           processId;
    DWORD           parentPid;
    DWORD           handleCount;
    DWORD           threadCount;
    LARGE_INTEGER   cpuTime100nsec;
    LARGE_INTEGER   memUsageByte;
    LARGE_INTEGER   vmemSizeByte;
};
struct FormattedData
{
    std::string     imageA;
    DWORD           processId;
    DWORD           parentPid;
    DWORD           handleCount;
    DWORD           threadCount;
    DWORD           cpuUsagePercent;
    LARGE_INTEGER   memUsageByte;
    LARGE_INTEGER   vmemSizeByte;
};

struct State
{
    LARGE_INTEGER sysPerfTime100nsec;
    std::vector< RawData > processesRawData;

    State(){
        sysPerfTime100nsec.QuadPart = 0;
    }
    void swap(State& other){
        std::swap(sysPerfTime100nsec, other.sysPerfTime100nsec);
        processesRawData.swap(other.processesRawData);
    }
};
}

typedef ProcessPerfData::FormattedData ProcessPD;
typedef std::vector< ProcessPD > ProcessesPD;

class ProcessPerfMonitor  
{
public:
    ProcessPerfMonitor(ZQ::common::Log&);
	~ProcessPerfMonitor();

    void CollectPerfData();
    ProcessPerfData::State Snapshot()
    {
        CollectPerfData();
        return _perfState2nd;
    }
    bool GetPerfData(ProcessesPD& pds);
private:
    void gatherPerfData(PPERF_DATA_BLOCK pPerfData);
private:
    //need two performance state to compute cpu usage
    ProcessPerfData::State  _perfState1st;
    ProcessPerfData::State  _perfState2nd;

    ZQ::common::Log &_log;
};

#endif // !defined(AFX_PROCESSPERFMONITOR_H__67C7A6A4_2FC5_4DDC_AAD5_FC105CC79066__INCLUDED_)
