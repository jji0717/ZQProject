// ActiveDataMgr.cpp: implementation of the ActiveDataMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveDataMgr.h"
#include "ActiveFolderData.h"
#include "ActiveMsgQueueData.h"
#include "MessageQueueExImpl.h"
#include "FolderExImpl.h"

using namespace TianShanIce::Application::DataOnDemand;
using namespace ZQ::common;

ActiveDataMgr::ActiveDataMgr()
{

}

ActiveDataMgr::~ActiveDataMgr()
{

}

ActiveData* ActiveDataMgr::create (TianShanIce::Application::DataOnDemand::DataPublishPointPrx& ch)
{
	ActiveData* channel;
	std::string strpath;

	try{
		::std::string type = ch->getType();
		std::string name = ch->getName();
		if (_ActiveDatas.find(name) != _ActiveDatas.end()) {
			// channel already is in the map
			assert(false);
			return NULL;
		}
		TianShanIce::Properties pre = ch->getProperties();
		strpath = pre["Path"];

		if(type == TianShanIce::Application::DataOnDemand::dataSharedFolder)
		{
			channel = new ActiveSharedFolderData(FolderExPrx::checkedCast(ch));
			_ActiveDatas.insert(ActiveDataMap::value_type(name, channel));
		}
		else if(type == TianShanIce::Application::DataOnDemand::dataLocalFolder)
		{
			channel = new ActiveLocalFolderData(FolderExPrx::checkedCast(ch),(char*)strpath.c_str(),gDODAppServiceConfig.lLocalFoldNotifytime);
			_ActiveDatas.insert(ActiveDataMap::value_type(name, channel));
		}
		else if(type == TianShanIce::Application::DataOnDemand::dataMessage)
		{
			channel = new ActiveMsgQueueData(MessageQueueExPrx::checkedCast(ch));
			_ActiveDatas.insert(ActiveDataMap::value_type(name, channel));
		}
		else
		{
			glog(Log::L_ERROR, CLOGFMT(ActiveDataMgr, "create() unknown channel(%s) type (%s)"), name.c_str(), type.c_str());
			return NULL;
		}
	} catch (Ice::Exception& ex) {

		glog(Log::L_ERROR, CLOGFMT(ActiveDataMgr, "create() failed to create active channel, errMsg(%s)"), ex.ice_name().c_str());
		return NULL;
	}

	if (!channel->initchannel())
	{
		delete channel;
		return NULL;
	}
	return channel;
}

bool ActiveDataMgr::remove(const std::string& name, bool destoryObj)
{
	ActiveData* channel;
	ActiveDataMap::iterator it = _ActiveDatas.find(name);
	if (it != _ActiveDatas.end()) {
		channel = it->second;
		if (channel) {
			channel->uninit();
		}

		_ActiveDatas.erase(it);
		
		if (destoryObj)
		{
			try
			{
				if(channel)
					delete channel;
				channel = NULL;
			}
			catch (...)
			{
				glog(Log::L_ERROR, CLOGFMT(ActiveDataMgr, "remove() failed to remove channel(%s)"), name.c_str());	
			}		
		}
		return true;
	}

	return false;
}

ActiveData* ActiveDataMgr::get(const std::string& name)
{
	ActiveDataMap::iterator it = _ActiveDatas.find(name);
	if (it != _ActiveDatas.end()) {
		return it->second;
	}

	return NULL;
}
