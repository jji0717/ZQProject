// FtpsXferExtension.h: interface for the CFtpsXferExtension class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FTPSXFEREXTENSION_H__39F6F33C_791D_43A1_9C48_C13418FCD736__INCLUDED_)
#define AFX_FTPSXFEREXTENSION_H__39F6F33C_791D_43A1_9C48_C13418FCD736__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FtpsXfer.h"
#include "PushSessI.h"

extern DWORD _dwSocketReadTimeoutSecs;	

#ifndef MPEG2_TRANSPORT_PACKET_SIZE
#define MPEG2_TRANSPORT_PACKET_SIZE		0x0BC			// 188 byte packets
#endif
#define DEF_BUFFERBLOCK_SIZE	(1000 * MPEG2_TRANSPORT_PACKET_SIZE)

class FtpsXferExtension:public FtpsXfer
{
public:
	FtpsXferExtension(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, NativeThreadPool& Pool, context_t* pContext=NULL);
	virtual ~FtpsXferExtension();

	virtual bool recvFile();
	void SetRcvBufSize(int size);

    wchar_t			_sLogHeaderW[40];

	void LogMsg(DWORD dwTraceLevel, const wchar_t* lpszFmt, ...);
	DWORD _dwBufferBlockSize;
	
};

#endif // !defined(AFX_FTPSXFEREXTENSION_H__39F6F33C_791D_43A1_9C48_C13418FCD736__INCLUDED_)
