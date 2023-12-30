// SnmpSender.cpp: implementation of the SnmpSender class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SnmpSender.h"
#include <snmp_pp/uxsnmp.h>
#include "FileSystemOp.h"

#ifdef ZQ_OS_MSWIN
#include <io.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//oid
#define OID_zqEnterprise					"1.3.6.1.4.1.22839"//enterprise oid

#define OID_serverTrap                      "1.3.6.1.4.1.22839.3.5"
#define OID_serverTrapName                  OID_serverTrap ".1.1" //trap name, DisplayString
#define OID_serverTrapCategory              OID_serverTrap ".1.2" //trap category,DisplayString
#define OID_serverTrapTime                  OID_serverTrap ".1.3" //trap timestamp,DisplayString
#define OID_serverTrapNetID					OID_serverTrap ".1.4" //trap source net id,DisplayString
#define OID_serverTrapDescription           OID_serverTrap ".1.5" //trap description,DisplayString
#define OID_serverTrapSeverity              OID_serverTrap ".1.6" //trap severity(DEBUG,INFO...),default value is:INFO,DisplayString

#define OID_serverTrapGenericEvent			OID_serverTrap ".2.1"//ZQ server trap generic event for notify id

#define SNMP_DEFAULT_SEVERITY				"INFO"

using namespace ZQ::common;
#define MAX_BUFSIZE  8192  //8k
SnmpSender::SnmpSender():_quit(false),_hMsgSem(10),_pSnmp(0),_hFile(0),
_dwPos(0),_nDequeSize(50),_bConnectOk(true)
{
	time(&_timeStamp);
}

SnmpSender::~SnmpSender()
{
	Close();
}

bool SnmpSender::init()
{
	if(!initSnmp())//init snmp
		return false;

	return start();
}

int SnmpSender::run()
{
	bool bReadF = false;//read event from file
	while(!_quit)
	{
		if(!_bConnectOk)
		{
			for(int nWait = 0; nWait < 40; nWait ++)
			{
				if(_quit)
					return 0;
				SYS::sleep(1000);
			}
			_bConnectOk = true;
			_hMsgSem.post();
		}
		//wait for a message
		_hMsgSem.wait();
		if (_quit) //exit
		{
			glog(Log::L_DEBUG,"SNMP wait a exit object");
			break;
		}
		
		//send message from file first if need
		if(_bConnectOk && bReadF)
		{
			bReadF = ReadEventFromFile();
		}

		while (_msgQue.size())
		{
			if(int(_msgQue.size()) > _nDequeSize)//if should save some message to file
			{
				bReadF = true;
				SaveEventToFile(_msgQue);
			}

			if(_bConnectOk && bReadF)
			{
				bReadF = ReadEventFromFile();
			}
			else if(_bConnectOk && !bReadF)
			{				
				bool bGetMsg = false;
				MSGSTRUCT msg;
				{			
					ZQ::common::MutexGuard MG(_lock);
					msg = _msgQue.front();
					_msgQue.pop_front();
					bGetMsg = true;
				}
				if(!bGetMsg)
					continue;
				try
				{
					if(!send(msg))
					{
						ZQ::common::MutexGuard MG(_lock);
						_msgQue.push_front(msg);
						break;
					}	
				}
				catch(...)
				{
					glog(Log::L_ERROR,"SNMP send message catch a exception");
					_bConnectOk = false;
					ZQ::common::MutexGuard MG(_lock);
					_msgQue.push_front(msg);
					break;
				}
			}
			else
				break;
		}		
	}

	glog(Log::L_INFO,"SNMP sender thread exit");

	return 0;
}

void SnmpSender::AddMessage(const MSGSTRUCT& msgStruct)
{
	bool bToSend = true;
	SnmpSenderInfo::Targets::const_iterator it;
	for(it =gConfig.targets.begin(); it != gConfig.targets.end(); it++)
	{
		if(it->nEnabled != 1)
			continue;
		
		bToSend = true;
		Target::Filters::const_iterator itF;
		for(itF = it->filters.begin(); itF != it->filters.end(); itF++)
		{
			bToSend = true;
			if(itF->strCategory.length() && stricmp((itF->strCategory).c_str(),msgStruct.category.c_str()) != 0)
			{
				bToSend = false;
				continue;
			}
			if(itF->strEventName.length() && stricmp((itF->strEventName).c_str(),msgStruct.eventName.c_str()) != 0)
			{
				bToSend = false;
				continue;
			}
			if(bToSend)
				break;
		}

		if(bToSend)
			break;
	}

	if(!bToSend)
		return;

	{	
		ZQ::common::MutexGuard MG(_lock);
		_msgQue.push_back(msgStruct);	
	}
	_hMsgSem.post();	
}

