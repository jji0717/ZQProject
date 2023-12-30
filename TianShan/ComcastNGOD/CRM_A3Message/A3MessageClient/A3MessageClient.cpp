// A3MessageClient.cpp : Defines the entry point for the console application.
//
/*
<< "          key=sourceIP         (Ftp and file system not use,  PGM or IP Multicast use(Optional))\n"
<< "          key=sourceIP1        (Ftp and file system not use,  PGM or IP Multicast use(Optional))\n"
<< "          key=userName         (Ftp and file system use,  PGM or IP Multicast not use)\n"
<< "          key=password         (Ftp and file system use,  PGM or IP Multicast not use)\n"
*/
#include "stdafx.h"
#include "A3MessageReq.h"
#include <stdio.h>
#include "strHelper.h"
#include "TimeUtil.h"
using namespace CRM::A3MessageClient;
ContentInfo _content;
bool _bQuit = false;
void showHelp()
{
	std::cout << "Usage: A3MessageClient <http://ip:port(10080)> [bindip] [port]\n"
		<< "Options:\n"
		<< "set <user.>key=value\n"       
		<< "        key=volumeName       (TransferConent required)\n"
		<< "        key=pid              (TransferConent required)\n"
		<< "        key=paid             (TransferConent required)\n"
		<< "        key=transferBitRate  (TransferConent required)\n"
		<< "        key=sourceURL        (TransferConent required)\n"
		<< "        key=responseURL      (TransferConent required)\n"
		<< "        key=sourceURL1       (TransferConent Optional)\n"
		<< "        key=captureStart     (required, PGM or IP Multicast)\n"
		<< "        key=captureEnd       (required, PGM or IP Multicast)\n"
		<< "        key=sourceIP         (Optional, PGM or IP Multicast use)\n"
		<< "        key=sourceIP1        (Optional, PGM or IP Multicast use)\n"
		<< "        key=userName         (required, Ftp and file system use)\n"
		<< "        key=password         (required, Ftp and file system use)\n"
		<< "        key=protocol         (ExposeContent required)\n"
		<< "        key=user.key or sys. (TransferConent ContentAsset metedata)\n"
		<< "get     return all key value\n"
		<< "exec1   send GetVolumeInfo(volumeName)\n"
		<< "exec2   send GetContentInfo(volumeName,pid,paid)\n"
		<< "exec3   send TransferContent(ContentInfo)\n"
		<< "exec4   send CancelTransfer(volumeName,pid,paid,reasonCode)\n"
		<< "exec5   send DeleteContent(volumeName,pid,paid,reasonCode)\n"
		<< "exec6   send GetContentChecksum(volumeName,pid,paid,responseURL)\n"
		<< "exec7   send ExposeContent(volumeName,pid,paid,protocol,transferBitRate)\n"
		<< "exec8   send GetTransferStatus(volumeName,pid,paid)\n"
		<< "reset   clear all key value\n"
		<< "quit|exit   quit program\n"
		<< "help|-h     show help\n"
		<< std::endl;
}
void set(std::vector<std::string>&strParamter)
{
	std::string strbuf = strParamter[1];
	int npos = strbuf.find('=');
	if(npos < 0)
	{
		printf("A3MessageClient>>invaild  set parameter command format: set <user.>key=value");
		return;
	}
	std::string strKey = strbuf.substr(0, npos);
	std::string strValue = strbuf.substr(npos +1);

	if(strKey.size() > 5 && (strKey.substr(0,5) == "user." ||strKey.substr(0,4) == "sys."))
	{
		std::string propkey;
		if(strKey.substr(0,4) == "sys.")
			propkey = strKey.substr(4);
		else 
			propkey = strKey.substr(5);

		if(_content.props.find(propkey) != _content.props.end())
			_content.props[propkey] = strValue;
		else	
		    _content.props.insert(std::map<std::string, std::string>::value_type(propkey, strValue));		
	}
	else if(!stricmp(strKey.c_str() ,"volumeName"))
		_content.volumeName = strValue;
	else if(!stricmp(strKey.c_str() ,"pid"))
		_content.pid = strValue;
	else if(!stricmp(strKey.c_str() ,"paid"))
		_content.paid = strValue;
	else if(!stricmp(strKey.c_str() ,"captureStart"))
		_content.captureStart = strValue;
	else if(!stricmp(strKey.c_str() ,"captureEnd"))
		_content.captureEnd = strValue;
	else if(!stricmp(strKey.c_str() ,"transferBitRate"))
		_content.transferBitRate = atoi(strValue.c_str());
	else if(!stricmp(strKey.c_str() ,"sourceURL"))
		_content.sourceURL = strValue;
	else if(!stricmp(strKey.c_str() ,"sourceIP"))
		_content.sourceIP = strValue;
	else if(!stricmp(strKey.c_str() ,"sourceURL1"))
		_content.sourceURL1 = strValue;
	else if(!stricmp(strKey.c_str() ,"sourceIP1"))
		_content.sourceIP1 = strValue;
	else if(!stricmp(strKey.c_str() ,"userName"))
		_content.userName = strValue;
	else if(!stricmp(strKey.c_str() ,"password"))
		_content.password = strValue;
	else if(!stricmp(strKey .c_str(),"responseURL"))
		_content.responseURL = strValue;
	else if(!stricmp(strKey.c_str() ,"reasonCode"))
		_content.reasonCode = atoi(strValue.c_str());
	else if(!stricmp(strKey.c_str() ,"protocol"))
		_content.protocol = strValue;

}
void init()
{
	_content.volumeName = "/70001";
	_content.pid = "XOR";
	_content.paid = "cdntest1234567890010006";

	int64 c = ZQ::common::now();

	char buf[512];
	memset(buf, 0, sizeof(buf));
	std::string startTimeUTC = ZQ::common::TimeUtil::TimeToUTC(c, buf, sizeof(buf) -2);
	//				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

	memset(buf, 0, sizeof(buf));
	std::string endTimeUTC = ZQ::common::TimeUtil::TimeToUTC(c + 3600 * 24 * 1000 * 7, buf, sizeof(buf) -2);

	_content.captureStart = startTimeUTC;
	_content.captureEnd = endTimeUTC;

	_content.transferBitRate = 3750000;
	_content.sourceURL = "ftp://192.168.81.52/4K-8M-H265";
//	_content.sourceURL = "ftp://192.168.81.52/ZIBO_H264_SD.ts";
//	_content.sourceURL = "ftp://192.168.81.52/M800_MPEG2.mpg";
//	_content.sourceURL = "raw://233.19.204.171:1234";
	_content.sourceIP = "192.168.81.52";
	_content.sourceURL1 = "";
	_content.sourceIP1 = "";
	_content.userName = "hl";
	_content.password = "hl";
	_content.responseURL = "http://192.168.81.101:4144";
	_content.props.clear();
	_content.reasonCode = -9999;
	_content.protocol = "";

	_content.props["cscontenttype"] = "CSI";
	_content.props["acscontenttype"] = "nCSaaI";
}

