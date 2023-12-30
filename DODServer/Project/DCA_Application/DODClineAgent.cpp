// DODClineAgent.cpp: implementation of the CDODClineAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODClineAgent.h"
#include "clog.h"
#include "Markup.h"
#include "FileFindExt.h"
//
#include <list>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//CDODClineAgent *g_pDCAgent;


BOOL g_bNotInternalTest;
CDODClineAgent::CDODClineAgent()
{

}

CDODClineAgent::~CDODClineAgent()
{
}
BOOL CDODClineAgent::Destroy()
{
	UnInitialize();
	return TRUE;
}
BOOL CDODClineAgent::create()
{
	g_bNotInternalTest=true;
	m_Parse=NULL;
	m_pPortManager=NULL;
	Initialize();	
	return TRUE;
}

BOOL CDODClineAgent::Pause()
{
	return TRUE;
}
BOOL CDODClineAgent::Resume()
{
	return TRUE;
}

void CDODClineAgent::Initialize()
{
	Clog( LOG_DEBUG,_T("CDODClineAgent::Initialize().") );

	CMarkup m_XmlDOM;   
	//////////////////////////////////////////////////////
//	Clog( LOG_DEBUG, _T("Load File: dca.xml"));
	m_XmlDOM.Load( "dca.xml" );

	m_XmlDOM.ResetPos();

	if(!m_XmlDOM.FindElem("DCAConfiguration"))
	{
		Clog( LOG_DEBUG,_T("Info - CDODClineAgent::Parse() - !FindElem(DCAConfiguration).") );
		return;
	}

	m_XmlDOM.IntoElem();

	// Do parse stuff
	if( !m_XmlDOM.FindElem( "DODJBOSS" ) )
	{
		Clog( LOG_DEBUG,_T("Info - CDODClineAgent::Parse() - !FindElem(DODJBOSS).") );
		return;
	}
	//PPsortInfo xxxinfo;
	CString sTmp = _T("");
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
	//_tcscpy( xxxinfo.cAgentServerIp, sTmp.GetBuffer( 0 ) );	
	int nport= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));

	CString m_ProviderValue;
	
	m_ProviderValue.Format(":%d",nport);

	m_ProviderValue=sTmp+m_ProviderValue;

	m_Configqueue = m_XmlDOM.GetAttrib("ConfigQueueName");

	if( !m_XmlDOM.FindElem( "SRMLocation" ) )
	{
		Clog( LOG_DEBUG,_T("Info - CDODClineAgent::Parse() - !FindElem(SRMLocation).") );
		return;
	}
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
	_tcscpy( m_portInfoSRM.cSrmtIp, sTmp.GetBuffer( 0 ) );
	
	m_portInfoSRM.wSrmPort = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));

 //	Clog( LOG_DEBUG, _T("Load File: %d"),m_portInfoSRM.wAgentServerPort);

	m_pPortManager=new CPortManager(); 

	m_pPortManager->m_kit->setSRMAdd(m_portInfoSRM.cSrmtIp,m_portInfoSRM.wSrmPort);

	Clog( LOG_DEBUG,_T("CDODClineAgent::DevKit star.") );

	if(g_bNotInternalTest)
	{
	
     CString  m_strQueueConnectionFactoryName="${queue.connectionfactory.name}";

     CString  m_strTopicConnectionFactoryName="${topic.connectionfactory.name}";

     CString m_Providerkey="java.naming.provider.url";
    
	 CString  m_FactoryInitialKey="java.naming.factory.initial";

     CString m_FactoryInitialValue="org.jnp.interfaces.NamingContextFactory";

     CString  m_FactoryurlKey="java.naming.factory.url.pkgs";

     CString  m_FactoryurlValue="org.jboss.naming";
	  
     CString  m_QueueConnectionFactoryKey="queue.connectionfactory.name";

     CString m_QueueConnectionFactoryValue="ConnectionFactory";

     CString m_TopicConnectionFactoryKey="topic.connectionfactory.name";
     
	 CString m_TopicConnectionFactoryValue="ConnectionFactory";

	 Clog( LOG_DEBUG,_T("CDODClineAgent::m_jms.Initialize") );

	  if(m_jms.Initialize(m_Providerkey,m_ProviderValue, 
	                 m_FactoryInitialKey,m_FactoryInitialValue,
                     m_QueueConnectionFactoryValue,
                     m_TopicConnectionFactoryValue,
                     m_FactoryurlKey,m_FactoryurlValue)==0)
	  {
		  Clog( LOG_DEBUG,_T("Connect JBOSS sucessful.") );
	  }
	  else
		  Clog( LOG_DEBUG,_T("Connect JBOSS error.") );


		m_Parse=new  CJMSParser(&m_jms);
		m_Parse->m_pm=m_pPortManager;
	//	m_Configqueue="queue/testQueue";
		
		Clog( LOG_DEBUG,_T("CDODClineAgent::m_jms.AddOneSendQueue") );
		m_jms.AddOneSendQueue(m_Configqueue.GetBuffer(0));

		CJMSMessage message(&m_jms,"COMMAND","3001");
		CString ssaa=m_Parse->SendGetConfig();
		message.SetRawData(ssaa.GetBuffer(0));
		m_jms.SyncSend(&message,m_Configqueue.GetBuffer(0),m_Parse,5);
		Clog( LOG_DEBUG,_T("CDODClineAgent::SyncSend config messagene") );

		if(1>(int)(m_pPortManager->m_PortVector.size()))
		{
			Clog( LOG_DEBUG,_T("Parse megServer error.") );
			return;
		}

		sTmp="queue/"+m_pPortManager->m_Localqueuename;
		m_jms.AddOneReceiveQueue(sTmp.GetBuffer(0),m_Parse);

		CPORTVECTOR::iterator iter;

		for(iter=m_pPortManager->m_PortVector.begin();iter!=m_pPortManager->m_PortVector.end();iter++)
		{

			CDODPort *pPort=*iter;

			if(pPort==NULL)
			{
				return ;
			}

			for(int i=0;i<pPort->m_ChannelNumber;i++)
			{
				CDODChannel *channel=pPort->m_channel[i];
				if(channel==NULL)
				{
					return ;
				}	
				sTmp="queue/"+channel->m_QueueName;
				m_jms.AddOneSendQueue(sTmp.GetBuffer(0));
				Clog( LOG_DEBUG,_T("CDODClineAgent::AddOneSendQueue ,each channel->queuename") );

			}
		}
	}	

	SetMessage();

}