void SnmpSender::Close()
{
    _quit = true;
    _hMsgSem.post();
    waitHandle(3000);

    _msgQue.clear();
    if(_hFile)
    {
        fclose(_hFile);
        _hFile = 0;
    }

    FS::remove(_strSaveName);

	if(_pSnmp != NULL)
	{
		try
		{
			Snmp::socket_cleanup();
			delete _pSnmp;
			_pSnmp = NULL;
		}
		catch(...){ };		
	}
}

bool SnmpSender::GetParFromFile()
{

	//agent udpaddress
	_strAgentIp = gConfig.agentIp;

	if(_strAgentIp.length() == 0)
		_strAgentIp = "0.0.0.0";//default udpaddress

	//targets
	if(gConfig.targets.size() == 0)
	{
		glog(Log::L_ERROR,"SnmpSender::GetParFromFile() Not get target,configuration set error");
		return false;
	}
	else
	{
		bool bHavTarget = false;
		SnmpSenderInfo::Targets::const_iterator it;
		for(it =gConfig.targets.begin(); it != gConfig.targets.end(); it++)
		{
			if(it->nEnabled == 1)
			{
				bHavTarget = true;
				break;
			}
		}
		if(!bHavTarget)
		{
			glog(Log::L_ERROR,"SnmpSender::GetParFromFile() Not get target,configuration set error");
			return false;
		}
	}

	_nDequeSize = gConfig.dequeSize;
	_strSaveName = gConfig.savePath;

	size_t nIndex = _strSaveName.rfind(FNSEPS);
	if(nIndex != std::string::npos)
		FS::createDirectory(_strSaveName.substr(0, nIndex), true);

	//create file to save message when network has problem

    _hFile = fopen(_strSaveName.c_str(), "w+b");
	if(!_hFile)
	{
		glog(Log::L_ERROR,"SNMP createFile %s failed",_strSaveName.c_str());
		return false;
	}

	return true;
}

bool SnmpSender::initSnmp()
{
	if(_pSnmp == NULL)
	{
		int nStatus;
		try
		{
			Snmp::socket_startup();	
			_pSnmp = new Snmp(nStatus);
		}
		catch(...)
		{
			glog(Log::L_EMERG,"SNMP sender catch a exception");
			return false;
		}
		
		if (nStatus != SNMP_CLASS_SUCCESS)
		{
			glog(Log::L_ERROR, "Cannot create Snmp: %s", _pSnmp->error_msg(nStatus));
			return false;
		}
	}	

	return true;
}

