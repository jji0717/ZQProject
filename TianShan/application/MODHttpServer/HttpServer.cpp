// HttpServer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <HttpEngine.h>
#include <FileLog.h>
#include <sstream>
#include <fstream>
#include <list>
#include "TimeUtil.h"
#include "XMLPreferenceEx.h"
#include "ZQ_common_conf.h"
#include "strHelper.h"

using namespace ZQ::common;
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
const std::string XML_HENAN_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n";
static std::string int2str(int i)
{
	char buf[12] = {0};
	return itoa(i, buf, 10);
}
class IAuthorizeRequestHandler : public ZQHttp::IRequestHandler 
{
private:
	std::string _cachedData;
	int         _dataLen;
	std::string _methodType;
	std::string _strFullURL;

	ZQHttp::IResponse* _response;
protected:
	bool parserAuthor(std::string& strResponse, std::string & transactionID,
		std::string& sendId, std::string& receiverId,
		std::string& sessionId, std::string& userId, 
		std::string& entitlementCode);
	bool parserNotice(std::string& strResponse, std::string & transactionID,
		std::string& sendId, std::string& receiverId,
		std::string& sessionId, std::string& userId);
public:
	IAuthorizeRequestHandler(ZQ::common::Log& log):_log(log)
	{
		_dataLen = 0;
		_cachedData = "";
	}
	~IAuthorizeRequestHandler(){}

