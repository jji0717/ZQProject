// SCLogCol.h: interface for the SCLogCol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCLOGCOL_H__E5A82163_140B_4580_920B_90E52A3E0F89__INCLUDED_)
#define AFX_SCLOGCOL_H__E5A82163_140B_4580_920B_90E52A3E0F89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KeyDefine.h"
#include "BaseInfoCol.h"

class SCLogCol : public BaseInfoCol  
{
public:
	
	SCLogCol();
	virtual ~SCLogCol();

	static const char* getTypeInfo()
	{
		return KD_KV_SOURCETYPE_SCLOG;
	}

	virtual bool init(InitInfo& initInfo, const char* szSessionName);

	virtual void close();

public:
	
	static DWORD getLogPos(HANDLE hFile);

protected:

	int run();
private:
	
	std::string _filename;
	DWORD  _pos;
	bool   _bQuit;
	
	int nextLogLine(char* buf, char** pline);

};

#endif // !defined(AFX_SCLOGCOL_H__E5A82163_140B_4580_920B_90E52A3E0F89__INCLUDED_)
