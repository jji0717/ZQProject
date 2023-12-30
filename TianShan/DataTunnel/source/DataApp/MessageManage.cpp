// MessageManage.cpp: implementation of the DBSyncServ class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "global.h"
#include "MessageManage.h"
#define DataTunnelAppMessage  "MessageQueues"
#define MESSAGEMANAGE         "MessageManage"
#define ICE_MessageDict		  "MessageInfos"
MessageManage::MessageManage(Ice::CommunicatorPtr&	communicator):
	_communicator(communicator)
{
}

MessageManage::~MessageManage()
{
}

bool MessageManage::init()
{
	std::string dbPath="";
	if (!gDODAppServiceConfig.szDODAppDBFolder.size())
	{
		char path[MAX_PATH];
		if (::GetModuleFileNameA(NULL, path, MAX_PATH-1)>0)
		{
			char* p = strrchr(path, FNSEPC);
			if (NULL !=p)
			{
				*p='\0';
				p = strrchr(path, FNSEPC);
				if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
					*p='\0';
			}

			strcat(path, FNSEPS "data" FNSEPS);
			dbPath = path;
		}
	}
	else dbPath = gDODAppServiceConfig.szDODAppDBFolder;

	if (FNSEPC != dbPath[dbPath.size()-1])
		dbPath += FNSEPS;
	try
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(MESSAGEMANAGE, 
			"opening database at path: %s"), dbPath.c_str());

		// open dictionary
		::CreateDirectoryA((dbPath + DataTunnelAppMessage FNSEPS).c_str(), NULL);

		_connCh = Freeze::createConnection(_communicator, (dbPath + DataTunnelAppMessage FNSEPS));

		_pmessageDict = new TianShanIce::Application::DataOnDemand::MessageListDict(_connCh, ICE_MessageDict);
		if(!_pmessageDict)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(MESSAGEMANAGE,
				"fail to create messagelistdict object"));
			return false;
		}
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MESSAGEMANAGE,
			"Caught ice exception: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MESSAGEMANAGE, 
			"Caught unknown exception (%d)"), GetLastError());
		return false;
	}
	return true;
}

int MessageManage::addMessage(const std::string& datappname, const TianShanIce::Application::DataOnDemand::messageinfo& message)
{
	IceUtil::LockT<IceUtil::RecMutex> lk(_dictLock);
	int nret = InsertPos::unknown;
	TianShanIce::Application::DataOnDemand::MessageListDict::iterator itorMsgList;
	itorMsgList =  _pmessageDict->find(datappname);

	if(itorMsgList == _pmessageDict->end())
	{
		TianShanIce::Application::DataOnDemand::MessageInfos msginfos;
		msginfos.push_back(message);
		try
		{
			_pmessageDict->put(TianShanIce::Application::DataOnDemand::MessageListDict::value_type(datappname, msginfos));
			nret = InsertPos::first;
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
				"addMessage()'%s' caught ice exception '%s'"),
				datappname.c_str(), ex.ice_name().c_str());
			nret = InsertPos::unknown;
		}
	}
	else
	{
		TianShanIce::Application::DataOnDemand::MessageInfos msginfos = itorMsgList->second;
		TianShanIce::Application::DataOnDemand::MessageInfos::iterator itormsg;
		if(message.bForever)
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
				"addMessage()'%s' this message(%s) is a forever message"),
				datappname.c_str(), message.messageID.c_str());

			msginfos.push_back(message);
			nret = InsertPos::last;
		}
		else
		{  
			itormsg = msginfos.begin();
			while(itormsg != msginfos.end())
			{
				if(itormsg->bForever)
					break;

				int seconds = message.deleteTime - itormsg->deleteTime;
				if(seconds > 0 )	
					itormsg++;
				else
					break;
			}

			if(itormsg == msginfos.begin())
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MESSAGEMANAGE,
					"addMessage()'%s' this message (%s) is Push_front"),
					datappname.c_str(),message.messageID.c_str());
				msginfos.push_front(message);
				nret = InsertPos::first;
			}
			else
				if(itormsg != msginfos.end())
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
						"addMessage()'%s' this kessage(%s) is Push_middle"),
						datappname.c_str(),message.messageID.c_str());
					msginfos.insert(itormsg, message);
					nret = InsertPos::middel;
				}
				else
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
						"addMessage()'%s' this message(%s) is Push_back"),
						datappname.c_str(),message.messageID.c_str());
					msginfos.push_back(message);
					nret = InsertPos::last;
				}
		}
		try
		{
			_pmessageDict->put(TianShanIce::Application::DataOnDemand::MessageListDict::value_type(datappname, msginfos));
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
				"addMessage()'%s' caught ice exception '%s'"),
				datappname.c_str(), ex.ice_name().c_str());
			nret = InsertPos::unknown;
		}
	}
	return nret;
}

