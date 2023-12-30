// BaseInfoCol.h: interface for the BaseInfoCol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEINFOCOL_H__AE76DD3F_8A7C_43AF_93D7_B85991AE7F27__INCLUDED_)
#define AFX_BASEINFOCOL_H__AE76DD3F_8A7C_43AF_93D7_B85991AE7F27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NativeThread.h"
#include <string>
#include <vector>
#include "InitInfo.h"

using namespace std;

class HandlerGroup;

class BaseInfoCol : public ZQ::common::NativeThread
{
public:
	BaseInfoCol();
	virtual ~BaseInfoCol();

	virtual bool init(InitInfo& initInfo, const char* szSessionName)=0;
	virtual void close()=0;

	void setHandlerGroup(HandlerGroup* pGroup)
	{
		_pHandlerGroup = pGroup;
	}

protected:

	virtual void OnNewMessage(const char* line);

	HandlerGroup*		_pHandlerGroup;
};

#endif // !defined(AFX_BASEINFOCOL_H__AE76DD3F_8A7C_43AF_93D7_B85991AE7F27__INCLUDED_)