	virtual bool onPostData(const ZQHttp::PostDataFrag& frag)
	{
		_cachedData += frag.data;
		return true;
	}
	virtual bool onPostDataEnd()
	{
		return true;
	}
	virtual void onRequestEnd()
	{
		char temp[513] = "";
		int length =_cachedData.size(); 
		ZQ::common::XMLPreferenceDocumentEx xmlDoc;
		try
		{
			if (!xmlDoc.read((void*)_cachedData.c_str(), length, 1))//successful
			{
				return;
			}
		}
		catch (...)
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "onRequestEnd() : invailed xml format[%s]"), _cachedData.c_str());
		}

		ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
		if(NULL == pXMLRoot)
		{
			xmlDoc.clear();
			return;
		}

		bool bret = pXMLRoot->getPreferenceName(temp, 513);
		if(!bret)
			return;
		if(stricmp(temp, "RequestPlaylist") == 0)
		{
           return heNanSetup();
		}
		else if(stricmp(temp, "SessionStart") == 0 || stricmp(temp, "SessionStop") == 0 ||stricmp(temp, "SessionFail") == 0)
		{
			_response->setHeader(NULL, NULL);
			_response->setHeader("Content-length", 0);
			_response->setStatus(200, "OK");
			_response->headerPrepared();
			_response->complete();
            return;
		}
		
		ZQ::common::XMLPreferenceEx* headpre = pXMLRoot->findSubPreference("Header");

		if(NULL == headpre)
		{
			pXMLRoot->free();
			xmlDoc.clear();
			return;
		}

		ZQ::common::XMLPreferenceEx* opCodepre = headpre->findChild("OpCode");
		if(NULL == opCodepre)
		{
			pXMLRoot->free();
			xmlDoc.clear();
			return;
		}

		memset(temp, 0, 513);
		opCodepre->getPreferenceText(temp, 512);
		_methodType = temp;

		pXMLRoot->free();
		xmlDoc.clear();

		if ("SM_AAA_S2_AUTHORIZE_REQ" == _methodType)
		{
			return sessionSetup();
		}
		else if ("SM_AAA_S2_STATUSNOTICE_REQ" == _methodType)
		{
			return sessionStatusNotice();
		}
		else
		{
			_response->setHeader(NULL, NULL);
			_response->setHeader("Content-length", 0);
			_response->setStatus(200, "OK");
			_response->headerPrepared();
			_response->complete();
			_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "onRequestEnd() : ingore this request uri=%s"), _strFullURL.c_str());
		}
	}
	// break the current request processing
	virtual void onBreak()
	{
	}

	/// on*() return true for continue, false for break.
	virtual bool onRequest(const ZQHttp::IRequest& request, ZQHttp::IResponse& response)
	{
		_response = &response;
		_strFullURL = request.uri();
//		_methodType = _strFullURL.substr(_strFullURL.find_last_of("/"));
		_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "onRequest() : receive request uri=%s"), _strFullURL.c_str());
		return true;
	}
	void sessionSetup()
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "sessionSetup() : %s"), _cachedData.c_str());
		std::vector<std::string> playlists;
        FILE* fp = fopen("playlists.txt", "r");
		if(fp == NULL)
		{
             playlists.push_back("<PlaylistItem>ZQ_C20037_SeaChangeChina vod 40000 1000-6000 FRP</PlaylistItem>\n");
			 playlists.push_back("<PlaylistItem>Mtest_005 ad 40000 500-600 FRP</PlaylistItem>\n");
		}
		else
		{
            while(!feof(fp))
			{
                char buf[1024]="";
				char* pread = fgets(buf, 1024, fp);
				if(pread  != NULL)
					playlists.push_back(buf);
			}
			fclose(fp);
		}
		try
		{
			std::string utctime;
			time_t currentTime;
			time(&currentTime);
			char buftime[65]="";
			TimeUtil::Time2Iso(currentTime, buftime, 64);
			utctime = buftime;

			std::string transactionID;
			std::string sendId,  receiverId, sessionId, userId ,entitlementCode;
			parserAuthor(_cachedData, transactionID, sendId, receiverId,  sessionId, userId, entitlementCode);

			std::string strResponse;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<Message>\n";
			buf << "  <Header>\n" ;
			buf << "    <TransactionID>" << transactionID << "</TransactionID>\n" ;
			buf << " 	<Time> "<< utctime <<"</Time>\n" ;
			buf << " 	<OpCode> SM_AAA_ S2_AUTHORIZE_REQ</OpCode>\n" ;
			buf << " 	<MsgType>RESP</MsgType>\n" ;
			buf << " 	<ReturnCode>0</ReturnCode>\n" ;
			buf << " 	<ErrorMessage></ErrorMessage>\n" ;
			buf << "  </Header>\n" ;
			buf << "  <Body>\n" ;
			buf << "    <AuthorizedResponseInfo  \n";
			buf << " 	    sessionID=\""<< sessionId << "\"\n";
			buf << " 	    userID =\""<< userId << "\"\n";
			buf << " 	    entitlementCode =\""<< entitlementCode << "\">\n";
			buf << " 	<MaxBitrate>40000</MaxBitrate>\n";
			buf << " 	<Duration>1600.00</Duration>\n";
			buf << " 	<PlayType>1</PlayType>\n";
			buf << " 	<Playlist>\n";
			std::vector<std::string>::iterator itorpl = playlists.begin();
			while(itorpl != playlists.end())
			{
				buf << " 	   " << *itorpl;
				itorpl++;
			}
			buf << " 	</Playlist>\n";
			buf << " 	</AuthorizedResponseInfo> \n";
			buf << "  </Body>\n" ;
			buf << "</Message>\n" ;

			strResponse = buf.str();

			_response->setHeader(NULL, NULL);

			char length[20];
			snprintf(length, sizeof(length), "%ld", strResponse.size());
			_response->setHeader("Content-type", "text/xml");
			_response->setHeader("Content-length", length);
			_response->setStatus(200, "OK");
			_response->headerPrepared();
			_response->addContent(strResponse.c_str(), strResponse.size());
			_response->complete();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "sessionSetup() : response [%s]"), strResponse.c_str());

		}
		catch (...)
		{
			
		}
	}
	void sessionStatusNotice()
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "sessionStatusNotice() : %s"), _cachedData.c_str());

		try
		{
			std::string utctime;
			time_t currentTime;
			time(&currentTime);
			char buftime[65]="";
			TimeUtil::Time2Iso(currentTime, buftime, 64);
			utctime = buftime;

			std::string transactionID;
			std::string sendId,  receiverId, sessionId, userId ;

			parserNotice(_cachedData, transactionID, sendId, receiverId,  sessionId, userId);

			std::string strResponse;
			std::ostringstream buf;
			buf << XML_HEADER ;
			buf << "<Message>\n";
			buf << "  <Header>\n" ;
			buf << "    <TransactionID>" << transactionID<< "</TransactionID>\n" ;
			buf << " 	<Time> "<< utctime << "</Time>\n" ;
			buf << " 	<OpCode> SM_AAA_ S2_STATUSNOTICE_REQ</OpCode>\n" ;
			buf << " 	<MsgType>RESP</MsgType>\n" ;
			buf << " 	<ReturnCode>0</ReturnCode>\n" ;
			buf << " 	<ErrorMessage></ErrorMessage>\n" ;
			buf << "  </Header>\n" ;
			buf << "  <Body>\n" ;
			buf << "  </Body>\n" ;
			buf << "</Message>\n" ;

			strResponse = buf.str();

			_response->setHeader(NULL, NULL);

			char length[20];
			snprintf(length, sizeof(length), "%ld", strResponse.size());
			_response->setHeader("Content-type", "text/xml");
			_response->setHeader("Content-length", length);	
			_response->setStatus(200, "OK");
			_response->headerPrepared();
			_response->addContent(strResponse.c_str(), strResponse.size());
			_response->complete();
			_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "sessionStatusNotice() : response [%s]"), strResponse.c_str());

		}
		catch (...)
		{
			
		}
	}
	void heNanSetup()
	{
		try
		{

			/*
			<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 
			<PhysicalPlaylist  playlistID=¡± [32-hex ID]¡± bitRate=¡±3750000¡±  startIndex =¡±1¡± startNPT =¡±0.0¡±>
			<ContentRef  providerID=¡±hncatv¡±  
			assetID =¡± hnca1234567890123456¡±
			startNPT=¡±0.0¡±   endNpt=¡±¡± />
			<ContentRef  providerID=¡± hncatv¡± 
			assetID =¡± hnca1234567890123457¡±  
			startNPT=¡±0.0¡±   endNpt=¡±¡± />
			</PhysicalPlaylist>
			*/
			std::string strResponse;
			std::ostringstream buf;
			buf << XML_HENAN_HEADER ;
			buf << "<PhysicalPlaylist  playlistID=\"12345\" bitRate=\"3750000\"  startIndex =\"1\" startNPT =\"0.0\">\n";
			buf << "  <ContentRef  providerID=\"hncatv\"  assetID=\"hnca1234567890123456\" startNPT=\"0.0\" endNpt=\"\" />\n" ;
			buf << "  <ContentRef  providerID=\"hncatv\"  assetID=\"hnca1234567890123457\" startNPT=\"0.0\" endNpt=\"\" />\n" ;
			buf << "</PhysicalPlaylist>\n" ;
			strResponse = buf.str();

			_response->setHeader(NULL, NULL);

			char length[20];
			snprintf(length, sizeof(length), "%ld", strResponse.size());
			_response->setHeader("Content-type", "text/xml");
			_response->setHeader("Content-length", length);
			_response->setStatus(200, "OK");
			_response->headerPrepared();
			_response->addContent(strResponse.c_str(), strResponse.size());
			_response->complete();
			_log(ZQ::common::Log::L_INFO, CLOGFMT(IAuthorizeRequestHandler, "sessionSetup() : response [%s]"), strResponse.c_str());

		}
		catch (...)
		{

		}
	}
