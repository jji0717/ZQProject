// HttpDlg.cpp: implementation of the HttpDlg class.
//
//////////////////////////////////////////////////////////////////////
#include "HttpDlg.h"
#include <log.h>
#include <strHelper.h>

#define DIALOG(x)	"HttpDialog[%12s]"##x,m_strDialogIdentity.c_str()

#ifdef envlog
#undef envlog
#endif //

#define envlog (*_env._pHttpSvcLog)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

HttpDlg::HttpDlg(ZQTianShan::Sentry::SentryEnv& env):_env(env)
{
	m_connection		=	NULL;
	m_bHttpHeaderComplete=	false;
	m_iHttpCurPostSize	=	0;
	m_iHttpCurHeaderSize=	0;
	m_iHttpPostTotalSize=	0;
	m_pHttpPostBuffer	=	NULL;

	
	ZeroMemory(m_httpHeaderBuffer,sizeof(m_httpHeaderBuffer));
	
	char	szConnIdent[32];
	ZeroMemory(szConnIdent,sizeof(szConnIdent));
	SYSTEMTIME st;
	GetSystemTime(&st);
	DWORD id =(DWORD )st.wHour << 28 | 
		(DWORD)st.wMinute << 22 | 
		(DWORD)st.wSecond << 16;	
	static WORD sLastLWord =0;	
	id = id | (sLastLWord ++);	
	sprintf(szConnIdent,"%12u",id++);
	m_strDialogIdentity	=	szConnIdent;
	envlog(Log::L_DEBUG,DIALOG("Create a new Http Dialog"));
}

