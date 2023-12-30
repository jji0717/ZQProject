// ProcessPerfMonitor.cpp: implementation of the ProcessPerfMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include <Log.h>
#include "ProcessPerfMonitor.h"
//////////////////////////////////////////////////////////////////////////
// helper buffer class
template < typename _Type >
class dynamic_buffer
{
public:
    explicit dynamic_buffer(size_t size = 0){
        init();
        resize(size);
    }
    ~dynamic_buffer(){
        clear();
    }
    size_t size(){
        return m_size;
    }
    _Type* ptr(){
        return m_buf;
    }
    operator _Type*(){
        return m_buf;
    }
    void reset(const _Type &val = _Type()){
        for(size_t i = 0; i < m_size; ++i)
            m_buf[i] = val;
    }
    void resize(size_t size, bool withOldData = false){
        //only grow the buffer
        if(m_size < size){
            _Type* newbuf = new _Type[size];
            if(newbuf){
                //restore old data
                if(withOldData){
                    for(size_t i = 0; i < m_size; ++i){
                        newbuf[i] = m_buf[i];
                    }
                }
                clear();
                m_buf       = newbuf;
                m_size      = size;
            }else{
                //fail to grow memory
            }
        }
    }
private:
    dynamic_buffer(const dynamic_buffer&);
    dynamic_buffer& operator=(const dynamic_buffer&);
    void init(){
        m_size = 0;
        m_buf = NULL;
    }
    void clear(){
        if(m_buf)
            delete []m_buf;
        init();
    }
private:
    _Type* m_buf;
    size_t m_size;
};

//////////////////////////////////////////////////////////////////////////
static void strW2strA(const std::wstring &wstr, std::string &astr)
{
    int requiredBufSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    dynamic_buffer< char > namebuf;
    namebuf.resize(requiredBufSize + 1);
    namebuf.reset();
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), namebuf, namebuf.size(), NULL, NULL);
    astr = namebuf.ptr();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//helper function to manipulate performance data structure
//see MSDN for detail

// performance object
static PPERF_OBJECT_TYPE FirstObject(PPERF_DATA_BLOCK PerfData)
{
    return( (PPERF_OBJECT_TYPE)((PBYTE)PerfData + 
        PerfData->HeaderLength) );
}

static PPERF_OBJECT_TYPE NextObject(PPERF_OBJECT_TYPE PerfObj)
{
    return( (PPERF_OBJECT_TYPE)((PBYTE)PerfObj + 
        PerfObj->TotalByteLength) );
}

static PPERF_OBJECT_TYPE FindObject(PPERF_DATA_BLOCK PerfData, DWORD nObjNameTitleIndex)
{
    PPERF_OBJECT_TYPE curObj = FirstObject(PerfData);
    for(DWORD iObj = 0; iObj < PerfData->NumObjectTypes; ++iObj)
    {
        if(curObj->ObjectNameTitleIndex == nObjNameTitleIndex)
            return curObj;
        curObj = NextObject(curObj);
    }
    return NULL;
}

// performance counter
static PPERF_COUNTER_DEFINITION FirstCounter(PPERF_OBJECT_TYPE PerfObj)
{
    return( (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj + 
        PerfObj->HeaderLength) );
}

static PPERF_COUNTER_DEFINITION NextCounter(PPERF_COUNTER_DEFINITION PerfCntr)
{
    return( (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr + 
        PerfCntr->ByteLength) );
}

static PPERF_COUNTER_DEFINITION FindCounter(PPERF_OBJECT_TYPE PerfObj, DWORD nCounterNameTitleIndex)
{
    PPERF_COUNTER_DEFINITION curCounter = FirstCounter(PerfObj);
    for(DWORD iCounter = 0; iCounter < PerfObj->NumCounters; ++iCounter)
    {
        if(curCounter->CounterNameTitleIndex == nCounterNameTitleIndex)
            return curCounter;
        curCounter = NextCounter(curCounter);
    }
    return NULL;
}

// instance
static PPERF_INSTANCE_DEFINITION FirstInstance(PPERF_OBJECT_TYPE PerfObj)
{
    return( (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj + 
        PerfObj->DefinitionLength) );
}

static PPERF_INSTANCE_DEFINITION NextInstance(PPERF_INSTANCE_DEFINITION PerfInst)
{
    PPERF_COUNTER_BLOCK PerfCntrBlk;

    PerfCntrBlk = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst + 
        PerfInst->ByteLength);

    return( (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfCntrBlk + 
        PerfCntrBlk->ByteLength) );
}

static LPCBYTE ReadPerfData(PPERF_INSTANCE_DEFINITION PerfInst, PPERF_COUNTER_DEFINITION PerfCntr)
{
    PPERF_COUNTER_BLOCK PerfCntrBlk;

    PerfCntrBlk = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst +
        PerfInst->ByteLength);

    return ((PBYTE)PerfCntrBlk + PerfCntr->CounterOffset);
}

