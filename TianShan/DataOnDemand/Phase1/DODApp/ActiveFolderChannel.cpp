// ActiveFolderChannel.cpp: implementation of the ActiveFolderChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveFolderChannel.h"
using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ActiveLocalFolderChannel::ActiveLocalFolderChannel(
	DataOnDemand::FolderChannelExPrx& channel): 
	_channelPrx(channel)
{

}

ActiveLocalFolderChannel::~ActiveLocalFolderChannel()
{
	glog(Log::L_DEBUG, "~ActiveLocalFolderChannel():DeleteObject success!");
}

//////////////////////////////////////////////////////////////////////////


ActiveSharedFolderChannel::ActiveSharedFolderChannel(
	DataOnDemand::FolderChannelExPrx& channel): 
	_channelPrx(channel)
{

}

ActiveSharedFolderChannel::~ActiveSharedFolderChannel()
{
	glog(Log::L_DEBUG, "~ActiveSharedFolderChannel():DeleteObject success!");
}
