// Parser.cpp: implementation of the CJMSParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Parser.h"
#include "DODClientAgent.h"
#include "clog.h"
#include "Markup.h"
#include "messagemacro.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#import "msxml3.dll"
//#import "msxml4.dll"
using namespace MSXML2;

extern BOOL g_bNotInternalTest;

typedef CDODChannel* CDODCHANNELARRAY;
typedef ZQSBFIPPORTINF* ZQSBFIPPORTINFARRAY;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJMSParser::~CJMSParser(void)
{

}

CJMSMessage  CJMSParser::ParseCommand(CJMSMessage* pMessage)
{
	CString data=pMessage->GetRawData();
	Clog( LOG_DEBUG,_T("CJMSMessage ParseCommand") );
	Clog( LOG_DEBUG,data);
// these codes will used to reply JMSCommand .but now the DCA project was not used it;

	CJMSMessage destmessage(m_pActiveJMS,TRUE,0,MESSAGEMODE_PTOP);
	CString replystring="reply ";
	//replystring+=data;
    destmessage.SetStringProperty("MESSAGECLASS","NOTIFICATION");

	CMarkup m_XmlDOM;
	m_XmlDOM.AddElem(DODMESSAGEFLAG);

	m_XmlDOM.IntoElem();
	m_XmlDOM.AddElem(DODMESSAGEHEADER );	
	m_XmlDOM.AddAttrib( DODMESSAGETIME, GetCurrDateTime()); 
	m_XmlDOM.AddAttrib( DODMESSAGECODE, FULLDATAWITHTCP);

	m_XmlDOM.IntoElem();
	m_XmlDOM.AddElem( DODMESSAGEBODY );	
	m_XmlDOM.AddAttrib( CONTENTRESPONSECODE,1);

	m_XmlDOM.IntoElem();
	m_XmlDOM.AddElem( DODMESSAGECONTENT );	
	
	int nMessageID=0;
	m_XmlDOM.AddAttrib( MESSAGEMESSAGEID, nMessageID);

	replystring=m_XmlDOM.GetDoc();
	destmessage.SetRawData(replystring.GetBuffer(0));


	m_XmlDOM.RestorePos();
	m_XmlDOM.SetDoc(data);
	if(!m_XmlDOM.FindElem(DODMESSAGEFLAG) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- FindElem(DODMESSAGEFLAG).") );
		return destmessage;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODMESSAGEHEADER).") );
		return destmessage;
	}
	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	CString temp=m_XmlDOM.GetDoc();
	switch(messagecode) 
	{	
	case FULLDATAWITHTCP:

		Clog( LOG_DEBUG,_T("ReceiverMsg  : FullData with TCP ") );
		ReceiverDataTCP(temp.GetBuffer(0),0);
		break;

	case UPDATEINTCPMODE:

		Clog( LOG_DEBUG,_T("ReceiverMsg  : Channel Data Update in TCP mode ") );
		ReceiverUpDataTCP(temp.GetBuffer(0),0);
		break;
	}
	return destmessage;

}
int CJMSParser::ParseStatus(CJMSMessage* pMessage)
{
	   CString data=pMessage->GetRawData();
	   Clog( LOG_DEBUG,_T("CJMSMessage ParseStatus") );
	  Clog( LOG_DEBUG,data);
	   return 0;
}
int CJMSParser::parseNotification(CJMSMessage* pMessage)
{
	CString data=pMessage->GetRawData();
	Clog( LOG_DEBUG,_T("CJMSMessage parseNotification") );
	Clog( LOG_DEBUG,data);
	CMarkup m_XmlDOM;

	m_XmlDOM.SetDoc(data);

	if( m_XmlDOM.FindElem( CONTENTCONFRESPONSE ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- FindElem(CONTENTCONFRESPONSE).") );
		ReceiverPortConfigMsg(data.GetBuffer(0),0,NULL);	
		return 1;
	}

	if(!m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- FindElem(DODMESSAGEFLAG).") );
		return 1;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	CString temp=m_XmlDOM.GetDoc();
	switch(messagecode) 
	{	
	case PORTCONFIGURATION:
	
		Clog( LOG_DEBUG,_T("ReceiverMsg  : portconfiguration ") );
		ReceiverPortConfigMsg(temp.GetBuffer(0),0,NULL);		
		break;
	case FULLDATAWITHSHAREDFOLDERS:

		Clog( LOG_DEBUG,_T("ReceiverMsg  : full data with shared folders ") );
		ReceiverDataFolder(temp.GetBuffer(0),0);
		break;
	case UPDATEINSHAREFOLDERMODE:

		Clog( LOG_DEBUG,_T("ReceiverMsg  : Channel Data Update in Share-Folder mode ") );
		ReceiverUpDataFolder(temp.GetBuffer(0),0);
		break;
	default:
		
		Clog( LOG_DEBUG,_T("ReceiverMsg   unknowable message code identifier") );
		return 1;
	}
	return 0;
}

CString CJMSParser::GetCurrDateTime()
{
	SYSTEMTIME time; 
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}

CString CJMSParser::SendGetFullDateMsg(int id1,int id2,int id3)
{
	Clog( LOG_DEBUG,_T("CJMSParser::SendGetFullDateMsg") );

	CString str;
	CMarkup m_XmlDOM;
	
	m_XmlDOM.AddElem(DODMESSAGEFLAG );
	m_XmlDOM.IntoElem();
	
	m_XmlDOM.AddElem( DODMESSAGEHEADER );	


	m_XmlDOM.AddAttrib( DODMESSAGETIME, GetCurrDateTime()); 

	str.Format("%d",GETFULLDATA);
	m_XmlDOM.AddAttrib( DODMESSAGECODE, str);


	m_XmlDOM.AddElem( DODMESSAGEBODY );		
/*	str.Format( _T("%d"), id1 );
	m_XmlDOM.AddAttrib( _T("PortID"),str);	

	str.Format( _T("%d"), id2 );
	m_XmlDOM.AddAttrib( _T("ChannelID"),str);	*/
	m_XmlDOM.AddAttrib( MESSAGEDATATYPE,id2);	
	m_XmlDOM.AddAttrib( MESSAGEGROUPID,id1);	


	str=m_XmlDOM.GetDoc();
	return str;
}


CString CJMSParser::SendGetConfig()
{
	Clog( LOG_DEBUG,_T("CJMSParser::SendGetConfig") );

	CString str="";	

	CMarkup m_XmlDOM;	
	m_XmlDOM.AddElem( MESSAGECONFREQUEST );	
	m_XmlDOM.AddAttrib( MESSAGECONFREQUESTKEY, 0); 
	m_XmlDOM.AddAttrib( MESSAGEGROUPIDAPPNAME, CONTENTMESSAGEDOD); 
	m_XmlDOM.AddAttrib( MESSAGETYPE, 3); 
	m_XmlDOM.AddAttrib(MESSAGEFORMAT, 2); 
	str=m_XmlDOM.GetDoc();
	return str;
}

BOOL CJMSParser::FindOutPortIndexAndChannelIndex(int dataType,int GroupID,CDODPort **portaddress,int &ChannelIndex)
{
	if (m_pm==NULL)
	{
		return FALSE;
	}

	if(1>(int)(m_pm->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("CJMSParser::FindOutPortIndexAndChannelIndex Parse megServer error.") );
		return FALSE;
	}

	CPORTVECTOR::iterator iter;

	for(iter=m_pm->m_PortVector.begin();iter!=m_pm->m_PortVector.end();iter++)
	{
		CDODPort *pPort=(*iter);
		if(pPort==NULL)
		{
			return FALSE;
		}

		if (GroupID != pPort->m_nGroupID)
			continue;

		for(int i=0;i<pPort->m_ChannelNumber;i++)
		{
			CDODChannel *channel=pPort->m_channel[i];
			if(channel==NULL)
			{
				return FALSE;
			}
			if (dataType == channel->m_nDataType)
			{
				*portaddress=pPort;
				ChannelIndex=i;
				return TRUE;
			}
		}
	}

	return FALSE;
}

int CJMSParser::ReceiverDataFolder(char *buf,int buffersize)
{
	CString str=buf;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverDataFolder") );
	Clog( LOG_DEBUG,str);

	str="";
		
	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEFLAG).") );
		return DODRECEIVERDATAFOLDERERROR;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port=0;
	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib(DODMESSAGECODE));

	if (messagecode !=FULLDATAWITHSHAREDFOLDERS)
		return DODRECEIVERDATAFOLDERERROR;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}
	int channelID=0; int channelIndex=0;
	channelID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE ));
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(channelID,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- MESSAGEDATATYPE or MESSAGEGROUPID error") );
		return 1;
	}

	int UpdateMode=0;
	m_XmlDOM.IntoElem();
	if( !m_XmlDOM.FindElem(CONTENTFILEOPERATION) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(CONTENTFILEOPERATION).") );
		return 1;
	}
	sTmp = m_XmlDOM.GetAttrib(CONTENTROOT);
	UpdateMode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib(CONTENTUPDATEMODE));
	CString subPath="";
	if (UpdateMode==0)
	{
		//new full data location
		teport->m_channel[channelIndex]->MoveAllFile(sTmp,Port,sTmp);
	}

	else
	{
		m_XmlDOM.IntoElem();

		if( !m_XmlDOM.FindElem() )
		{
			Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(CONTENTFILE).") );
			return 1;
		}
		subPath = m_XmlDOM.GetAttrib(CONTENTPATH);
		switch(UpdateMode)
		{
		case 1:
			// the sub folder to be updated
			teport->m_channel[channelIndex]->UpdateSubFile(subPath,Port,sTmp);		
			break;
		case 2:
			//the sub folder to be deleted
			teport->m_channel[channelIndex]->DeleteSubFile(subPath,Port,sTmp);	
			break;
		case 3:
			// the file with full path to be deleted
			teport->m_channel[channelIndex]->DeleteFullPath(sTmp,Port,sTmp);	
			break;
		case 4:
			// is the file with full path to be Modofied
			teport->m_channel[channelIndex]->FullPathModofied(sTmp,Port,sTmp);	

			break;
		case 5:
			// the file with full path to be newed
			teport->m_channel[channelIndex]->FullPathToNew(sTmp,Port,sTmp);	
			break;

		default:
			break;
		}
	}	
	
	//m_pm->m_pPort[Port]->m_channel[channelIndex]->MoveAllFile(sTmp,Port,sTmp);
	
	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,teport->m_nID,teport->m_channel[channelIndex]->m_channelID);
	return 0; 
}

