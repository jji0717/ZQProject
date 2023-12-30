// ActiveChannelMgr.cpp: implementation of the ActiveChannelMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveChannelMgr.h"
#include "ActiveFolderChannel.h"
#include "ActiveMsgChannel.h"
#include "MessageChannelImpl.h"
#include "FolderChannelImpl.h"

using namespace DataOnDemand;
using namespace ZQ::common;

ActiveChannelMgr::ActiveChannelMgr()
{

}

ActiveChannelMgr::~ActiveChannelMgr()
{

}

ActiveChannel* ActiveChannelMgr::create (
	ChannelPublishPointPrx& ch)
{
	ActiveChannel* channel;
	std::string strpath;
	// ActiveLocalFolderChannel* localFolderChannel;
	// ActiveSharedFolderChannel* sharedFolderChannel;
	// ActiveMsgChannel* msgChannel;

	try{
		ChannelType type = ch->getType();
		std::string name = ch->getName();
		if (_activeChannels.find(name) != _activeChannels.end()) {
			// channel already is in the map
			assert(false);
			return NULL;
		}
		TianShanIce::Properties pre = ch->getProperties();
		strpath = pre["Path"];

		switch(type) {
		case dodSharedFolder:
			channel = new ActiveSharedFolderChannel(
				FolderChannelExPrx::checkedCast(ch));
			_activeChannels.insert(
				ActiveChannelMap::value_type(name, channel));
			break;

		case dodLocalFolder:			
			channel = new ActiveLocalFolderChannel(
				FolderChannelExPrx::checkedCast(ch),
				         (char*)strpath.c_str(),
						 gDODAppServiceConfig.lLocalFoldNotifytime);
			_activeChannels.insert(
				ActiveChannelMap::value_type(name, channel));
			break;

		case dodMessage:

			channel = new ActiveMsgChannel(
				MessageChannelExPrx::checkedCast(ch));
			_activeChannels.insert(
				ActiveChannelMap::value_type(name, channel));
			break;

		}
	} catch (Ice::Exception& e) {

		glog(Log::L_ERROR, "ActiveChannelMgr::create() failed (%s).", 
			e.ice_name().c_str());

		return NULL;
	}

	if (!channel->initchannel())
	{
		delete channel;
		return NULL;
	}
	return channel;
}

bool ActiveChannelMgr::remove(const std::string& name, bool destoryObj)
{
	ActiveChannel* channel;
	ActiveChannelMap::iterator it = _activeChannels.find(name);
	if (it != _activeChannels.end()) {
		channel = it->second;
		if (channel) {
			channel->uninit();
		}

		_activeChannels.erase(it);
		
		if (destoryObj)
		{
			try
			{
				delete channel;
				channel = NULL;
			}
			catch (...)
			{
				glog(Log::L_ERROR, "ActiveChannelMgr::remove() failed (%s).", 
					name.c_str());	
			}		
		}
		return true;
	}

	return false;
}

ActiveChannel* ActiveChannelMgr::get(const std::string& name)
{
	ActiveChannelMap::iterator it = _activeChannels.find(name);
	if (it != _activeChannels.end()) {
		return it->second;
	}

	return NULL;
}
