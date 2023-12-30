// {AF7986C4-593E-47db-93E2-6C477A18992B}

#ifndef _FLTINIT_H_
#define _FLTINIT_H_

#include <guiddef.h>
static const GUID IID_IFilterInit = {0xaf7986c4, 0x593e, 0x47db, 
	{0x93, 0xe2, 0x6c, 0x47, 0x7a, 0x18, 0x99, 0x2b}};

class ISvcLog {
public:

	enum LogLevel { //	level range: value 0~7

		L_EMERG = 0, L_ALERT,  L_CRIT, L_ERROR,
		L_WARNING, L_NOTICE, L_INFO, L_DEBUG, L_DEBUG_DETAIL, 
	};

	virtual void log(LogLevel level, const char* fmt, ...) = 0;
	virtual void log0(LogLevel level, const char* str) = 0;
	virtual LogLevel getLevel() = 0;
	virtual unsigned int getMaxSize() = 0;
};

#include <unknwn.h>
class IFilterInit: public IUnknown {
public:
	virtual void initLog(ISvcLog* log) = 0;
};

#endif // #ifndef _FLTINIT_H_