int CJMSParser::ReceiverDataTCP(char *buf,int buffersize)
{
	CString str;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverDataFolder") );
	//Clog( LOG_DEBUG,str);

	//str="";

	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem(DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEFLAG).") );
		return 1;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port= 0;

	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	if (messagecode !=FULLDATAWITHTCP)
		return 1;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}

	int channelID=0; int channelIndex=0;
	channelID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE ));
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(channelID,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- MESSAGEDATATYPE or MESSAGEGROUPID error") );
		return 1;
	}

	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGECONTENT))
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DataList).") );
		return 1;
	}
	CString filename;
	filename=m_XmlDOM.GetAttrib( "Destination");
	int leafTime=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ExpiredTime"));
	int nMessageID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEMESSAGEID));
	int OperationCode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "OperationCode"));
	int datatype=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE));
	/*
	int count= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "BlockCount" ));
	if(count<1)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- BlockCount <1.") );
		return 1;
	}

	m_XmlDOM.IntoElem();
	if( !m_XmlDOM.FindElem( "PayLoad" ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(PayLoad).") );
		return 1;
	}
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
*/
	sTmp=m_XmlDOM.GetSubDoc();
	teport->m_channel[channelIndex]->CreateMsgFile(sTmp,filename,leafTime,nMessageID,OperationCode,datatype);


	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,0,0);
	return 0;
}


