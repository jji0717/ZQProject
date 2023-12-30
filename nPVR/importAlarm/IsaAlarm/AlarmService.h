// AlarmService.h: interface for the AlarmService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALARMSERVICE_H__5288DFD4_8420_4C86_A036_87028A3E7520__INCLUDED_)
#define AFX_ALARMSERVICE_H__5288DFD4_8420_4C86_A036_87028A3E7520__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSchangeServiceApplication.h"

class InfoCollector;

class AlarmService : public ZQ::common::BaseSchangeServiceApplication  
{
public:
	AlarmService();
	virtual ~AlarmService();

protected:
	HRESULT OnInit();
	
	HRESULT OnStart();

	HRESULT OnStop();

	HRESULT OnUnInit();
	
	InfoCollector*	_pInfoCollector;
};

#endif // !defined(AFX_ALARMSERVICE_H__5288DFD4_8420_4C86_A036_87028A3E7520__INCLUDED_)