void CDODClineAgent::Stop()
{	
	Clog( LOG_DEBUG,_T("CDODClineAgent Stop ") );

	if (m_pPortManager)
	{	m_pPortManager->Stop();

	}
}

void CDODClineAgent::UnInitialize()
{
	Clog( LOG_DEBUG,_T("CDODClineAgent UnInitialize ") );

	if(m_pPortManager)
	{
		delete m_pPortManager;
		m_pPortManager=NULL;
	}
	
	if(m_Parse)
	{
		delete m_Parse;
		m_Parse=NULL;
	}
}
void CDODClineAgent::Updatecatelog()
{
	Clog( LOG_DEBUG,_T("CDODClineAgent Updatecatelog ") );

	m_pPortManager->UpdateCatalog();
}

int  CDODClineAgent::EnableChannel(BOOL bEnable)
{
	return 	m_pPortManager->EnableChannel(bEnable);
}

CString CDODClineAgent::GetPort()
{

	return m_pPortManager->GetPort();
}
void CDODClineAgent::ClosePort()
{
	Clog( LOG_DEBUG,_T("CDODClineAgent ClosePort ") );

	m_pPortManager->ClosePort();
}

int CDODClineAgent::GetState(int index)
{
	return m_pPortManager->GetState(index);
}


void CDODClineAgent::SetMessage()
{
	//
	//TCHAR szRemote[MAX_PATH] = _T("\\\\");
	//_tcscat( szRemote, "192.168.80.81" );
	//
	//NETRESOURCE tmpNetRes;
	//tmpNetRes.dwType = RESOURCETYPE_ANY;
	//tmpNetRes.lpLocalName = NULL;
	//tmpNetRes.lpRemoteName = szRemote;
	//tmpNetRes.lpProvider = NULL;
	//
	//CString m_sUserName="administrator";
	//CString m_sPasswd="cdci";
	//if(0)
	////if( WNetAddConnection2( &tmpNetRes, m_sUserName, m_sPasswd, CONNECT_UPDATE_PROFILE ) != NO_ERROR )
	//{
	//	Clog( LOG_DEBUG, _T("Auto logon the remote server... IP Address: 192.168.80.81"));
	//	return ;
	//}
	//
	
//	CJMSParser pp;

//		CMarkup m_XmlDOM;
//	Clog( LOG_DEBUG, _T("Load File: dca.xml"));
//	m_XmlDOM.Load( "DOD_JMS_response.xml" );
//	m_XmlDOM.Load( "msgconfig.xml" );

	
//	m_XmlDOM.ResetPos();
//	CString sstr="rrr";		
//	sstr = m_XmlDOM.GetDoc();
//	TRACE("sdfsdf==%d\n",sstr.GetLength());
//	str.Format("\n%d",sstr.GetLength()); 	
//	TRACE(str);	
//	TRACE(sstr.GetBuffer(0)); 

//	m_Parse->ReceiverPortConfigMsg(sstr.GetBuffer(0),0,&m_comeIP);

	if(1>(int)(m_pPortManager->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("Parse megServer error.") );
		return;
	}

	CString        strCurDir,sTmp;
	char           sModuleName[1025];
	DWORD dSize = GetModuleFileName(AfxGetApp()->m_hInstance,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"
 //m_logFileLocation=strCurDir + "TaskEngine.log";
//	strCurDir="D:\\work\\DODDevKit";
	m_pPortManager->m_path=strCurDir; 
	
	Clog( LOG_DEBUG,_T("CDODClineAgent ApplyParameter star ") );

	m_pPortManager->ApplyParameter(m_Parse);

	CPORTVECTOR::iterator iter;

	for(iter=m_pPortManager->m_PortVector.begin();iter!=m_pPortManager->m_PortVector.end();iter++)
	{

		CDODPort *pPort=*iter;

		if(pPort==NULL)
		{
			return ;
		}

		for(int i=0;i<pPort->m_ChannelNumber;i++)
		{
			CDODChannel *channel=pPort->m_channel[i];
			if(channel==NULL)
			{
				return ;
			}	

			strCurDir=m_Parse->SendGetFullDateMsg(pPort->m_nGroupID,channel->m_nDataType,0);

			CJMSMessage message(&m_jms,TRUE,0,MESSAGEMODE_PTOP);

			message.SetStringProperty("MESSAGECLASS","NOTIFICATION");

			message.SetRawData(strCurDir.GetBuffer(0));
			sTmp="queue/"+channel->m_QueueName;
			m_jms.PtopSend(&message,sTmp.GetBuffer(0));

			Clog( LOG_DEBUG,_T("CDODClineAgent m_jms.PtopSend :each channel sendfulldata ") );

		}
	}
}