#if !defined (NPRVSMSDB_H)
#define NPRVSMSDB_H

#include "afx.h"
#include "afxdisp.h"

#import "c:\program files\common files\system\ado\msado15.dll" \
	no_namespace \
	rename ("EOF", "adoEOF")

class SMSMsg;

class NPVRSMSDB
{
public:
	NPVRSMSDB();
	virtual ~NPVRSMSDB();
	
	bool ConnectDB(char* path);

	bool Insert(int		packageLength, 
				int		cmd, 
				int		uid,
				char*	serviceCode,
				char*   callerNumber,
				char*	sendTime,
				char*   SMSContent,
				char*	SendContent,
				char*   leftContent,
				bool	TicpFinished,
				bool	SmsFinished);

	bool SelectUnfinished(char* time);

	bool SelectByTime(char* time);
	
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

	bool UpdateState(int		uid,
					 bool		TicpFinished,
					 bool		SMSFinished,
					 char*		TicpSMSContent = NULL);

	int  GetUID();

	bool DeleteOverdueMessage(char* time);

private:
	void PopulateComError(const char* sText, _com_error& cComErr, char* sError);

private:
	_ConnectionPtr	m_pConnection;
	_RecordsetPtr	m_pRecordset;
};

#endif