//////////////////////////////////////////////////////////////////////////
#define PERFLOG (_log)

#define PERF_OBJECT_IDXSTR_PROCESS      "230"
#define PERF_OBJECT_IDX_PROCESS         230 // desc=Process

#define PERF_COUNTER_IDX_CPUTIME          6 // desc=% Processor Time
#define PERF_COUNTER_IDX_MEMUSAGE       180 // desc=Working Set
#define PERF_COUNTER_IDX_VMEMSIZE       184 // desc=Page File Bytes
#define PERF_COUNTER_IDX_THREADCOUNT    680 // desc=Thread Count
#define PERF_COUNTER_IDX_PROCESSID      784 // desc=ID Process
#define PERF_COUNTER_IDX_PARENTPID     1410 // desc=Creating Process ID
#define PERF_COUNTER_IDX_HANDLECOUNT    952 // desc=Handle Count


ProcessPerfMonitor::ProcessPerfMonitor(ZQ::common::Log& log)
:_log(log)
{
}

ProcessPerfMonitor::~ProcessPerfMonitor()
{

}


void ProcessPerfMonitor::gatherPerfData(PPERF_DATA_BLOCK pPerfData)
{
    if(NULL == pPerfData)
        return;

    PPERF_OBJECT_TYPE pProcessObj = FindObject(pPerfData, PERF_OBJECT_IDX_PROCESS);
    if(NULL == pProcessObj)
        return;

    PPERF_COUNTER_DEFINITION pCpuTimeCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_CPUTIME);
    if(NULL == pCpuTimeCounter || PERF_100NSEC_TIMER != pCpuTimeCounter->CounterType)
        return;

    PPERF_COUNTER_DEFINITION pMemUsageCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_MEMUSAGE);
    if(NULL == pMemUsageCounter || PERF_COUNTER_LARGE_RAWCOUNT != pMemUsageCounter->CounterType)
        return;

    PPERF_COUNTER_DEFINITION pVmemSizeCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_VMEMSIZE);
    if(NULL == pVmemSizeCounter || PERF_COUNTER_LARGE_RAWCOUNT != pVmemSizeCounter->CounterType)
        return;

    PPERF_COUNTER_DEFINITION pThreadCountCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_THREADCOUNT);
    if(NULL == pThreadCountCounter || PERF_COUNTER_RAWCOUNT != pThreadCountCounter->CounterType)
        return;

    PPERF_COUNTER_DEFINITION pProcessIdCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_PROCESSID);
    if(NULL == pProcessIdCounter || PERF_COUNTER_RAWCOUNT != pProcessIdCounter->CounterType)
        return;

    PPERF_COUNTER_DEFINITION pParentPidCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_PARENTPID);
    if(NULL == pParentPidCounter || PERF_COUNTER_RAWCOUNT != pParentPidCounter->CounterType)
        return;

    PPERF_COUNTER_DEFINITION pHandleCountCounter = FindCounter(pProcessObj, PERF_COUNTER_IDX_HANDLECOUNT);
    if(NULL == pHandleCountCounter || PERF_COUNTER_RAWCOUNT != pHandleCountCounter->CounterType)
        return;

    {
        ProcessPerfData::State curState;
        curState.processesRawData.reserve(pProcessObj->NumInstances);

        PPERF_INSTANCE_DEFINITION curInstance = FirstInstance(pProcessObj);
        for(LONG iInstance = 0; iInstance < pProcessObj->NumInstances; ++iInstance)
        {
            ProcessPerfData::RawData instRawData;
            instRawData.cpuTime100nsec = *(PLARGE_INTEGER)ReadPerfData(curInstance, pCpuTimeCounter);
            instRawData.memUsageByte = *(PLARGE_INTEGER)ReadPerfData(curInstance, pMemUsageCounter);
            instRawData.vmemSizeByte = *(PLARGE_INTEGER)ReadPerfData(curInstance, pVmemSizeCounter);
            instRawData.threadCount = *(PDWORD)ReadPerfData(curInstance, pThreadCountCounter);
            instRawData.processId = *(PDWORD)ReadPerfData(curInstance, pProcessIdCounter);
            instRawData.parentPid = *(PDWORD)ReadPerfData(curInstance, pParentPidCounter);
            instRawData.handleCount = *(PDWORD)ReadPerfData(curInstance, pHandleCountCounter);
            instRawData.imageW = (LPCWSTR)((PBYTE)curInstance + curInstance->NameOffset);

            curState.processesRawData.push_back(instRawData);
            curInstance = NextInstance(curInstance);
        }

        // use the _Total's processor time as the base
        if(!curState.processesRawData.empty())
        {
            // _Total always be the last instance
            std::vector< ProcessPerfData::RawData >::reverse_iterator rit_proc;
            for(rit_proc = curState.processesRawData.rbegin(); rit_proc != curState.processesRawData.rend(); ++rit_proc)
            {
                if(rit_proc->processId == 0 && rit_proc->imageW == L"_Total")
                {
                    curState.sysPerfTime100nsec = rit_proc->cpuTime100nsec;
                    break;
                }
            }
        }

        _perfState1st.swap(_perfState2nd);
        _perfState2nd.swap(curState);
    }
}