bool SnmpSender::send(MSGSTRUCT &msg)
{
	Pdu pdu;	
	Vb vbName, vbCat, vbTime, vbNetId, vbDes, vbLevel;

	SetTrapHeader(pdu);
	
	vbName.set_oid(OID_serverTrapName);
	vbName.set_value(msg.eventName.c_str());
	pdu += vbName;
	vbCat.set_oid(OID_serverTrapCategory);
	vbCat.set_value(msg.category.c_str());
	pdu += vbCat;
	vbTime.set_oid(OID_serverTrapTime);
	vbTime.set_value(msg.timestamp.c_str());
	pdu += vbTime;
	vbNetId.set_oid(OID_serverTrapNetID);
	vbNetId.set_value(msg.sourceNetId.c_str());
	pdu += vbNetId;

	vbDes.set_oid(OID_serverTrapDescription);
	std::string strval;
	std::string strLevel;
	if(msg.property.size() > 0)
	{	
		std::map<std::string,std::string>::iterator it;		
		for(it = msg.property.begin(); it != msg.property.end(); it++)
		{
			if(stricmp(it->first.c_str(),"#snmp.severity") == 0)
			{
				strLevel = it->second;
				continue;
			}
			strval += it->first + "=" + it->second + ";";						
		}
	}
	vbDes.set_value(strval.c_str());
	pdu += vbDes;

	if(strLevel.size() < 4)
		strLevel = SNMP_DEFAULT_SEVERITY;
	vbLevel.set_oid(OID_serverTrapSeverity);
	vbLevel.set_value(strLevel.c_str());
	pdu += vbLevel;
	
	SnmpSenderInfo::Targets::const_iterator it;
	for(it =gConfig.targets.begin(); it != gConfig.targets.end(); it++)
	{
		if(it->nEnabled != 1)
			continue;

		bool bneedSend = true;//send out if no filters
		Target::Filters::const_iterator itF;
		for(itF = it->filters.begin(); itF != it->filters.end(); itF++)
		{
			bneedSend = false;
			if(itF->strCategory.length() && stricmp((itF->strCategory).c_str(),msg.category.c_str()) != 0)
				continue;
			if(itF->strEventName.length() && stricmp((itF->strEventName).c_str(),msg.eventName.c_str()) != 0)
				continue;
			bneedSend = true;
			break;
		}
		if(!bneedSend)
			continue;

		char chUdpAddr[30] = {0};
		sprintf(chUdpAddr,"%s:%d", it->ipAddress.c_str(), it->port);
		std::string comm = it->community;
		UdpAddress addTrg(chUdpAddr);
		//set target 
		CTarget ctg(addTrg);
		
		{	
			snmp_version version = version1;
			ctg.set_version(version);
			OctetStr community(comm.c_str());
			ctg.set_readcommunity(community);
		}

		
		if (_pSnmp == NULL)
		{
			glog(Log::L_ERROR,"Snmp is null");
			_bConnectOk = false;
			return false;
		}

		int nStatus = _pSnmp->trap(pdu, ctg);

		if (nStatus == SNMP_CLASS_SUCCESS)
		{
			if(!_bConnectOk)
				_bConnectOk = true;
			glog(Log::L_DEBUG,"SNMP send out a message to %s",chUdpAddr);
		}
		else
		{
			glog(Log::L_ERROR,"SNMP not send out the message to %s,error code is %s",chUdpAddr,_pSnmp->error_msg(nStatus));
			_bConnectOk = false;
			break;
		}
	}
	SYS::sleep(5);
	return _bConnectOk;
}

bool SnmpSender::SetTrapHeader(Pdu& pdu)
{
	Oid oid(OID_serverTrapGenericEvent);
	pdu.set_notify_id(oid);
	Oid ent(OID_zqEnterprise);
	pdu.set_notify_enterprise(ent);

	IpAddress addSender(_strAgentIp.c_str());
	pdu.set_v1_trap_address(addSender);

	time_t curtime;
	time(&curtime);
	double lftime = difftime(curtime, _timeStamp);

	unsigned long ultime = (unsigned long)(lftime * 100);

	TimeTicks tt(ultime);
	pdu.set_notify_timestamp(tt);

	return true;
}

