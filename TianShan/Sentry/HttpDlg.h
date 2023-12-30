// HttpDlg.h: interface for the HttpDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPDLG_H__B1794B7A_5D03_4B39_896B_27D1C80C820B__INCLUDED_)
#define AFX_HTTPDLG_H__B1794B7A_5D03_4B39_896B_27D1C80C820B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning(disable:4786)
#include <zq_common_conf.h>
#include <string>
#include <vector>
#include <map>
#include <NativeThreadPool.h>
#include <locks.h>

#include "sentryenv.h"
#define SOCKADDR_IN6_DEFINED
#include "ServiceFrame.h"
#include "httpdInterface.h"

//class HttpEnv
//{
//public:
//	const char*		FindReference(const std::string& key )
//	{
//		std::map<std::string,std::string>::iterator it = _reference.begin();
//		for ( ; it != _reference.end() ;it++ )
//		{
//			if ( key == it->first ) 
//			{
//				return it->second.c_str();
//			}
//		}
//		return NULL;
//	}
//public:
//	std::map<std::string,std::string>	_reference;
//	std::string					_strWebRoot;	
//	std::string					_strDllConfig;
//	std::string					_strDefaultPage;
//	unsigned short				_iServePort;
//	ZQ::common::Log*			_pLog;
//};
#define HTTP_REQUEST_HEADER_BUFFER_SIZE	4*1024

class HttpDlg  : public IDialogue,public IHttpYeoman
{
public:
	HttpDlg(ZQTianShan::Sentry::SentryEnv& env);
	virtual ~HttpDlg();
public:
	
	virtual void		onConnected(IN IMainConn* conn);
	
	
	virtual void		onRequest(IN IMainConn* conn, IN const void* buf,IN int size);
	
	virtual void		onIdlenessTimeout(IN IMainConn* conn);
	
	virtual void		onConnectionDestroyed(IN IMainConn* conn) ;

	virtual bool		ProcessHttpReqeust();

	const char*			getHeaderValue(const char* key);

	bool				SendData(const char* pBuf , int iLen );

	const char*			GetRequestPostData( );

    /// the post data length
    int GetPostDataSize();

	const std::map<std::string,std::string>&	GetHeaderFieldMap()
	{
		return m_mapHeaderField;
	}
protected:

	virtual void				ClearResource();
	
protected:
	
	std::string							m_strFirstLine;
	std::vector<std::string>			m_vecHeaderField;
	std::map<std::string,std::string>	m_mapHeaderField;

	std::string				m_strDialogIdentity;
	char*					m_pHttpPostBuffer;			/*http request buffer*/
	char					m_httpHeaderBuffer[HTTP_REQUEST_HEADER_BUFFER_SIZE];		/*http request without content body*/
	IMainConn*				m_connection;					/*socket connection*/
private:	
	bool					m_bHttpHeaderComplete;			/*indicate if the header complete or not*/
	int						m_iHttpPostTotalSize;		/*http request size in byte,include content body*/
	int						m_iHttpCurHeaderSize;
	int						m_iHttpCurPostSize;	
	ZQTianShan::Sentry::SentryEnv&				_env;
};


#endif // !defined(AFX_HTTPDLG_H__B1794B7A_5D03_4B39_896B_27D1C80C820B__INCLUDED_)