int CJMSParser::ReceiverPortConfigMsg(char *buf,int buffersize,PPsortInfo *info)
{
	CString xmlstr;
	CString sTmp = _T("");
	BOOL bReturn=FALSE;

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg") );

	CString name;
	MSXML2::IXMLDOMDocumentPtr pDoc; 
	MSXML2::IXMLDOMElementPtr  childNode ;
	MSXML2::IXMLDOMNodePtr       pNode = NULL;
	MSXML2::IXMLDOMNodePtr		 pTmp = 0;
	MSXML2::IXMLDOMNodePtr		 pFather = 0;
	MSXML2::IXMLDOMNodePtr		 pSon = 0;
	MSXML2::IXMLDOMNamedNodeMapPtr pNamedNode = 0;
	COleVariant nodeValue;
	MSXML2::IXMLDOMNodeListPtr childlist; 
	xmlstr.Format("%s",buf);

	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))    
	{		//AfxMessageBox("无法创建DOMDocument对象，请检查是否安装了MS XML Parser 运行库!"); 
	//	AfxMessageBox(" IXMLDOMDocumentPtr::CreateInstance error !"); 
		return 1;
	}

//The follow codes was used the programer when pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument));

	//pDoc->PutvalidateOnParse(VARIANT_FALSE);
	//pDoc->PutresolveExternals(VARIANT_FALSE);
	//pDoc->PutpreserveWhiteSpace(VARIANT_TRUE);

	if(pDoc->loadXML((_bstr_t)CString(xmlstr))!=VARIANT_TRUE)
	{
		return FALSE;
	}

	//find "CONTENTCONFRESPONSE"
	childNode = (MSXML2::IXMLDOMElementPtr)(pDoc->selectSingleNode(CONTENTCONFRESPONSE));
    
	pNode = childNode->GetfirstChild();
	name = CString((wchar_t*)pNode->GetnodeName());
	if(name.Compare("ListOfNode")!=0)
	{
		Clog( LOG_DEBUG,_T("ListOfNode is not found") );
		return 1;
	}

	pNode = pNode->GetfirstChild();
	name = CString((wchar_t*)pNode->GetnodeName());
	if(name.Compare("DCAPortConfiguration")!=0)
	{
		Clog( LOG_DEBUG,_T("DCAPortConfiguration is not found") );
		return 1;
	}

		pNamedNode = pNode->Getattributes();
	if(pNamedNode==NULL)
		return FALSE;

	//the queuename is existed JMS for APP 's configuration

	pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
	if(pTmp==NULL)
		return FALSE;
	nodeValue = pTmp->GetnodeValue();

	m_pm->m_Localqueuename=CString((wchar_t*)nodeValue.bstrVal);

	
	pNode = pNode->GetfirstChild();
	name = CString((wchar_t*)pNode->GetnodeName());
	if(name.Compare("Port")!=0)
	{
		Clog( LOG_DEBUG,_T("Port is not found") );
		return 1;
	}
	
// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月28日 11:08:00
	pNamedNode = pNode->Getattributes();
	if(pNamedNode==NULL)
		return FALSE;
	
	long nchildNumber=0;

	childlist=pNode->GetchildNodes();
	hr=childlist->get_length(&nchildNumber);
	if (nchildNumber <1)
	{
		Clog( LOG_DEBUG,_T("CJMSParser Portnumber is zero") );
		return 1;
	}

	pNode = pNode->GetfirstChild();

	//get all config ports.
	Clog( LOG_DEBUG,_T("CJMSParser::get all portconfigs") );

	try
	{
		while (pNode)
		{
			CDODPort *tempport=new CDODPort;			//get port layer attrib

			tempport->m_strName=CString((wchar_t*)pNode->GetnodeName());
			pNamedNode = pNode->Getattributes();
			if(pNamedNode==NULL)
				return FALSE;

			pTmp = pNamedNode->getNamedItem((_bstr_t)"PMTPID");
			if(pTmp==NULL)
				return FALSE;
			nodeValue = pTmp->GetnodeValue();
			nodeValue.ChangeType(VT_INT);
			tempport->m_nPmtPID = nodeValue.intVal;


			pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEGROUPID));
			if(pTmp==NULL)
				return FALSE;
			nodeValue = pTmp->GetnodeValue();
			nodeValue.ChangeType(VT_INT);
			tempport->m_nGroupID = nodeValue.intVal;

			pTmp = pNamedNode->getNamedItem((_bstr_t)"TotalRate");
			if(pTmp==NULL)
				return FALSE;
			nodeValue = pTmp->GetnodeValue();
			nodeValue.ChangeType(VT_INT);
			tempport->m_nTotalRate = nodeValue.intVal;

			// get all channels and all castports.

			pFather=pNode->GetfirstChild();

			//pNode->get
			while (pFather)
			{

				//BSTR bStr; 
				//CString strrrr;
				//pFather->get_xml(&bStr);
				//strrrr=bStr;
				
				//get child layer attrib,but this layer is possible IPPort or IPChannel.
				name=CString((wchar_t*)pFather->GetnodeName());
				if(name.Compare("IPPort")==0)
				{
					childlist=pFather->GetchildNodes();
					hr=childlist->get_length(&nchildNumber);
					if (nchildNumber <1)
					{
						Clog( LOG_DEBUG,_T("CJMSParser IPPort number is zero") );
						return 1;
					}
					tempport->m_castPort= new ZQSBFIPPORTINFARRAY[nchildNumber];

					pSon=pFather->GetfirstChild();
					
					int k=0;  //This variable show the number of broadcast ouput
					
					while (pSon)
					{
						tempport->m_castPort[k]=new ZQSBFIPPORTINF;
						name=CString((wchar_t*)pSon->GetnodeName());
						pNamedNode = pSon->Getattributes();
						if(pNamedNode==NULL)
							return FALSE;

						pTmp = pNamedNode->getNamedItem((_bstr_t)"SendType");
						if(pTmp==NULL)
							return FALSE;
						nodeValue = pTmp->GetnodeValue();

						//Note:
						//In Configuration Management System Ver0.0.0.1 interface.if user add a note (IPPort'member),but it's IPAddress.Port and SendType are not configured,
						//their value will be saved to SQL as cstring _ fotmat.Then .ChangeType was be called .there are a error dialog (Type matching). 

						nodeValue.ChangeType(VT_INT);
						tempport->m_castPort[k]->wSendType = nodeValue.intVal;

						pTmp = pNamedNode->getNamedItem((_bstr_t)"IPAddress");
						if(pTmp==NULL)
							return FALSE;
						nodeValue = pTmp->GetnodeValue();
						strcpy(tempport->m_castPort[k]->cDestIp,(CString((wchar_t*)nodeValue.bstrVal)).GetBuffer(0));

						pTmp = pNamedNode->getNamedItem((_bstr_t)"IPPort");
						if(pTmp==NULL)
							return FALSE;
						nodeValue = pTmp->GetnodeValue();
						nodeValue.ChangeType(VT_INT);
						tempport->m_castPort[k]->wDestPort = nodeValue.intVal;

						k++;
						pSon = pSon->GetnextSibling();
						// get a same layer about IPPort attrib
					}
					tempport->m_castcount=k;
				}
				else
					if(name.Compare("Channel")==0)
					{
						childlist=pFather->GetchildNodes();
						hr=childlist->get_length(&nchildNumber);
						if (nchildNumber <1)
						{
							Clog( LOG_DEBUG,_T("CJMSParser Channel number is zero") );
							return 1;
						}
						tempport->m_channel= new CDODCHANNELARRAY[nchildNumber];

						tempport->m_ChannelNumber=0;

						pSon=pFather->GetfirstChild();
						int k=0;
						while (pSon)
						{
							name=CString((wchar_t*)pSon->GetnodeName());
							pNamedNode = pSon->Getattributes();
							if(pNamedNode==NULL)
								return FALSE;

							CDODChannel *tempchannel=new CDODChannel();

							pTmp = pNamedNode->getNamedItem((_bstr_t)"Encrypted");
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_bEncrypted = nodeValue.intVal;


							pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamType");
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							nodeValue.ChangeType(VT_INT);
							tempchannel->nStreamType = nodeValue.intVal;


							pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamID");
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nStreamID = nodeValue.intVal;


							pTmp = pNamedNode->getNamedItem((_bstr_t)"Tag");
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							strcpy(tempchannel->m_strTag,(CString((wchar_t*)nodeValue.bstrVal)).GetBuffer(0));

							pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							tempchannel->m_QueueName=CString((wchar_t*)nodeValue.bstrVal);


							pTmp = pNamedNode->getNamedItem((_bstr_t)"ChannelRate");
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nRate = nodeValue.intVal;

							pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEDATATYPE));
							if(pTmp==NULL)
								return FALSE;
							nodeValue = pTmp->GetnodeValue();
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nDataType = nodeValue.intVal;

							int tempmessagecode=0;							

							switch(tempchannel->m_nDataType)
							{
							case DATATYPE_NAVIGATION :
								tempmessagecode=JMSMESSAGE_NAVIGATION;
								break;

							case DATATYPE_CHANNELLINEUP :
								tempmessagecode=JMSMESSAGE_CHANNELLINEUP;
								break;


							case DATATYPE_PO:
								tempmessagecode=JMSMESSAGE_PO;
								break;


							case DATATYPE_PORTAL:
								tempmessagecode=JMSMESSAGE_PORTAL;
								break;

							default:
								break;
							}

							tempchannel->m_nMessageType=tempmessagecode;

							tempchannel->m_channelID=k+1;
							tempport->m_channel[k]=tempchannel;
							tempport->m_ChannelNumber ++ ;
							k++;
							pSon = pSon->GetnextSibling();
						}
					}

					//get channel or IPPort 's sibling.
					pFather = pFather->GetnextSibling();

			}
			tempport->m_nID=m_pm->m_PortNumber+1;
			m_pm->m_PortVector.push_back(tempport);
			m_pm->m_PortNumber ++;	

			//get all Port 's sibling.

			pNode = pNode->GetnextSibling();

		}

	}
	catch(_com_error &e)
	{
		CString msg = CString((LPCWSTR)e.Description());
		msg = " "+msg;
	Clog( LOG_DEBUG,msg);
	}
