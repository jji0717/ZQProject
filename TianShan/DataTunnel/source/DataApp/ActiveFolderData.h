// ActiveFolderData.h: interface for the ActiveFolderData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ActiveFolderData_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_)
#define AFX_ActiveFolderData_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataAppEx.h"
#include "ActiveData.h"
#include "FolderExImpl.h"
#include "dirconsumer.h"
#include "Locks.h"
class ActiveLocalFolderData: public ActiveData,
                                public DirConsumer {
public:

	ActiveLocalFolderData(TianShanIce::Application::DataOnDemand::FolderExPrx& channel, 
		                     char* dir,
							 long monitorTime);
	virtual ~ActiveLocalFolderData();
	bool initchannel(void);
	void uninit();

protected:
	TianShanIce::Application::DataOnDemand::FolderExPrx	_dataPPPrx;
	std::string _contentpath;
	ZQ::common::Mutex _mutex;
protected:
    bool NotifyMuxItem();
	bool WrapData();
	virtual bool notifyFoldChange();
};

class ActiveSharedFolderData: public ActiveData {
public:

	ActiveSharedFolderData(TianShanIce::Application::DataOnDemand::FolderExPrx& channel);
	virtual ~ActiveSharedFolderData();
    bool NotifyMuxItem(int groupId, int verNumber);

protected:
	bool WrapData(int groupId,std::string& contentpath, int verNumber);

protected:
	TianShanIce::Application::DataOnDemand::FolderExPrx	_dataPPPrx;
    ZQ::common::Mutex _mutex;
};

#endif // !defined(AFX_ActiveFolderData_H__716A31CB_C5E8_41BA_B32E_06C54C365922__INCLUDED_)
