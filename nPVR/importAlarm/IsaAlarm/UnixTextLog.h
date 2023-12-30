// UnixTextLog.h: interface for the UnixTextLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNIXTEXTLOG_H__91578AA5_7332_443E_A407_7B1282EC735A__INCLUDED_)
#define AFX_UNIXTEXTLOG_H__91578AA5_7332_443E_A407_7B1282EC735A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KeyDefine.h"
#include "BaseMessageReceiver.h"

class UnixTextLog : public BaseMessageReceiver  
{
public:
	UnixTextLog(int channelID);
	virtual ~UnixTextLog();

	virtual bool init(InitInfo& initInfo, const char* szSessionName);
	virtual void close();

	virtual void OnMessage(int nMessageID, MessageFields* pMessage);

	virtual void requireFields(std::vector<std::string>& fields);

	static const char* getTypeInfo()
	{
		return KD_KV_RECEIVERTYPE_UNIXLOG;
	}

protected:
	bool checkLog();
	
	HANDLE	_hFile;
	std::string _filename;

	int		_nMaxFileSize;
	int		_nMaxFileNumber;
	
	DWORD	_dwFileSize;

	static	const char*	_requiredFields[];
	static  int			_nRequiredField;
};

#endif // !defined(AFX_UNIXTEXTLOG_H__91578AA5_7332_443E_A407_7B1282EC735A__INCLUDED_)