HttpDlg::~HttpDlg()
{
	ClearResource();
	//clear all resource
}
void HttpDlg::onConnected(IN IMainConn* conn)
{
	if( conn == NULL )
	{
		envlog(Log::L_ERROR,DIALOG("onConnected() NULL conn passed in"));
		return;
	}
	m_connection = conn;
	envlog(Log::L_DEBUG,DIALOG("onConnected() HttpDialog %s "),m_strDialogIdentity.c_str());
	
}
void HttpDlg::onRequest(IN IMainConn* conn, IN const void* buf,IN int size)
{
	if( conn==NULL )
	{
		envlog(Log::L_ERROR,DIALOG("onRequest() null conn passed in"));
		return;
	}

	if( !m_bHttpHeaderComplete )
	{//the request message is insufficient
		bool	bOverflow = false;
		int		restData = 0;
		if (m_iHttpCurHeaderSize+size>HTTP_REQUEST_HEADER_BUFFER_SIZE-1) 
		{
			bOverflow = true;
			int tempSize = HTTP_REQUEST_HEADER_BUFFER_SIZE-1 - m_iHttpCurHeaderSize;
			restData = size-tempSize;
			size = tempSize;
			//how to deal with the rest message data ?????
			
		}

		memcpy(m_httpHeaderBuffer+m_iHttpCurHeaderSize,buf,size);
		char* pDoubleReturnChar = strstr(m_httpHeaderBuffer,"\r\n\r\n");
		
		if ( bOverflow && !pDoubleReturnChar )
		{
			envlog(ZQ::common::Log::L_ERROR,
				DIALOG("onRequest() Invalid request and request data is more than %d bytes"),
				HTTP_REQUEST_HEADER_BUFFER_SIZE-1);

#pragma message(__MSGLOC__"TODO : Should I post Bad request back to client ???? ")
			return;
			ClearResource();
		}

		if( pDoubleReturnChar == NULL )
		{//"\r\n\r\n" is not here,wait for it
			m_iHttpCurHeaderSize += size;
		}
		else
		{//get "\r\n\r\n"
			m_bHttpHeaderComplete = true;
			int iPostSize= size + restData - ( pDoubleReturnChar+4 - m_httpHeaderBuffer -m_iHttpCurHeaderSize);
			if(iPostSize < 0)
			{
				//something wrong
				assert(false);
			}
			m_iHttpCurHeaderSize += size;			

			m_vecHeaderField.clear();
			m_mapHeaderField.clear();
			stringHelper::SplitString(m_httpHeaderBuffer , m_vecHeaderField ,"\n","\r");
			if(m_vecHeaderField.size() <=0 )
			{
				assert(false);
			}
			
			m_strFirstLine = m_vecHeaderField[0];
			for(int iCount = 1 ; iCount < (int) m_vecHeaderField.size() ; iCount++ )
			{
				const char* pElement = m_vecHeaderField[iCount].c_str();
				const char* pToken = strstr( pElement,":");
				if( pToken )
				{
					std::string	strKey ;
					strKey.append(pElement,pToken-pElement);
					std::string	strValue;
					strValue.append(pToken+1);
					ZQ::common::stringHelper::TrimExtra(strKey , " ");
					ZQ::common::stringHelper::TrimExtra(strValue , " ");
					m_mapHeaderField.insert(std::make_pair<std::string,std::string>(strKey,strValue));
				}
			}
			bool	bFoundCtntLength = false;
			const char* pBodyLength = getHeaderValue("Content-Length");
			if(pBodyLength)
			{
				m_iHttpPostTotalSize = atoi(pBodyLength);
				envlog(Log::L_DEBUG,DIALOG("onRequest() get content body length %d"),m_iHttpPostTotalSize );
				bFoundCtntLength = true;
			}			
			if(bFoundCtntLength)
			{//found content-body field
				m_pHttpPostBuffer = new char[m_iHttpPostTotalSize+1];
				ZeroMemory(m_pHttpPostBuffer,m_iHttpPostTotalSize+1);
				iPostSize = iPostSize > m_iHttpPostTotalSize ? m_iHttpPostTotalSize :iPostSize;
				memcpy(m_pHttpPostBuffer,pDoubleReturnChar+4,iPostSize);
				m_iHttpCurPostSize=iPostSize;				
				if(m_iHttpCurPostSize >= m_iHttpPostTotalSize)
				{
					m_pHttpPostBuffer[m_iHttpPostTotalSize]='\0';
					m_bHttpHeaderComplete = true;

					try
					{
						ProcessHttpReqeust();
					}
					catch (...) 
					{
					}					

					ClearResource();
					return;
				}
			}
			else
			{//no content-body field is found
				m_pHttpPostBuffer = NULL;
				m_bHttpHeaderComplete = true;
				ProcessHttpReqeust();
				ClearResource();
				return ;
			}
		}
	}
	else
	{
		if ( m_iHttpCurPostSize + size >m_iHttpPostTotalSize ) 
		{
			size = m_iHttpPostTotalSize -m_iHttpCurPostSize;
		}
		memcpy(m_pHttpPostBuffer+m_iHttpCurPostSize,buf,size);
		m_iHttpCurPostSize+=size;
		if(m_iHttpCurPostSize>=m_iHttpPostTotalSize)
		{
			m_pHttpPostBuffer[m_iHttpPostTotalSize]='\0';
			m_bHttpHeaderComplete = true;
			ProcessHttpReqeust();
			ClearResource();
		}
		return ;
	}
}
const char* HttpDlg::getHeaderValue(const char* key)
{
	std::map<std::string,std::string>::iterator it =m_mapHeaderField.begin();
	for( ; it != m_mapHeaderField.end() ; it ++ )
	{
		if ( stricmp(key,it->first.c_str()) == 0) 
		{
			return it->second.c_str();
		}
	}
	return NULL;
}
bool HttpDlg::ProcessHttpReqeust()
{
	if(m_pHttpPostBuffer == NULL)
	{
		envlog(Log::L_ERROR,DIALOG("ProcessHttpReqeust() no request buffer "));
		return false;
	}
	return false;
}
void HttpDlg::onIdlenessTimeout(IN IMainConn* conn)
{
}
void HttpDlg::onConnectionDestroyed(IN IMainConn* conn)
{
	envlog(Log::L_INFO,DIALOG("onConnectionDestroyed() "));
	conn->release();
}
void HttpDlg::ClearResource()
{
	if(m_pHttpPostBuffer)
	{
		delete[] m_pHttpPostBuffer;
		m_pHttpPostBuffer = NULL;
	}
	m_bHttpHeaderComplete = false;
	m_iHttpCurHeaderSize =0;
	m_iHttpPostTotalSize =0;
	m_iHttpCurPostSize   =0;
	ZeroMemory(m_httpHeaderBuffer,sizeof(m_httpHeaderBuffer));	
}
bool HttpDlg::SendData(const char* pBuf , int iLen )
{
	if(!m_connection)
	{
		envlog(Log::L_ERROR,DIALOG("NO avaiable connection,sendData failed"));
		return false;
	}
	return m_connection->send(pBuf,iLen)==iLen;
}
const char* HttpDlg::GetRequestPostData( )
{
	return m_pHttpPostBuffer;
}

// the post data length
int HttpDlg::GetPostDataSize()
{
    return m_iHttpPostTotalSize;
}