//
//
//
////
//	m_XmlDOM.IntoElem();
//
//	if( !m_XmlDOM.FindElem( "ListOfNode" ) )
//	{
//		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(ListOfNode).") );
//		return 1;
//	}
//
//	m_XmlDOM.IntoElem();
//
//
//	if( !m_XmlDOM.FindElem( "DCAPortConfiguration" ) )
//	{
//		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DCAPortConfiguration).") );
//		return 1;
//	}
//
//	int portnumber=0;
//
//	//portnumber=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Count" ));
//
//	m_pm->m_Localqueuename=m_XmlDOM.GetAttrib( "QueueName" );
//	if (g_bNotInternalTest)
//	{
//		
//	}	
//	m_XmlDOM.IntoElem();
//
//	if( !m_XmlDOM.FindElem( "Port" ) )
//	{
//		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(Port).") );
//		return 1;
//	}
//
//	m_XmlDOM.IntoElem();
//
//	CString strPort="port";
//	int CircleNumber=1;
//	strPort.Format("port%d",CircleNumber);
//
//
//	while(1)
//	{	
//		strPort=m_XmlDOM.();
//		if (strPort.GetLength()<1)
//		{
//			break;
//		}
//		CDODPort *tempport=new CDODPort();
//
//
//		tempport->m_nPmtPID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "PMTPID" ));
//		tempport->m_nTotalRate=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "TotalRate" ));
//
//	}
//
//
//	for(int i=0;i<portnumber;i++)
//	{
//		if( !m_XmlDOM.FindElem( "DODPort" ) )
//		{
//			Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODPort).") );
//			return 1;
//		}
//
//		CDODPort *tempport=new CDODPort();
//
//
//		tempport->m_nPmtPID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "PMTPID" ));
//		tempport->m_nTotalRate=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "TotalRate" ));
//		m_XmlDOM.IntoElem();
//
//
//		if( !m_XmlDOM.FindElem( "IPPort" ) )
//		{
//			Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(IPPort).") );
//			return 1;
//		}
//		tempport->m_castcount=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPortCount" ));
//		m_XmlDOM.IntoElem();
//		for (int k=0;k<tempport->m_castcount;k++)
//		{
//			if( !m_XmlDOM.FindElem( "DODIPPort" ) )
//			{
//				Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODIPPort).") );
//				return 1;
//			}
//
//			sTmp = m_XmlDOM.GetAttrib("IPAddress");
//			_tcscpy( tempport->m_castPort[k].cDestIp, sTmp.GetBuffer( 0 ) );
//	
//			tempport->m_castPort[k].wDestPort=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));
//			tempport->m_castPort[k].wSendType=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "SendType" ));			
//		}
//
//		m_XmlDOM.OutOfElem();
//
//
//		if( !m_XmlDOM.FindElem( "channel" ) )
//		{
//			Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(channel).") );
//			return 1;
//		}
//		int nchannelnumber=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ChannelCount" ));
//		m_XmlDOM.IntoElem();
//	
//		for (k=0;k<nchannelnumber;k++)
//		{
//			if( !m_XmlDOM.FindElem( "DODChannel" ) )
//			{
//				Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODChannel).") );
//				return 1;
//			}
//
//			CDODChannel *tempchannel=new CDODChannel();
//
//			tempchannel->m_nType=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ChannelType" ));
//
//			if(tempchannel->m_nType !=1 && tempchannel->m_nType !=0)
//			{
//				Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODChannel).") );
//				delete tempport;
//				return 1;
//			}
//
//			sTmp = m_XmlDOM.GetAttrib("Tag");
//			_tcscpy( tempchannel->m_strTag, sTmp.GetBuffer( 0 ) );
//
//			int tempmessagecode=0;
////			
//			
//			tempchannel->m_nMessageType=tempmessagecode;
//
//			tempchannel->m_QueueName=m_XmlDOM.GetAttrib( "QueueName" );
//			if (g_bNotInternalTest)
//			{
//				m_pActiveJMS->AddOneSendQueue(tempchannel->m_QueueName.GetBuffer(0));
//			}
//			tempchannel->m_nRepeateTime=5;
//			tempchannel->m_nRate=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "channelRate" ));
////			tempchannel->m_nWorkDuration=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "LifeTime" ));
//
//
//			tempchannel->nStreamType=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "StreamType" ));
//			tempchannel->m_nStreamID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "StreamID" ));
//			tempchannel->m_bEncrypted=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "Encrypted" ));
//		//	tempchannel->m_bDetected=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "BeDetect" ));//BeDetectInterval
//		//	tempchannel->m_DetectInterVal=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "BeDetectInterval" ));
//
//			tempport->m_channel[k]=tempchannel;
//			tempport->m_ChannelNumber ++ ;
//		}
//		m_XmlDOM.OutOfElem();
//		m_XmlDOM.OutOfElem();
//
//		m_pm->m_pPort[i]=tempport;
//		m_pm->m_PortNumber ++;	
//	}
	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg parse end") );

	return 1;
}

