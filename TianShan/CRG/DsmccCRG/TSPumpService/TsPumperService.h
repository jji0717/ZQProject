#if !defined(AFX_TsPumperService_H__95E43CAC_5C0B_407F_9C55_A3D4E6E2220C__INCLUDED_)
#define AFX_TsPumperService_H__95E43CAC_5C0B_407F_9C55_A3D4E6E2220C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable : 4251 4275) 

#include <string>
#include "TsPump.h"

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#elif defined ZQ_OS_LINUX
#include "ZQDaemon.h"
#endif

class TsPumperService : public ZQ::common::BaseZQServiceApplication 
{
public:
	TsPumperService(void);
	~TsPumperService(void);
protected: 
	HRESULT OnInit();
	HRESULT OnStart();
	HRESULT OnStop();
	HRESULT OnUnInit();
protected:
#ifdef ZQ_OS_MSWIN
	bool initMiniDump();
#elif defined ZQ_OS_LINUX

#endif
public:
	friend	class  TsPumper;
	TsPumper* _pTsPumper;

};

#endif // !defined(AFX_TsPumperService_H__95E43CAC_5C0B_407F_9C55_A3D4E6E2220C__INCLUDED_)
