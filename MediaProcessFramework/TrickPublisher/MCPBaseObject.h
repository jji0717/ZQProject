// MCPBaseFilter.h: interface for the CMCPBaseFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCPBASEFILTER_H__F5879BDF_0C65_4E6D_87D5_241F761333C3__INCLUDED_)
#define AFX_MCPBASEFILTER_H__F5879BDF_0C65_4E6D_87D5_241F761333C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "streams.h"

class CMCPBaseObject : public CBaseFilter
{
public:
	static enum EVENT_DATA_TYPE
	{
		EVENT_NONE,
		EVENT_LOG,
		EVENT_PROGRESS
	};
	
	typedef struct tagEventData
	{
		tagEventData()
		{
			pDataSourceName=NULL;
			pLogData=NULL;
		}
		
		int		nEventType;//1 log, 2 event, 3 progress
		char *	pDataSourceName;
		char *  pLogData;//for log
		int     nPosition;//for progress
	}EventData;
	
public:
		CMCPBaseObject(const TCHAR    *pName,
			LPUNKNOWN  pUnk,
			CCritSec   *pLock,
			REFCLSID   clsid);
		
		virtual ~CMCPBaseObject();
		
		HRESULT Report(int nEventCode,int nEventType,//1 log, 2 event, 3 progress
							char *	pDataSourceName,
							char *  pLogData,//for log
							int     nPosition);//for progress);
		
};

#endif // !defined(AFX_MCPBASEFILTER_H__F5879BDF_0C65_4E6D_87D5_241F761333C3__INCLUDED_)