void reset()
{
	_content.volumeName = "";
	_content.pid = "";
	_content.paid = "";
	_content.captureStart = "";
	_content.captureEnd = "";
	_content.transferBitRate = -1;
	_content.sourceURL = "";
	_content.sourceIP = "";
	_content.sourceURL1 = "";
	_content.sourceIP1 = "";
	_content.userName = "";
	_content.password = "";
	_content.responseURL = "";
	_content.props.clear();
	_content.reasonCode = -9999;
	_content.protocol = "";
}
void resetTime()
{
	int64 c = ZQ::common::now();
	char buf[512];
	memset(buf, 0, sizeof(buf));
	std::string startTimeUTC = ZQ::common::TimeUtil::TimeToUTC(c, buf, sizeof(buf) -2);
	//				::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

	memset(buf, 0, sizeof(buf));
	std::string endTimeUTC = ZQ::common::TimeUtil::TimeToUTC(c + 3*60*1000, buf, sizeof(buf) -2);

	_content.captureStart = startTimeUTC;
	_content.captureEnd = endTimeUTC;
}
void getContentInfo()
{
	if(_content.volumeName != "")
		std::cout<< "	volumeName="<< _content.volumeName<<"\n";
	if(_content.pid != "")
		std::cout<< "	pid=" << _content.pid<<"\n";
	if(_content.paid != "")
		std::cout<< "	paid=" <<_content.paid<<"\n";
	if(_content.captureStart != "")
		std::cout<< "	captureStart="<<_content.captureStart<<"\n";
	if(_content.captureEnd != "")
		std::cout<< "	captureEnd="<<_content.captureEnd<<"\n";
	if(_content.transferBitRate >= 0)
		std::cout<< "	transferBitRate="<<_content.transferBitRate<<"\n";
	if(_content.sourceURL != "")
		std::cout<< "	sourceURL="<<_content.sourceURL<<"\n";
	if(_content.sourceIP != "")
		std::cout<< "	sourceIP="<<_content.sourceIP<<"\n";
	if(_content.sourceURL1 != "")
		std::cout<< "	sourceURL1="<<_content.sourceURL1<<"\n";
	if(_content.sourceIP1 != "")
		std::cout<< "	sourceIP1="<<_content.sourceIP1<<"\n";
	if(_content.userName != "")
		std::cout<< "	userName="<<_content.userName<<"\n";
	if(_content.password != "")
		std::cout<< "	password="<<_content.password<<"\n";
	if(_content.responseURL != "")
		std::cout<< "	responseURL="<<_content.responseURL<<"\n";
	if(_content.reasonCode >= 0)
		std::cout<< "	reasonCode="<<_content.reasonCode<<"\n";
	if(_content.protocol != "")
		std::cout<< "	protocol="<<_content.protocol<<"\n";

	std::map<std::string, std::string>::iterator itorprops;
	for(itorprops = _content.props.begin(); itorprops != _content.props.end(); itorprops++)
	{
		std::cout <<"	user." << itorprops->first << "=" << itorprops->second << "\n";
	}
}

