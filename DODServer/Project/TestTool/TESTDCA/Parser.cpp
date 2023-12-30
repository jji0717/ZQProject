// Parser.cpp: implementation of the CJMSParser class.
//
//2006-3-14 Review Code for bug: Get Info from MMA-5310 XML while VIX file notfication in ParseStatus
//          Modify Clog Log Level 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Parser.h"
#include "clog.h"
#include "Markup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#import "msxml3.dll"
//#import "msxml4.dll"
using namespace MSXML2;

extern CString g_strEXEPath;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJMSParser::CJMSParser(CJMS *inJms,CBaseSource *client)
{
	m_jms=inJms;
//	m_pBaseSource=client;
//	m_bStartReceive=FALSE;
//	m_bIsLocalConfig=FALSE;
	return; 
}
CJMSParser::~CJMSParser(void)
{

}
void CJMSParser::onMessage(Message *ms)
{
	if (ms==NULL)
	{
		Clog( LOG_ERROR,_T(" CJMSParser::onMessage "));
		return ;
	}
	char name[MAX_PATH];
	memset(name,0,MAX_PATH);
	CJMSTxMessage *tm = (CJMSTxMessage *)ms;

	if (tm->_message==NULL)
	{
		Clog( LOG_ERROR,_T("CJMSParser::onMessage(Message *ms)tm->_message==NULL  error"));
		return ;
	}
	//int stringSize=0;
	tm->getStringProperty("MESSAGECLASS",name,MAX_PATH);

	if(strcmpi(name,"COMMAND")==0)
	{
		//now ,this founction is not performance.
		ParseCommand(tm);  
	}
	else
		if(strcmpi(name,"STATUS")==0)
		{
			ParseStatus(tm);  
		}
		else
			if(strcmpi(name,"NOTIFICATION")==0)
			{
				parseNotification(tm);
			}
			else
			{
				Clog( LOG_ERROR,_T(" tm->getStringProperty MESSAGECLASS,MESSAGECLASS is other value,error"));
				return ;
			}
}