bool
MessageManage::deleteMessage(const std::string& datappname, const std::string& messageId)
{
	IceUtil::LockT<IceUtil::RecMutex> lk(_dictLock);
	bool bret = false;
	long msgcount = 0;
	TianShanIce::Application::DataOnDemand::MessageListDict::iterator itorMsgList;
	itorMsgList =  _pmessageDict->find(datappname);

	if(itorMsgList != _pmessageDict->end())
	{
		TianShanIce::Application::DataOnDemand::MessageInfos msginfos = itorMsgList->second;
		TianShanIce::Application::DataOnDemand::MessageInfos::iterator itormsg;
		for(itormsg = msginfos.begin(); itormsg != msginfos.end(); itormsg++)
		{
			if(itormsg->messageID == messageId)
			{
				msginfos.erase(itormsg);
				try
				{
					_pmessageDict->put(TianShanIce::Application::DataOnDemand::MessageListDict::value_type(datappname, msginfos));
				}
				catch (Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
						"deleteMessage()'%s' caught ice exception '%s'"),
						datappname.c_str(), ex.ice_name().c_str());
					return false;
				}
				break;
				return true;
			}
		}
	}
	return false;
}
bool
MessageManage::modityMessage(const std::string& datappname,const std::string& messageId)
{
	IceUtil::LockT<IceUtil::RecMutex> lk(_dictLock);
	bool bret = false;
	long msgcount = 0;
	time_t deletime;
	TianShanIce::Application::DataOnDemand::messageinfo msginfo;
	TianShanIce::Application::DataOnDemand::MessageListDict::iterator itorMsgList;
	itorMsgList =  _pmessageDict->find(datappname);

	if(itorMsgList != _pmessageDict->end())
	{
		TianShanIce::Application::DataOnDemand::MessageInfos msginfos = itorMsgList->second;
		TianShanIce::Application::DataOnDemand::MessageInfos::iterator itormsg;
		for(itormsg = msginfos.begin(); itormsg != msginfos.end(); itormsg++)
		{
			if(itormsg->messageID == messageId)
			{	
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MESSAGEMANAGE,
					"modityMessage() '%s' find deleted message(%s)"),
					datappname.c_str(),messageId.c_str());
				msginfo = *itormsg;
				time(&deletime ); 
				msginfo.deleteTime  = deletime;
				msginfo.bForever = FALSE;
				msginfos.erase(itormsg);
				msginfos.push_front(msginfo);
				try
				{
					_pmessageDict->put(TianShanIce::Application::DataOnDemand::MessageListDict::value_type(datappname, msginfos));
				}
				catch (Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
						"modityMessage()'%s' caught ice exception '%s'"),
						datappname.c_str(), ex.ice_name().c_str());
					return false;
				}
				return true;
			}
		}
	}
	return false;
}
bool 
MessageManage::getMessage(const std::string& datappname, TianShanIce::Application::DataOnDemand::messageinfo& message)
{
	IceUtil::LockT<IceUtil::RecMutex> lk(_dictLock);
	long msgcount = 0;
	TianShanIce::Application::DataOnDemand::MessageListDict::iterator itorMsgList;
	itorMsgList =  _pmessageDict->find(datappname);

	if(itorMsgList != _pmessageDict->end())
	{
		if(itorMsgList->second.size()> 0)
		{
			message = itorMsgList->second.front();
			return true;
		}
		else
			return false;		
	}
	return false;
}

long  
MessageManage::getMessageCount(const std::string& datappname)
{
	IceUtil::LockT<IceUtil::RecMutex> lk(_dictLock);
	long msgcount = 0;
	TianShanIce::Application::DataOnDemand::MessageListDict::iterator itorMsgList;
	itorMsgList =  _pmessageDict->find(datappname);

	if(itorMsgList != _pmessageDict->end())
		msgcount = itorMsgList->second.size();
	return msgcount;
}
bool MessageManage::removeDataPublishPointMessage(const std::string& datappname)
{
	IceUtil::LockT<IceUtil::RecMutex> lk(_dictLock);
	long msgcount = 0;
	TianShanIce::Application::DataOnDemand::MessageListDict::iterator itorMsgList;
	itorMsgList =  _pmessageDict->find(datappname);

	if(itorMsgList != _pmessageDict->end())
	{		
		try
		{
             _pmessageDict->erase(itorMsgList);	
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(MESSAGEMANAGE,
				"modityMessage()'%s' caught ice exception '%s'"),
				datappname.c_str(), ex.ice_name().c_str());
			return false;
		}
		return true;
	}
	return false;
}