int main(int argc, char* argv[])
{
	bool bfile = false;
	if(argc < 2)
	{
		showHelp();
		return 0;
	}

	std::string filepath;
	reset();	
	if(argc >= 5)
	{
		bfile = true;
		filepath = argv[4];
	}
	std::string reqURL = argv[1];
	std::string strbindip="";
	int nport = 0;
	if(argc >= 4)
		nport = atoi(argv[3]);
	if(argc >= 3)
		strbindip = argv[2];


	ZQ::common::FileLog a3MsgLog("a3MessageClient.log", ZQ::common::Log::L_DEBUG);
	A3MessageReq _a3MessageReq(a3MsgLog, reqURL);
	if(!_a3MessageReq.init(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, strbindip, nport))
		return 0;

	printf("A3MessageClient>>successful to init http client with <%s>\n", reqURL.c_str());

	init();
	char strbuf[512]="";

	while(!_bQuit)
	{	
		if(bfile)
		{

			FILE* fin = fopen(argv[4],"r");
			if(NULL == fin)
			{
				printf("open %s error\n",argv[4]);
				break;
			}

			char readStr[512];

			while(NULL != fgets(readStr,512,fin))
			{
				//read from file
				std::vector<std::string>strParamter;
				strParamter = ZQ::common::stringHelper::split(readStr, ' ');

				//the line is not configuration info
				if(5 == strParamter.size())
					continue;
				if(0 == strcmp(strParamter[0].c_str(),"\n"))
					continue;

				std::string key;
				if(1 == strParamter.size())
				{
					int pos = strParamter[0].find('\n');
					key = strParamter[0].substr(0,pos);
				}
				if(strParamter.size() > 0)
				{

					if(0 == strcmp(strParamter[0].c_str(),"set"))
					{
						if(strParamter.size() >= 2)
							set(strParamter);
						else
							printf("A3MessageClient>>invaild set command format:set <user.>key=value\n");
					}
					
					else if(!stricmp(key.c_str(),"get"))
					{
						getContentInfo();
					}
					else if(!stricmp(key.c_str(),"exec1"))
					{
						bool bRet = _a3MessageReq.GetVolumeInfoReq(_content.volumeName);
						if(!bRet)
							printf("A3MessageClient>>failed to send Get Volume Info request\n");
					}
					else if(!stricmp(key.c_str(),"exec2"))
					{
						bool bRet = _a3MessageReq.GetContentInfoReq(_content.volumeName, _content.pid, _content.paid);
						if(!bRet)
							printf("A3MessageClient>>failed to send Get Content Info request\n");
					}
					else if(!stricmp(key.c_str(),"exec3"))
					{
						bool bRet = _a3MessageReq.TransferContentReq(_content);
						if(!bRet)
							printf("A3MessageClient>>failed to send Transfer Content request\n");
					}
					else if(!stricmp(key.c_str(),"exec4"))
					{
						bool bRet = _a3MessageReq.CancelTransferReq(_content.volumeName, _content.pid, _content.paid, _content.reasonCode);
						if(!bRet)
							printf("A3MessageClient>>failed to send Cancel Transfer request\n");
					}
					else if(!stricmp(key.c_str(),"exec5"))
					{
						bool bRet = _a3MessageReq.DeleteContentReq(_content.volumeName, _content.pid, _content.paid, _content.reasonCode);
						if(!bRet)
							printf("A3MessageClient>>failed to send Delete Content request\n");
					}
					else if(!stricmp(key.c_str(),"exec6"))
					{
						bool bRet = _a3MessageReq.GetContentChecksumReq(_content.volumeName, _content.pid, _content.paid,_content.responseURL);
						if(!bRet)
							printf("A3MessageClient>>failed to send Get Content Checksum request\n");
					}
					else if(!stricmp(key.c_str(),"exec7"))
					{
						bool bRet = _a3MessageReq.ExposeContentReq(_content.volumeName, _content.pid, _content.paid, _content.protocol, _content.transferBitRate);
						if(!bRet)
							printf("A3MessageClient>>failed to send Expose Content request\n");
					}
					else if(!stricmp(key.c_str(),"exec8"))
					{
						bool bRet = _a3MessageReq.GetTransferStatusReq(_content.volumeName, _content.pid, _content.paid);
						if(!bRet)
							printf("A3MessageClient>>failed to send Get Transfer Status request\n");
					}
					else if(!stricmp(key.c_str(),"reset"))
					{
						reset();
					}
					else if(!stricmp(key.c_str(),"resetTime"))
					{
						resetTime();
					}
					else if(!stricmp(key.c_str(),"quit") || !stricmp(strParamter[0].c_str(),"exit"))
					{
						_bQuit = true;
					}
					else if(!stricmp(key.c_str(),"help") || !stricmp(strParamter[0].c_str(),"-h"))
					{
						showHelp();
					}
					else
						std::cout <<"A3MessageClient>>unknown command \n";

				}
			}
			Sleep(2000);
			_bQuit = true;
			continue;
		}
		
		//read from command line		
		printf("A3MessageClient>>");
		char* pStr = gets(strbuf);
		std::vector<std::string>strParamter;
		strParamter = ZQ::common::stringHelper::split(strbuf, ' ');
		if(strParamter.size() > 0)
		{ 
			if(!stricmp(strParamter[0].c_str(),"set"))
			{
              if(strParamter.size() >= 2)
			  {
				  set(strParamter);
			  }
			  else
			  {
				  printf("A3MessageClient>>invaild set command format:set <user.>key=value\n");
			  }
			}
			else if(!stricmp(strParamter[0].c_str(),"get"))
			{
				getContentInfo();
			}
			else if(!stricmp(strParamter[0].c_str(),"exec1"))
			{
				bool bRet = _a3MessageReq.GetVolumeInfoReq(_content.volumeName);
				if(!bRet)
					printf("A3MessageClient>>failed to send Get Volume Info request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec2"))
			{
				bool bRet = _a3MessageReq.GetContentInfoReq(_content.volumeName, _content.pid, _content.paid);
				if(!bRet)
					printf("A3MessageClient>>failed to send Get Content Info request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec3"))
			{
				bool bRet = _a3MessageReq.TransferContentReq(_content);
				if(!bRet)
					printf("A3MessageClient>>failed to send Transfer Content request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec4"))
			{
				bool bRet = _a3MessageReq.CancelTransferReq(_content.volumeName, _content.pid, _content.paid, _content.reasonCode);
				if(!bRet)
					printf("A3MessageClient>>failed to send Cancel Transfer request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec5"))
			{
				bool bRet = _a3MessageReq.DeleteContentReq(_content.volumeName, _content.pid, _content.paid, _content.reasonCode);
				if(!bRet)
					printf("A3MessageClient>>failed to send Delete Content request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec6"))
			{
				bool bRet = _a3MessageReq.GetContentChecksumReq(_content.volumeName, _content.pid, _content.paid,_content.responseURL);
				if(!bRet)
					printf("A3MessageClient>>failed to send Get Content Checksum request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec7"))
			{
				bool bRet = _a3MessageReq.ExposeContentReq(_content.volumeName, _content.pid, _content.paid, _content.protocol, _content.transferBitRate);
				if(!bRet)
					printf("A3MessageClient>>failed to send Expose Content request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"exec8"))
			{
				bool bRet = _a3MessageReq.GetTransferStatusReq(_content.volumeName, _content.pid, _content.paid);
				if(!bRet)
					printf("A3MessageClient>>failed to send Get Transfer Status request\n");
			}
			else if(!stricmp(strParamter[0].c_str(),"reset"))
			{
				reset();
			}
			else if(!stricmp(strParamter[0].c_str(),"resetTime"))
			{
				resetTime();
			}
			else if(!stricmp(strParamter[0].c_str(),"quit") || !stricmp(strParamter[0].c_str(),"exit"))
			{
				_bQuit = true;
			}
			else if(!stricmp(strParamter[0].c_str(),"help") || !stricmp(strParamter[0].c_str(),"-h"))
			{
				showHelp();
			}
			else
				std::cout <<"A3MessageClient>>unknown command \n";
		}
	
	}

	return 0;
}
