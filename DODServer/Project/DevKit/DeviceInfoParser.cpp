
/*
**	FILENAME			DeviceInfoParser.cpp
**
**	PURPOSE				The file is a .cpp file, used along with DeviceInfoParser.h.
**						a pure-virtual class CDeviceInfoParser is declared in the file, and this class is used to provide a 
**						interface for parsing packages that are got from socket connecting with AM.
**						
**						
**
**	CREATION DATE		19-07-2004
**	LAST MODIFICATION	21-07-2004
**
**	AUTHOR				Leon.li (Interactive ZQ)
**
**
*/

#include "StdAfx.h"
#include "clog.h"
#include "DeviceInfoParser.h"
//#include "common.h"
#include "basecommand.h"
#include "Markup.h"
#include "SingleConnect.h"
#include "MessageMacro.h"

struIP		m_struIP;
#define MAXNUMBER 65536
#define OVERTIME 12

int CDeviceInfoParser::GetCommandID()
{
	if (m_nCommandID>MAXNUMBER)
	{
		m_nCommandID=1;
	}
	else
		m_nCommandID++;
	return m_nCommandID;
}

VOID CDeviceInfoParser::Parse(FredPtr block )
{	
	DWORD SessionID;
	if( block->GetSize() <= 0 )
	{
		Clog(LOG_ERROR,"DeviceInfoParse : find bloce size is negative");	
		return;
	}
	CMarkup m_XmlDOM;

	if(!m_XmlDOM.SetDoc(CString(block->GetBlock(), block->GetSize())))
	{
		Clog( LOG_DEBUG, _T("Info - CDSAParse::Parse() - !Parse Message Error."));
		return;
	}
	////////////////////////////////////////////////////////////
	if(!m_XmlDOM.FindElem( MESSAGEFLAG ) )
		return;
	m_XmlDOM.IntoElem();

	// Do parse stuff
	if( !m_XmlDOM.FindElem( MESSAGEHEADER ) )
		return;
	UINT32 MessageCode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECODE ));
	if(MessageCode==0)	
		return;
	CString strTime = m_XmlDOM.GetAttrib( MESSAGETIME );
	UINT32 BeReturn = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGERETURN ));

	if(BeReturn!=1)
	{
		Clog( LOG_DEBUG, _T("SRM Connect error- There are a few error in connecting DSA serivice !"));
		return ;
	}
	SessionID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( SESSION ));
	int nCommandID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( COMMANDID ));
	if(m_CommandList==NULL)
	{
		Clog( LOG_DEBUG, _T("DODKit memory error- m_CommandList is NULL."));
		return ;
	}
	try
	{
		CBaseCommand *receCommand=NULL;
		POSITION pos1=m_CommandList->GetHeadPosition();
		while(pos1)
		{
			CBaseCommand* iteme1=(CBaseCommand *)(m_CommandList->GetNext(pos1));
			if(iteme1)
			{
				if(iteme1->m_nCommandID ==nCommandID)
				{
					receCommand=iteme1;
					break;
				}
			}
		}

		if (receCommand==NULL)
		{
			Clog( LOG_DEBUG, _T("Receive Connect error- The received CommandID was not found in the sendingCommandList."));
			return ;
		}	
		switch(MessageCode)
		{
		case MESSAGECODE_GETSESSIONID:

			// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê5ÔÂ31ÈÕ 10:16:51
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}
			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_GETSESSIONID m_nReturn !=0 in the XML_file"));	
				return;
			}
			receCommand->m_nSessionID=SessionID;
			break;
		case MESSAGECODE_DELETESESSIONID:
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}
			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			break;
		case MESSAGECODE_OPEN:
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}
			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));

			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_OPEN m_nReturn !=0 in the XML_file"));			
			}
			break;
		case MESSAGECODE_STOP:
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_STOP m_nReturn !=0 in the XML_file"));			
			}
			break;
		case MESSAGECODE_CLOSE:
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_CLOSE m_nReturn !=0 in the XML_file"));			
			}

			break;
		case MESSAGECODE_RUN:
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_RUN m_nReturn !=0 in the XML_file"));			
			}
			break;

		case MESSAGECODE_GETPORTSTATE:
			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 <0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_RUN m_nReturn <0 in the XML_file"));			
			}
			break;
		case MESSAGECODE_ENABLECHANNEL:

			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_ENABLECHANNEL m_nReturn !=0 in the XML_file"));			
			}
			break;
		case MESSAGECODE_FORCEREDETECTED:

			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_UPDATECATALOGNAME m_nReturn !=0 in the XML_file"));			
			}

			break;
		case MESSAGECODE_SETCATALOGNAME:

			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}

			receCommand->m_nReturn1 = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Return" ));
			if (receCommand->m_nReturn1 !=0)
			{
				Clog( LOG_DEBUG, _T("Receive MESSAGECODE_SETCATALOGNAME m_nReturn !=0 in the XML_file"));			
			}

			break;

		case MESSAGECODE_GETPORT:

			if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			{
				Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
				return ;
			}
			receCommand->m_strReturn1 = m_XmlDOM.GetSubDoc();		
			break;
		default:
			Clog( LOG_DEBUG, _T("There are SRM Connect error- ! Message code Error."));
			return ;
		}
		receCommand->m_nRetuenCommand=nCommandID;

	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("Parse message exception.errorcode=%d"),GetLastError());
	}
}

CString CDeviceInfoParser::GetCurrDateTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}