bool SnmpSender::ReadEventFromFile()
{
	if(!_hFile)  //failed
	{
		glog(Log::L_ERROR,"Read event from file error ,SNMP file handle is invalid");
		return false;
	}
	
    fseek(_hFile, 0L, SEEK_END);
    long dwS = ftell(_hFile);
	if(dwS == 0)
	{
		glog(Log::L_DEBUG,"SNMP read file exit ,file size is 0");
		return false;
	}
	if(dwS == _dwPos) //set file size zero
	{
#ifdef ZQ_OS_MSWIN
		_chsize(fileno(_hFile), 0);
#else
        ftruncate(fileno(_hFile), 0);
#endif
		_dwPos = 0;

		glog(Log::L_DEBUG,"SNMP set file length zero");
		return false;
	}
	
	char* buf = new char[sizeof(char)*MAX_BUFSIZE];
	memset(buf,0,sizeof(char)*MAX_BUFSIZE);

    fseek(_hFile, _dwPos, SEEK_SET);
	
	bool bHasOne = false;
	char* pBP = NULL;
	char* pBeg = NULL;
	char* pSec = NULL;
	char* pMM = NULL;
    if(fread(buf, 1, sizeof(char)*MAX_BUFSIZE-3, _hFile) > 0)
	{
		pMM = buf;
		pBP = buf;
		pBeg = buf;
		pSec = buf;		

		do{	
			MSGSTRUCT msg;
			bHasOne = false;
	
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //id
			{
				*pMM = '\0';
				msg.id = atoi(pBeg);
			}
			else
				break;
			
			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //category
			{
				*pMM = '\0';
				msg.category = pBeg;
			}
			else
				break;
			
			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //timestamp
			{
				*pMM = '\0';
				msg.timestamp = pBeg;
			}
			else
				break;

			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //eventName
			{
				*pMM = '\0';
				msg.eventName = pBeg;
			}
			else
				break;

			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //sourceNetId
			{
				*pMM = '\0';
				msg.sourceNetId = pBeg;
			}
			else
				break;
			

			pMM++;
			pBeg = pMM;
			
			if(*pMM == '\r')//no property
			{
				pMM += 2;
				bHasOne = true;
			}
			else
			{
				while(*pMM != '\r' && *pMM != '\0')
				{
						
					while(*pMM != '\n' && *pMM != '\0')
					{
						pMM++;
					}
					if(*pMM == '\n') //first
					{
						*pMM = '\0';
					}
					else
						break;

					pMM++;
					pSec = pMM;
					while(*pMM != '\r' && *pMM != '\n' && *pMM != '\0')
					{
						pMM++;
					}
					if(*pMM == '\n' && *(pMM+1) == '\r')  //second
					{
						bHasOne = true;
						*pMM = '\0';
						msg.property[pBeg] = pSec;
						pMM += 3;
					}
					if(*pMM == '\n' && *(pMM+1) != '\r')  
					{
						*pMM = '\0';
						msg.property[pBeg] = pSec;
						++pMM;
					}
					else
						break;

					pBeg = pMM;
					if(bHasOne)
						break;
				}
			}
			if(bHasOne) //post event
			{
				try
				{
					if(!send(msg))
						break;
				}
				catch(...)
				{
					glog(Log::L_ERROR,"SNMP send message catch a exception");
					_bConnectOk = false;
					break;
				}
				
				_dwPos += pMM-pBP;
				pBP = pMM;
				pBeg = pMM;
			}
			
		}while(*pMM != '\0');
	}

	if(dwS == _dwPos) //read end of the file
	{
#ifdef ZQ_OS_MSWIN
		_chsize(fileno(_hFile), 0);
#else
        ftruncate(fileno(_hFile), 0);
#endif
		_dwPos = 0;
		glog(Log::L_DEBUG,"SNMP set file length zero");
	}

	delete[] buf;
	return true;
}

bool SnmpSender::SaveEventToFile(std::deque<MSGSTRUCT>& deq)
{

	if(!_hFile)
	{
		glog(Log::L_ERROR,"SNMP save event to file error ,file handle is invalid");
		return false;
	}

    fseek(_hFile, 0L, SEEK_END);
	int count = _nDequeSize>100 ? 100 : _nDequeSize;
	
	while(count--)
	{
		MSGSTRUCT msg;
		{
			ZQ::common::MutexGuard MG(_lock);
			msg = _msgQue.front();
			_msgQue.pop_front();
		}

		char a[10] = {0};
		sprintf(a,"%d",msg.id);
		std::string text = "";
		text = a;				//id
		text += "\n";
		text += msg.category;	//category
		text += "\n";
		text += msg.timestamp;	//timestamp
		text += "\n";
		text += msg.eventName;  //eventName
		text += "\n";
		text += msg.sourceNetId;//sourceNetId
		text += "\n";

		for(std::map<std::string,std::string>::iterator itmap=msg.property.begin(); itmap!=msg.property.end(); itmap++)
		{
			text += itmap->first + "\n";
			text += itmap->second + "\n";
		}
		text += "\r\n";

        fwrite(text.c_str(), 1, text.size(), _hFile);
        if(ferror(_hFile))
		{
			glog(Log::L_ERROR,"SNMP write file error");
			return false;
		}
			
	}

	return true;
}