#define PERF_DATA_SIZE_BASE     (1024 * 128)
#define PERF_DATA_SIZE_INCR     (1024 * 64)
void ProcessPerfMonitor::CollectPerfData()
{
    PERFLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ProcessPerfMonitor, "enter CollectPerfData()."));
    DWORD nBufSize = PERF_DATA_SIZE_BASE;
    dynamic_buffer< BYTE > PerfDataBuf;
    PerfDataBuf.resize(nBufSize);
    PERFLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ProcessPerfMonitor, "prepare data buffer. [bufsize = %u]"), PerfDataBuf.size());

    LONG nRegRet = RegQueryValueEx(
        HKEY_PERFORMANCE_DATA,
        PERF_OBJECT_IDXSTR_PROCESS,
        NULL,
        NULL,
        PerfDataBuf,
        &nBufSize
    );
    while(ERROR_MORE_DATA == nRegRet)
    {
        nBufSize += PERF_DATA_SIZE_INCR;
        PerfDataBuf.resize(nBufSize);
        PERFLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ProcessPerfMonitor, "grow data buffer. [bufsize = %u]"), PerfDataBuf.size());
        nRegRet = RegQueryValueEx(
            HKEY_PERFORMANCE_DATA,
            PERF_OBJECT_IDXSTR_PROCESS,
            NULL,
            NULL,
            PerfDataBuf,
            &nBufSize
        );
    }
    if(ERROR_SUCCESS != nRegRet)
    {
        PERFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ProcessPerfMonitor, "failed to read registry data. [error = %d]"), nRegRet);

        RegCloseKey(HKEY_PERFORMANCE_DATA);
        return;
    }
    PPERF_DATA_BLOCK PerfData = (PPERF_DATA_BLOCK)(PerfDataBuf.ptr());
    gatherPerfData(PerfData);
    RegCloseKey(HKEY_PERFORMANCE_DATA);
    PERFLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ProcessPerfMonitor, "leave CollectPerfData()."));
}

bool ProcessPerfMonitor::GetPerfData(ProcessesPD& pds)
{
    pds.clear();
    LONGLONG sysPerfTimeInterval100nsec = _perfState2nd.sysPerfTime100nsec.QuadPart - _perfState1st.sysPerfTime100nsec.QuadPart;

    if(sysPerfTimeInterval100nsec <= 0)
    {
        PERFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ProcessPerfMonitor, "bad performance data. time interval invalid."));
        return false;
    }
    pds.reserve(_perfState2nd.processesRawData.size());
    std::vector< ProcessPerfData::RawData >::const_iterator cit_process2nd;
    for(cit_process2nd = _perfState2nd.processesRawData.begin(); cit_process2nd != _perfState2nd.processesRawData.end(); ++cit_process2nd)
    {
        //find same process
        std::vector< ProcessPerfData::RawData >::const_iterator cit_process1st;
        for(cit_process1st = _perfState1st.processesRawData.begin(); cit_process1st != _perfState1st.processesRawData.end(); ++cit_process1st)
        {
            if(cit_process1st->processId == cit_process2nd->processId && cit_process1st->imageW == cit_process2nd->imageW)
                break; // got the process
        }
        if(cit_process1st == _perfState1st.processesRawData.end())
            continue;

        ProcessPD pd;
        pd.processId = cit_process2nd->processId;
        pd.parentPid = cit_process2nd->parentPid;
        pd.handleCount = cit_process2nd->handleCount;
        pd.threadCount = cit_process2nd->threadCount;
        pd.memUsageByte = cit_process2nd->memUsageByte;
        pd.vmemSizeByte = cit_process2nd->vmemSizeByte;

        //cpu usage
        LONGLONG curPerfTimeInterval100nsec = cit_process2nd->cpuTime100nsec.QuadPart - cit_process1st->cpuTime100nsec.QuadPart;
        pd.cpuUsagePercent = (DWORD)(100 * curPerfTimeInterval100nsec / sysPerfTimeInterval100nsec);

        //image name
        strW2strA(cit_process2nd->imageW, pd.imageA);
        pds.push_back(pd);
    }
    return true;
}