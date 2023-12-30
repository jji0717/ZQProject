// DataDebug.h: interface for the DataDebug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATADEBUG_H__E7019278_58F7_47EA_9519_4EF6813FAE86__INCLUDED_)
#define AFX_DATADEBUG_H__E7019278_58F7_47EA_9519_4EF6813FAE86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _DEBUG

#define DBG_STDSENDER_COUNTER		0
#define DBG_STDREADER_COUNTER		1
#define DBG_PSISENDER_COUNTER		2

#define DBG_MAX_COUNTER				3

#define dataDebug		(::DataStream::DataDebug::getDataDebug())

namespace DataStream {

class DataDebug  
{
protected:
	DataDebug();	
	
public:
	virtual ~DataDebug();
	void finalDetect();
	static DataDebug& getDataDebug();

	LONG incCounter(unsigned int counterId)
	{
		if (counterId >= DBG_MAX_COUNTER) {
			assert(false);
			return 0;
		}

		return InterlockedIncrement(&_dbgCounter[counterId]);
	}

	LONG decCounter(unsigned int counterId)
	{
		if (counterId >= DBG_MAX_COUNTER) {
			assert(false);
			return 0;
		}
		
		return InterlockedDecrement(&_dbgCounter[counterId]);
	}

	int dbgPrint(const char* fmt, ...)
	{
		if (&glog == NULL)
			return 0;

		char buf[2024];
		va_list vlist;
		va_start(vlist, fmt);
		int r = vsprintf(buf, fmt, vlist);
		glog(ZQLIB::Log::L_DEBUG, buf);
		return r;
	}
	
public:
	LONG _dbgCounter[DBG_MAX_COUNTER];
};

} // namespace DataStream {

#endif // #ifdef _DEBUG

#endif // !defined(AFX_DATADEBUG_H__E7019278_58F7_47EA_9519_4EF6813FAE86__INCLUDED_)