protected:
	ZQ::common::Log& _log;
};
bool IAuthorizeRequestHandler::parserAuthor(std::string& strResponse, std::string & transactionID,
											std::string& sendId, std::string& receiverId,
											std::string& sessionId, std::string& userId, 
											std::string& entitlementCode)
{
	char temp[513] = "";
	if (strResponse.size() < 1)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(IAuthorizeRequestHandler, "parserAuthor() Response message is NULL")); 
		return false;
	}

	int length =strResponse.size(); 
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)strResponse.c_str(), length, 1))//successful
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor() failed to init XMLdocument")); 
		return false;
	}
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get root Preference")); 

		xmlDoc.clear();
		return false;
	}
	ZQ::common::XMLPreferenceEx* headpre = pXMLRoot->findSubPreference("Header");

	if(NULL == headpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get header Preference")); 

		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}

	ZQ::common::XMLPreferenceEx* transactionIdpre = headpre->findChild("TransactionID");
	if(NULL == transactionIdpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get header/TransactionID child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	memset(temp, 0, 513);
	transactionIdpre->getPreferenceText(temp, 512);
	transactionID = temp;

	ZQ::common::XMLPreferenceEx* sendIdpre = headpre->findChild("SenderID");
	if(NULL == sendIdpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get header/SenderID child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	memset(temp, 0, 513);
	sendIdpre->getPreferenceText(temp, 512);
	sendId = temp;

	ZQ::common::XMLPreferenceEx* receiverIdpre = headpre->findChild("ReceiverID");
	if(NULL == receiverIdpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get header/ReceiverID child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	memset(temp, 0, 513);
	receiverIdpre->getPreferenceText(temp, 512);
	receiverId = temp;

	ZQ::common::XMLPreferenceEx* bodypre = pXMLRoot->findSubPreference("Body");
	if(NULL == bodypre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get body Preference")); 

		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}

	ZQ::common::XMLPreferenceEx* authorpre = bodypre->findChild("AuthorizedRequestInfo");
	if(NULL == authorpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get body/AuthorizedRequestInfo child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("sessionID", temp))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get body/AuthorizedRequestInfo/sessionId attribute")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	};
	sessionId = temp;

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("userID", temp))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get body/AuthorizedRequestInfo/userID attribute")); 
		pXMLRoot->free();
		xmlDoc.clear();

		return false;
	};
	userId = temp;

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("entitlementCode", temp))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserAuthor()failed to get body/AuthorizedRequestInfo/entitlementCode attribute")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	};
	entitlementCode = temp;

	pXMLRoot->free();
	xmlDoc.clear();
	return true;
}
bool IAuthorizeRequestHandler::parserNotice(std::string& strResponse, std::string& transactionID,
											std::string& sendId, std::string& receiverId,
											std::string& sessionId, std::string& userId)
{
	char temp[513] = "";
	if (strResponse.size() < 1)
	{
		return false;
	}

	int length =strResponse.size(); 
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)strResponse.c_str(), length, 1))//successful
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice() failed to init XMLdocument")); 
		return false;
	}
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get root Preference")); 

		xmlDoc.clear();
		return false;
	}
	ZQ::common::XMLPreferenceEx* headpre = pXMLRoot->findSubPreference("Header");

	if(NULL == headpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get header Preference")); 

		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}

	ZQ::common::XMLPreferenceEx* transactionIdpre = headpre->findChild("TransactionID");
	if(NULL == transactionIdpre)
	{	
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get header/TransactionID child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	memset(temp, 0, 513);
	transactionIdpre->getPreferenceText(temp, 512);
	transactionID = temp;

	ZQ::common::XMLPreferenceEx* sendIdpre = headpre->findChild("SenderID");
	if(NULL == sendIdpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get header/SenderID child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	memset(temp, 0, 513);
	sendIdpre->getPreferenceText(temp, 512);
	sendId = temp;

	ZQ::common::XMLPreferenceEx* receiverIdpre = headpre->findChild("ReceiverID");
	if(NULL == receiverIdpre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get header/ReceiverID child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	memset(temp, 0, 513);
	receiverIdpre->getPreferenceText(temp, 512);
	receiverId = temp;

	ZQ::common::XMLPreferenceEx* bodypre = pXMLRoot->findSubPreference("Body");
	if(NULL == bodypre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get body Preference")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}

	ZQ::common::XMLPreferenceEx* noticepre = bodypre->findChild("StatusNoticeInfo");
	if(NULL == noticepre)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get body/StatusNoticeInfo child")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}

	memset(temp, 0, 513);
	if(!noticepre->getAttributeValue("sessionID", temp))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get body/StatusNoticeInfo/sessionID attribute")); 
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	};
	sessionId = temp;

	memset(temp, 0, 513);
	if(!noticepre->getAttributeValue("userID", temp))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(IAuthorizeRequestHandler, "parserNotice()failed to get body/StatusNoticeInfo/userID attribute")); 
		pXMLRoot->free();
		xmlDoc.clear();

		return false;
	};
	userId = temp;

	pXMLRoot->free();
	xmlDoc.clear();
	return true;
}
class AuthorizeRequestFactory: public ZQHttp::IRequestHandlerFactory
{
public:
	AuthorizeRequestFactory(ZQ::common::Log& log):_log(log){};
	 ~AuthorizeRequestFactory(){}
	 ZQHttp::IRequestHandler* create(const char* uri)
	{
		return new IAuthorizeRequestHandler(_log);
	}
	 void destroy(ZQHttp::IRequestHandler* h)
	 {
		 if(h)
			 delete h;
	 }
protected:
	ZQ::common::Log& _log;

};
ZQ::common::Log g_log(ZQ::common::Log::L_DEBUG);

bool _bQuit = false;

bool WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:	
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		printf("Exit program\n");
		_bQuit = true;
		break;
	default:
		break;
	}
	return true;
}

static void showUsage()
{
	printf("HttpServer <logpath> <host> <port(option 10180)> \n");
}
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		showUsage();
		return 1;
	}
	const char* logpath = argv[1];

	ZQ::common::FileLog modHttpSLog(logpath, ZQ::common::Log::L_DEBUG);

	ZQHttp::Engine e(modHttpSLog);

	AuthorizeRequestFactory authorizeFac(modHttpSLog);
	e.registerHandler("/AUTHORIZE", &authorizeFac);
	e.registerHandler("/STATUSNOTICE", &authorizeFac);
	e.registerHandler("", &authorizeFac);
	e.registerHandler("/*", &authorizeFac);

	std::string  port = "10180";
	std::string  host = "0.0.0.0";
	if(argc > 2)
	{
		host = argv[2];
	}
	if(argc > 3)
	{
		port = argv[3];
	}

	e.setEndpoint(host, port);
	e.start();

	if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,true) == false)  
		printf("unable to register console handler\n");

	while(!_bQuit)
	{
		Sleep(500);
	}
	e.stop();
	return 0;
}
