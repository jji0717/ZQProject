#pragma once

#include <DODAppEx.h>
#include <Freeze/Freeze.h>
#include "DODAppImpl.h"
#include "Util.h"
#include "ChannelPublishPointImpl.h"
namespace DataOnDemand {

class FolderChannelImpl: public ChannelPublishPointImpl<FolderChannelEx>, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
	
public:	
	FolderChannelImpl();
	virtual ~FolderChannelImpl();

	virtual void notifyFullUpdate(const ::std::string&,
			  bool,
			  ::Ice::Int groupId, 
			  const Ice::Current&);

    virtual void notifyPartlyUpdate(const ::std::string&,
				    const ::std::string&,
					::Ice::Int groupId, 
				    const Ice::Current&);

    virtual void notifyFolderDeleted(const ::std::string&,
		::Ice::Int groupId, const Ice::Current&);

    virtual void notifyFileDeleted(const ::std::string&,
		::Ice::Int groupId, const Ice::Current&);

    virtual void notifyFileModified(const ::std::string&,
				    const ::std::string&,
					::Ice::Int groupId, 
				    const Ice::Current&);

    virtual void notifyFileAdded(const ::std::string&,
				 const ::std::string&,
				 ::Ice::Int groupId, 
				 const Ice::Current&);				 	

public:
	virtual bool init();

private:
	bool NotityMuxItem(int groupId);
};

}
