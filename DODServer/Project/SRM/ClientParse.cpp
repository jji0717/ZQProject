/*****************************************************************************
File Name:     Parse.cpp
Author:        haiping.wan
Security:      SEACHANGE SHANGHAI
Description:   implements class CParse
Function Inventory: 
Modification Log:
When           Version        Who						What
------------------------------------------------------------------------------
2005/04/22     1.0            haiping.wan					Created
*******************************************************************************/
#include "StdAfx.h"
#include"ClientParse.h"

#include"SessionManager.h"
#include"scqueue.h"
#include"Markup.h"
#include "MessageMacro.h"
CClientParse::CClientParse(CTCPConnection* pConnection)
{
	m_Connection=pConnection;
}

CClientParse::~CClientParse(void)
{
}
void CClientParse::SetSessionManager(CSessionManager * pManager)
{
	try
	{
		m_pSessionManager=pManager;
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientParse::SetSessionManager") );
	}

}
void CClientParse::Parse(CSCMemoryBlockPtr block)
{
	try
	{
		//printf("receive\n");
		DWORD SessionID;
		if( block->GetSize() <= 0 )
		{
			return;
		}
		CMarkup m_XmlDOM;

		//Clog(LOG_DEBUG,"Receive From Client");
		//Clog( LOG_DEBUG, CString(block->GetBlock(), block->GetSize()) );
		if(!m_XmlDOM.SetDoc(CString(block->GetBlock(), block->GetSize())))
		{
			Clog( LOG_DEBUG, _T("Info - CDSAParse::Parse() - !Parse Message Error."));
			return;
		}
		Clog( 0, _T("%s \r\n"), m_XmlDOM.GetDoc() );

		////////////////////////////////////////////////////////////
		if(!m_XmlDOM.FindElem( MESSAGEFLAG ) )
		{
			return;
		}
		m_XmlDOM.IntoElem();
		// Do parse stuff
		if( !m_XmlDOM.FindElem( MESSAGEHEADER ) )
		{
			return;
		}

		UINT32 MessageCode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECODE ));
		CString strTime = m_XmlDOM.GetAttrib( MESSAGETIME );
		UINT32 BeReturn = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGERETURN ));
		SessionID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( SESSION ));
		int CommandID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( COMMANDID ));
		if(MessageCode==0)
		{
			// Clog( LOG_DEBUG, _T("heart beat") );
			return;
		}
		//////////////////////////////////////////////////////////////////////////////
		else if(MessageCode==MESSAGECODE_OPEN)
		{
			Clog(LOG_DEBUG,"Receive From Client-message open");
		//	Clog( LOG_DEBUG, CString(block->GetBlock(), block->GetSize()) );
			//Request a sessionID
			CMarkup returnXmlDOM;
			//tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("1") ); 
			CDSAResource* DSAResource=m_pSessionManager->m_pResourceManager->SelectAResource();
			if(DSAResource!=NULL)
			{
				SESSION_STATUS state;
				state=normal;
				DWORD ID=m_pSessionManager->GenerateSessionID();
				CDODSession session(m_Connection,DSAResource,ID,state);
				m_pSessionManager->AddASession(session);

				// CMarkup tmpXmlDOM;
				returnXmlDOM.AddElem( _T("Message") );
				returnXmlDOM.IntoElem();
				CString sessionid,commandid;
				sessionid.Format("%d",ID);
				commandid.Format("%d",CommandID);
				returnXmlDOM.AddElem( _T("MessageHeader") );
				returnXmlDOM.AddAttrib( _T("MessageCode"), _T("2001") );  
				returnXmlDOM.AddAttrib( _T("SessionID"),sessionid );
				returnXmlDOM.AddAttrib( _T("CommandID"),commandid );
				returnXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
				returnXmlDOM.AddAttrib( _T("BeReturn"), _T("1") ); 

				//added by whp 5.30
				returnXmlDOM.AddElem( _T("MessageBody") );
				returnXmlDOM.AddAttrib( _T("Return"), _T("0") ); 


				Clog( LOG_DEBUG, _T("Create Session succsessfully") );
			}
			else
			{
				// CMarkup tmpXmlDOM;
				returnXmlDOM.AddElem( _T("Message") );
				returnXmlDOM.IntoElem();
				CString sessionid,commandid;
				sessionid.Format("%d",SessionID);
				commandid.Format("%d",CommandID);
				returnXmlDOM.AddElem( _T("MessageHeader") );
				returnXmlDOM.AddAttrib( _T("MessageCode"), _T("2001") );  
				returnXmlDOM.AddAttrib( _T("SessionID"),sessionid );
				returnXmlDOM.AddAttrib( _T("CommandID"),commandid );
				returnXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
				//tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
				returnXmlDOM.AddAttrib( _T("BeReturn"), _T("1") ); 

				//added by whp 5.30
				returnXmlDOM.AddElem( _T("MessageBody") );
				returnXmlDOM.AddAttrib( _T("Return"), _T("1") ); 

				Clog( LOG_DEBUG, _T("fail to Create Session succsessfully can't get a resource") );
			}
			CString sTmp = returnXmlDOM.GetDoc();
			int length = sTmp.GetLength();
			if(m_Connection!=NULL)
			{
				m_Connection->SendData(sTmp.GetBuffer(0),length);
			}
		}
		//////////////////////////////////////////////////////////////////////////////
		else if(MessageCode==MESSAGECODE_CLOSE)
		{
			//delete a session
			//Parse XML Get SessionID and delete the session 
			Clog(LOG_DEBUG,"Receive From Client-message close");
		//	Clog( LOG_DEBUG, CString(block->GetBlock(), block->GetSize()) );

			bool ret= m_pSessionManager->DeleteSession(SessionID);
			/////////////////////////
			CMarkup deleteXmlDOM;
			deleteXmlDOM.AddElem( _T("Message") );
			deleteXmlDOM.IntoElem();
			CString sessionid,commandid;
			sessionid.Format("%d",SessionID);
			commandid.Format("%d",CommandID);
			deleteXmlDOM.AddElem( _T("MessageHeader") );
			deleteXmlDOM.AddAttrib( _T("MessageCode"), _T("2002") );  
			deleteXmlDOM.AddAttrib( _T("SessionID"),sessionid );
			deleteXmlDOM.AddAttrib( _T("CommandID"),commandid );
			deleteXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
			//tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
			deleteXmlDOM.AddAttrib( _T("BeReturn"), _T("1") ); 

			//added by whp 5.30
			deleteXmlDOM.AddElem( _T("MessageBody") );
			if(ret)
			{
				deleteXmlDOM.AddAttrib( _T("Return"), _T("0") ); 
			}
			else
			{
				deleteXmlDOM.AddAttrib( _T("Return"), _T("1") ); 
			}
			CString sTmp = deleteXmlDOM.GetDoc();
			int length = sTmp.GetLength();
			if(m_Connection!=NULL)
			{
				m_Connection->SendData(sTmp.GetBuffer(0),length);
			}

			//Clog( LOG_DEBUG, _T("delete session %d",SessionID) );
		}
		//////////////////////////////////////////////////////////
		else
		{
			Clog(LOG_DEBUG,"Receive From Client-message else");

			Clog( LOG_DEBUG, CString(block->GetBlock(), block->GetSize()) );

			char* data=block->GetBlock();
			int length=block->GetSize();
			CDSAResource *pDSAResource;
			pDSAResource=m_pSessionManager->GetDSResource(SessionID);
			if(pDSAResource!=NULL)
			{
				if(pDSAResource->GetState())
				{
					pDSAResource->GetConnection()->SendData(data,length);
					Clog( LOG_DEBUG, _T("begin to Transfer message from client") );
					return;
				}
			}
			Clog( LOG_DEBUG, _T("fail to Transfer message from client") );
			CMarkup errxmlDOM;
			errxmlDOM.AddElem( _T("Message") );
			errxmlDOM.IntoElem();
			CString sessionid,commandid,messagecode;
			messagecode.Format("%d",MessageCode);
			sessionid.Format("%d",SessionID);
			commandid.Format("%d",CommandID);
			errxmlDOM.AddElem( _T("MessageHeader") );
			errxmlDOM.AddAttrib( _T("MessageCode"),messagecode );  
			errxmlDOM.AddAttrib( _T("SessionID"),sessionid );
			errxmlDOM.AddAttrib( _T("CommandID"),commandid );
			errxmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
			//tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
			errxmlDOM.AddAttrib( _T("BeReturn"), _T("1") ); 

			//added by whp 5.30
			errxmlDOM.AddElem( _T("MessageBody") );
			errxmlDOM.AddAttrib( _T("Return"), _T("1") ); 

			CString sTmp = errxmlDOM.GetDoc();
			int length1 = sTmp.GetLength();
			if(m_Connection!=NULL)
			{
				m_Connection->SendData(sTmp.GetBuffer(0),length1);
			}
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientParse::Parse") );
	}

	/////////////////////////////////////////////////////////////////////////
}
CString CClientParse::GetCurrDateTime()
{
	try
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		CString sTime;
		sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
		return sTime;
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientParse::GetCurrDateTime") );
	}
	return "";
}
void CClientParse::SetConnection(CTCPConnection* pConnection)
{
	try
	{
		m_Connection=pConnection;
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientParse::SetConnection") );
	}
}
////////////////////////////////////////////////////////////////////////
CDSAParse::CDSAParse(CDSAResource* pDSAParse)
{
	m_Connection=pDSAParse;
}
CDSAParse::~CDSAParse(void)
{
}
void CDSAParse::SetSessionManager(CSessionManager * pManager)
{
	try
	{
		m_pSessionManager=pManager;
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAParse::SetSessionManager") );
	}
}
void CDSAParse::Parse(CSCMemoryBlockPtr block)
{
	try
	{
		//Parse XML Get SessionID
		//constrct the send block pFred
		DWORD SessionID;
		if( block->GetSize() <= 0 )
			return;

		//Clog(LOG_DEBUG,"Receive From Server");
		// Clog( LOG_DEBUG, CString(block->GetBlock(), block->GetSize()) );
		TCHAR sTmp[128] = {0};
		CMarkup m_XmlDOM;
		if(!m_XmlDOM.SetDoc(CString(block->GetBlock(), block->GetSize())))
		{
			Clog( LOG_DEBUG, _T("Info - CDSAParse::Parse() - !Parse Message Error."));
			return;
		}
		//TRACE( _T("%s \r\n"), CString(block->GetBlock(), block->GetSize()) );
		TRACE( _T("%s \r\n"), m_XmlDOM.GetDoc() );
		if(!m_XmlDOM.FindElem( MESSAGEFLAG ) )
			return;
		m_XmlDOM.IntoElem();
		// Do parse stuff
		if( !m_XmlDOM.FindElem( MESSAGEHEADER ) )
			return;
		UINT32 MessageCode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECODE ));
		CString strTime = m_XmlDOM.GetAttrib( MESSAGETIME );
		UINT32 BeReturn = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGERETURN ));
		SessionID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( SESSION ));
		int CommandID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( COMMANDID ));
		//if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
		//return;
		char* data=block->GetBlock();
		int length=block->GetSize();
		if(MessageCode!=0)
		{
			Clog(LOG_DEBUG,"Receive From Server");
			Clog( LOG_DEBUG, CString(block->GetBlock(), block->GetSize()) );
			CTCPConnection *pConnection;
			pConnection=m_pSessionManager->GetConnection(SessionID);
			if(pConnection!=NULL)
			{
				if(pConnection->GetState())
				{
					pConnection->SendData(data,length);
					Clog( LOG_DEBUG, "begin to transfer the message from Server" );
				}
			}
			else
			{
				Clog( LOG_DEBUG, "fail to transfer themessage fron Server" );
			}
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAParse::Parse") );
	}
}
void CDSAParse::SetDSAResource(CDSAResource* pConnection)
{
	try
	{
		m_Connection=pConnection;
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAParse::SetDSAResource") );
	}
}



