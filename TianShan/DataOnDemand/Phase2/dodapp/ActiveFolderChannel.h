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
#include "dirconsumer.h"
#include "Locks.h"
class ActiveLocalFolderChannel: public ActiveChannel,
                                public DirConsumer {
public:

	ActiveLocalFolderChannel(DataOnDemand::FolderChannelExPrx& channel, 
		                     char* dir,
							 long monitorTime);
	virtual ~ActiveLocalFolderChannel();
	bool initchannel(void);
	void uninit();

protected:
	DataOnDemand::FolderChannelExPrx	_channelPrx;
	std::string _contentpath;
	ZQ::common::Mutex _mutex;
protected:
    bool NotifyMuxItem();
	bool WrapData();
	virtual bool notifyFoldChange();
};

class ActiveSharedFolderChannel: public ActiveChannel {
public:

	ActiveSharedFolderChannel(DataOnDemand::FolderChannelExPrx& channel);
	virtual ~ActiveSharedFolderChannel();
    bool NotifyMuxItem(int groupId, int verNumber);

protected:
	bool WrapData(int groupId,std::string& contentpath, int verNumber);

protected:
	DataOnDemand::FolderChannelExPrx	_channelPrx;
    ZQ::common::Mutex _mutex;
};

#endif // !defined(AFX_ACTIVEFOLDERCHANNEL_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_)
