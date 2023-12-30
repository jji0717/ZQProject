#pragma once

#include <DataAppEx.h>
#include <Freeze/Freeze.h>
#include "DataAppImpl.h"
#include "Util.h"
#include "DataPublishPointImpl.h"
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {

class FolderExImpl: public DataPublishPointImpl<FolderEx>, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
	
public:	
	FolderExImpl();
	virtual ~FolderExImpl();

	virtual void onFullUpdate(::Ice::Int groupId,const ::std::string&,
			  bool,	int verNumber,const Ice::Current&);

    virtual void onPartlyUpdate(::Ice::Int groupId, const ::std::string&,
				    const ::std::string&,int verNumber,const Ice::Current&);

    virtual void onFolderDeleted(::Ice::Int groupId,const ::std::string&,
		                         int verNumber,const Ice::Current&);

    virtual void onFileDeleted(::Ice::Int groupId,const ::std::string&,
                              int verNumber,const Ice::Current&);

    virtual void onFileModified(::Ice::Int groupId,const ::std::string&,
				    const ::std::string&, int verNumber, const Ice::Current&);

    virtual void onFileAdded(::Ice::Int groupId,const ::std::string&,
				 const ::std::string&,int verNumber,const Ice::Current&);

	virtual ::std::string getContentName(::Ice::Int groupId, const Ice::Current&);
	virtual void setContentName(::Ice::Int groupId, const ::std::string&,
										const Ice::Current&);
public:
	virtual bool init();
	void NotifyMuxItem(int groupId, int verNumber);
};
} // END DataOnDemand
} // END Application
} // END TianshanICE
