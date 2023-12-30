#if !defined (NPRVSMSDB_H)
#define NPRVSMSDB_H

#include "afx.h"
#include "afxdisp.h"

#import "c:\program files\common files\system\ado\msado15.dll" \
	no_namespace \
	rename ("EOF", "adoEOF")

class SMSDB
{
public:
	SMSDB();
	virtual ~SMSDB();
	
	bool ConnectDB(char* path, WCHAR* error);

	bool Insert(int		packageLength, 
				int		cmd, 
				int		uid,
				char*	serviceCode,
				char*   callerNumber,
				char*	sendTime,
				char*   SMSContent,
				char*	SendContent,
				bool	TicpFinished,
				bool	SmsFinished,
				char*   leftContent = NULL);

	bool UpdateState(int		uid,
					 bool		TicpFinished,
					 bool		SMSFinished,
					 char*		TicpSMSContent = NULL);

	bool SelectUnfinished(char* time, WCHAR* sErr);

	bool SelectPart(char* time, WCHAR* sErr);
	
	bool getData( int&		packageLength,
				  int&		cmd,
				  int&		uid,
				  char*		serviceCode,
				  char*		callerNumber,
				  char*		sendTime,
				  char*		SMSContent,
				  char*		SendContent,
				  char*		leftContent,
				  char*		TicpContent,
				  bool&		TicpFinished,
				  bool&		SmsFinished);

	int  GetUID();

	bool DeleteOverdueMessage(char* time);

private:
	void PopulateComError(const WCHAR* sText, _com_error& cComErr, WCHAR* sError);

private:
	_ConnectionPtr	m_pConnection;
	_RecordsetPtr	m_pRecordset;
};

#endif