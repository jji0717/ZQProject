
#ifndef STATUSREPORT_H
#define STATUSREPORT_H

class CStatusReport
{
public:
	static CStatusReport * s_pStatusReport;
private:
	long m_lState;
	long m_lError;
	DWORD m_dwMsgLength;
	TCHAR m_szMsg[256];
public:
	CStatusReport()
	{
		s_pStatusReport = this;
		m_lState = 0;
	};
	bool GetState( long * outState );
	bool GetLastError( long * outError );
	bool GetErrorMsg( char * outMsg, BYTE * outLength );

	void SetState( long inState );
	void SetLastError( long inError );
	void SetErrorMsg( LPCTSTR lpMsg );
};

#endif	// STATUSREPORT_H