int CJMSParser::ReceiverUpDataFolder(char *buf,int buffersize)
{
	CString str;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverUpDataFolder") );

	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEFLAG).") );
		return DODRECEIVERDATAFOLDERERROR;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port=0;
	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	if (messagecode !=FULLDATAWITHSHAREDFOLDERS)
		return DODRECEIVERDATAFOLDERERROR;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}
	int channelID=0; int channelIndex=0;
	channelID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE ));
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(channelID,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- MESSAGEDATATYPE or MESSAGEGROUPID error") );
		return 1;
	}

	int UpdateMode=0;
	m_XmlDOM.IntoElem();
	if( !m_XmlDOM.FindElem( CONTENTFILEOPERATION ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(CONTENTFILEOPERATION).") );
		return 1;
	}
	sTmp = m_XmlDOM.GetAttrib(CONTENTROOT);
	UpdateMode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( CONTENTUPDATEMODE));
	CString subPath="";
	if (UpdateMode==0)
	{
		//new full data location
		teport->m_channel[channelIndex]->MoveAllFile(sTmp,Port,sTmp);
	}

	else
	{
		m_XmlDOM.IntoElem();

		if( !m_XmlDOM.FindElem() )
		{
			Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(CONTENTFILE).") );
			return 1;
		}
		subPath = m_XmlDOM.GetAttrib(CONTENTPATH);
		switch(UpdateMode)
		{
		case 1:
			// the sub folder to be updated
			teport->m_channel[channelIndex]->UpdateSubFile(subPath,Port,sTmp);		
			break;
		case 2:
			//the sub folder to be deleted
			teport->m_channel[channelIndex]->DeleteSubFile(subPath,Port,sTmp);	
			break;
		case 3:
			// the file with full path to be deleted
			teport->m_channel[channelIndex]->DeleteFullPath(sTmp,Port,sTmp);	
			break;
		case 4:
			// is the file with full path to be Modofied
			teport->m_channel[channelIndex]->FullPathModofied(sTmp,Port,sTmp);	

			break;
		case 5:
			// the file with full path to be newed
			teport->m_channel[channelIndex]->FullPathToNew(sTmp,Port,sTmp);	
			break;

		default:
			break;
		}
	}

	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,teport->m_nID,teport->m_channel[channelIndex]->m_channelID);

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverUpDataFolder end") );

	return 0; 
}

int CJMSParser::ReceiverUpDataTCP(char *buf,int buffersize)
{
	CString str;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverUpDataTCP") );

	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEFLAG).") );
		return 1;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port= 0;

	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	if (messagecode !=FULLDATAWITHTCP)
		return 1;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}

	int channelID=0; int channelIndex=0;
	channelID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE ));
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(channelID,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- MESSAGEDATATYPE or MESSAGEGROUPID error") );
		return 1;
	}

	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGECONTENT ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DataList).") );
		return 1;
	}
	CString filename;
	filename=m_XmlDOM.GetAttrib( "Destination");
	int leafTime=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ExpiredTime"));
	int nMessageID=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEMESSAGEID));
	int OperationCode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "OperationCode"));
	int datatype=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE));
	
	sTmp=m_XmlDOM.GetSubDoc();
	teport->m_channel[channelIndex]->CreateMsgFile(sTmp,filename,leafTime,nMessageID,OperationCode,datatype);

	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,teport->m_nID,teport->m_channel[channelIndex]->m_channelID);

	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverUpDataTCP end") );

	return 0;
}