// SnmpSender.cpp: implementation of the SnmpSender class.
//
//////////////////////////////////////////////////////////////////////

#include "SnmpSender.h"
#include <FileLog.h>
#include <SystemUtils.h>
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

class PublishCmd : public ZQ::common::ThreadRequest
{
public:
	PublishCmd(ZQ::common::NativeThreadPool& thpool, SnmpSender& snmpPool, const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
		:ThreadRequest(thpool), _snmpPool(snmpPool), _msg(msgStruct), 
		_mid(mid), _ctx(ctx)
	{
	}
protected:
	int run()
	{
		while(!_snmpPool._bQuit)
		{
			try
			{
				if(!_snmpPool.send(_msg))
				{
					SYS::sleep(2000);
					continue;
				}
			}
			catch(...)
			{
				SYS::sleep(2000);
				LOG(Log::L_ERROR, CLOGFMT(SnmpSender, "SNMP send message catch a exception"));
				continue;
			}

			if(g_pIMsgSender)
				g_pIMsgSender->ack(_mid, _ctx);
			break;
		}

		return 0;
	}

	void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}

private:
	SnmpSender&		_snmpPool;
	MSGSTRUCT		_msg;
	MessageIdentity	_mid;
	void*			_ctx;
};

////////////////////
//class SnmpSender
////////////////////
SnmpSender::SnmpSender(int poolSize)
	:_thPool(poolSize)
{
	_pSnmp = NULL;
	time(&_timeStamp);
	
	_bConnectOk = true;
	_bQuit = false;
	_sysLog = new SysLog("SnmpSender");
	
}

SnmpSender::~SnmpSender()
{
	if (NULL != _sysLog)
		delete _sysLog;
	_sysLog = NULL;
	Close();
}

bool SnmpSender::init()
{
	if(!initSnmp())//init snmp
		return false;

	return true;
}

void SnmpSender::AddMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
{
	bool bToSend = true;
	SnmpSenderInfo::Targets::const_iterator it;
	for(it =pSnmpSenderCfg->targets.begin(); it != pSnmpSenderCfg->targets.end(); it++)
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
	{
		if(g_pIMsgSender)
			g_pIMsgSender->ack(mid, ctx);

		return;
	}

	(new PublishCmd(_thPool, *this, msgStruct, mid, ctx))->start();
}

void SnmpSender::Close()
{
	if( !_bQuit )
	{
		_bQuit = true;
		_thPool.stop();
	}
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

bool SnmpSender::GetParFromFile(const char* pFileName)
{
	//get information form configuration file
	if(pFileName == NULL || strlen(pFileName) == 0)
	{
// 		if(plog != NULL)
// 			LOG(Log::L_ERROR,"SnmpSender::GerParFromFile() configuration file path is NULL");
		if (NULL != _sysLog)
			(*_sysLog)(Log::L_ERROR,"SnmpSender::GerParFromFile() configuration file path is NULL");
		return false;
	}

	if(pSnmpSenderCfg == NULL)
	{
		pSnmpSenderCfg = new Config::Loader< SnmpSenderInfo >("");


		if(!pSnmpSenderCfg)
		{	
// 			if(plog != NULL)
// 				LOG(Log::L_ERROR,"SnmpSender::GetParFromFile() Create SnmpConfig object error");
			if (NULL != _sysLog)
				(*_sysLog)(Log::L_ERROR,"SnmpSender::GetParFromFile() Create SnmpConfig object error");
			return false;
		}
		if(!pSnmpSenderCfg->load(pFileName))
		{
// 			if(plog != NULL)
// 				LOG(Log::L_ERROR,"SNMP not load config item from xml file:%s",pFileName);
			if (NULL != _sysLog)
				(*_sysLog)(Log::L_ERROR,"SNMP not load config item from xml file:%s",pFileName);
			return false;	
		}
		pSnmpSenderCfg->snmpRegister("");
	}
	
	try
	{
		if(plog == NULL)
		{
			plog = new ZQ::common::FileLog(pSnmpSenderCfg->logPath.c_str(), pSnmpSenderCfg->logLevel, pSnmpSenderCfg->logNumber, pSnmpSenderCfg->logSize);
		}
		if(NULL != plog)
		{
			if (NULL != _sysLog)
				delete _sysLog;
			_sysLog = NULL;
		}
	}
	catch(FileLogException& ex)
	{
#ifdef _DEBUG
		printf("SnmpSender::GetParFromFile() Catch a file log exception: %s\n",ex.getString());
#endif	
		return false;			
	}
	catch(...)
	{
		return false;
	}	

	//agent udpaddress
	_strAgentIp = pSnmpSenderCfg->agentIp;

	if(_strAgentIp.length() == 0)
		_strAgentIp = "0.0.0.0";//default udpaddress

	//targets
	if(pSnmpSenderCfg->targets.size() == 0)
	{
		LOG(Log::L_ERROR,"SnmpSender::GetParFromFile() Not get target,configuration set error");
		return false;
	}
	else
	{
		bool bHavTarget = false;
		SnmpSenderInfo::Targets::const_iterator it;
		for(it =pSnmpSenderCfg->targets.begin(); it != pSnmpSenderCfg->targets.end(); it++)
		{
			if(it->nEnabled == 1)
			{
				bHavTarget = true;
				break;
			}
		}
		if(!bHavTarget)
		{
			LOG(Log::L_ERROR,"SnmpSender::GetParFromFile() Not get target,configuration set error");
			return false;
		}
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
			LOG(Log::L_EMERG,"SNMP sender catch a exception");
			return false;
		}
		
		if (nStatus != SNMP_CLASS_SUCCESS)
		{
			LOG(Log::L_ERROR, "Cannot create Snmp: %s", _pSnmp->error_msg(nStatus));
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
	for(it =pSnmpSenderCfg->targets.begin(); it != pSnmpSenderCfg->targets.end(); it++)
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
			LOG(Log::L_ERROR,CLOGFMT(SnmpSender,"Snmp is null"));
			_bConnectOk = false;
			return false;
		}

		int nStatus = _pSnmp->trap(pdu, ctg);

		if (nStatus == SNMP_CLASS_SUCCESS)
		{
			if(!_bConnectOk)
				_bConnectOk = true;
			LOG(Log::L_DEBUG, CLOGFMT(SnmpSender, "SNMP send out a message to %s"),chUdpAddr);
		}
		else
		{
			LOG(Log::L_ERROR,CLOGFMT(SnmpSender, "SNMP not send out the message to %s,error code is %s"),chUdpAddr,_pSnmp->error_msg(nStatus));
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

