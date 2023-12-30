// ActiveFolderChannel.h: interface for the ActiveFolderChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACTIVEFOLDERCHANNEL_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_)
#define AFX_ACTIVEFOLDERCHANNEL_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DODAppEx.h"
#include "ActiveChannel.h"
#include "FolderChannelImpl.h"

class ActiveLocalFolderChannel: public ActiveChannel {
public:

	ActiveLocalFolderChannel(DataOnDemand::FolderChannelExPrx& channel);
	virtual ~ActiveLocalFolderChannel();

protected:
	DataOnDemand::FolderChannelExPrx	_channelPrx;
};

class ActiveSharedFolderChannel: public ActiveChannel {
public:

	ActiveSharedFolderChannel(DataOnDemand::FolderChannelExPrx& channel);
	virtual ~ActiveSharedFolderChannel();

protected:
	DataOnDemand::FolderChannelExPrx	_channelPrx;
};

#endif // !defined(AFX_ACTIVEFOLDERCHANNEL_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_)