CJMSTxMessage  CJMSParser::ParseCommand(CJMSTxMessage* pMessage)
{

	if (pMessage==NULL)
	{
		Clog( LOG_ERROR,_T(" CJMSParser::onMessage "));
		return *pMessage;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;

	if (tm->_message==NULL)
	{
		Clog( LOG_ERROR,_T(" CJMSTxMessage  CJMSParser::ParseCommand( handle error"));
		return *pMessage;
	}
	int datasize=tm->GetDataSize();

	char *name = new char[datasize];

	tm->getText(name,datasize);

	CString data;
	data=name;

	delete name;

	Clog( LOG_DEBUG,_T("CJMSParser:: received a msg ,type=Command !") );
	Clog( LOG_DEBUG,data);
// these codes will used to reply JMSCommand .but now the DCA project was not used it;



	TextMessage TmpMsg;

	//TmpMsg.setText("temp message"); 


	CJMSTxMessage destmessage;
	//CString replystring="reply ";
	////replystring+=data;

	//destmessage.setText(replystring.GetBuffer(0));

 //   destmessage.setStringProperty("MESSAGECLASS","NOTIFICATION");
 //
	CMarkup m_XmlDOM;

	CString strFilename;
	strFilename = "Localconfig.xml";

	m_XmlDOM.Load(strFilename);

	CString sContent = m_XmlDOM.GetDoc();

	Destination tempDest;
	Producer	producer;
	TextMessage textmessage;

	tm->getReplyTo( tempDest );
	if( !tempDest._destination )
	{
		Clog( LOG_DEBUG, _T("CJMSListener::onMessage() - NULL == tempDest._destination.") );
		return destmessage;
	}

	Session writerSession;  
	if(m_jms->m_Connection.createSession(writerSession) == FALSE)
	{
		Clog( LOG_DEBUG,_T("Processing command: m_Connection.createSession error "));
		return destmessage;
	}

	writerSession.createProducer(&tempDest,producer);
	writerSession.textMessageCreate( sContent.GetBuffer(0),textmessage);
	textmessage.setStringProperty( "MESSAGECLASS", "NOTIFICATION" ); // STATUS

	if(producer.send(&textmessage) == FALSE)
	{
		Clog( LOG_DEBUG,_T("Processing command: producer.send error"));
		return destmessage;
	}

	return destmessage;

}
int CJMSParser::parseNotification(CJMSTxMessage* pMessage)
{
	if (pMessage==NULL)
	{
		printf("\n CJMSParser::onMessage pMessage is null\n");
		return 0;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;

	if (tm->_message==NULL)
	{
		printf("\n CJMSParser::parseNotification  get message handle error\n");
		return 0;
	}

	int datasize=tm->GetDataSize();

	char *name = new char[datasize];

	tm->getText(name,datasize);

	CString data;
	data=name;
	delete name;
	Clog( LOG_DEBUG,_T("CJMSMessage parseNotification") );
	Clog( LOG_DEBUG,data);

	//CMarkup m_XmlDOM;

	//m_XmlDOM.SetDoc(data);

//	int OperateCode=-1;

//	ASSETINFO *info=new ASSETINFO;

	return 0;
}

//Following Message is from MMA5310 About File Notification
//2006-3-14 Review code for bug
/*	<Message Time="2004070101:00:00" Flag="1" No="100001" Source="8" MessageCode="103011" EquipmentID="am">
		<File FileName="aaaaa.mpg" Operate="5" FileType="3"/>
		<Mpd>
			<AssetInfo
				IsVixExist="true"
				VIXFileName="aaaaa.vix"
			/>
		</Mpd>
	</Message>
*/
int CJMSParser::ParseStatus(CJMSTxMessage* pMessage)
{
	if (pMessage==NULL) 
	{
		//Modify
		Clog( LOG_ERROR,_T("ERROR::[CJMSParser:ParseStatus]pMessage is NULL"));
		return 1;
	}
	
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;

	if (tm->_message==NULL)
	{
		//Modify
		Clog( LOG_ERROR,_T("ERROR::[CJMSParser:ParseStatus]get message handle NULL"));
		return 1;
	}

	int datasize=tm->GetDataSize();
	char *name = new char[datasize+1]; //Add 1 for boundry

	tm->getText(name,datasize);

	CString data;
	data=name;
	delete name;
	
	Clog( LOG_DEBUG,_T("DEBUG::[CJMSParser:ParseStatus]received a msg ,type=Status ,Message content is below :") );
	Clog( LOG_DEBUG,data);
	CMarkup m_XmlDOM;
	
	m_XmlDOM.SetDoc(data);
	
	
	
	//CString strFilename=m_XmlDOM.GetAttrib( "FileName" );
	//CString strOldFileName=m_XmlDOM.GetAttrib( "OldFileName" );
	//
	//strcpy((char *)(info->strFilename),strFilename.GetBuffer(0));
	//
	//OperateCode=Safeatoi((LPCTSTR)m_XmlDOM.GetAttrib( "Operate" ));
	//int FileType=Safeatoi((LPCTSTR)m_XmlDOM.GetAttrib( "FileType" ));
	
	//New and Modify Event	
	return 0;
}

int CJMSParser::Safeatoi(CString str)
{
	int len=str.GetLength();
	if (len <1)
	{
	//	if the element 's attributer is null ,dafault value is zero;
		return 0;
	}

	char a;
	for(int i=0;i<len;i++)
	{
		a=str.GetAt(i);
		if (a=='-')
			continue;
		if (a=='.')
			continue;
		if(isdigit(a)==false)
		{
		//	Clog( LOG_DEBUG,_T("ReceiverDataTCP CheckDigiteORIsalpha- isdigit(a)==false. %s error "),str);
			return 0;
		}	
	}
	len=_ttoi((LPCTSTR)str);
	return len;
}