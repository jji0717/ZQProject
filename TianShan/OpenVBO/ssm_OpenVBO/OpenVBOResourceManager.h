#ifndef __EVENT_IS_VODI5_RESOURCE_MANAGER_H__
#define __EVENT_IS_VODI5_RESOURCE_MANAGER_H__

#include "SelectionResourceManager.h"

class OpenVBOResourceManager : public NgodResourceManager
{
public:
	OpenVBOResourceManager(SelectionEnv& env);
	~OpenVBOResourceManager();
public:
	//virtual const ResourceStreamerAttrMap findByIdentifier( const std::string& identifier ) const;
	//virtual ResourceStreamerAttrMap findByIdentifier( const std::string& identifier );

	virtual NgodResourceManager::StreamerIterMap	findByIdentifier( const std::string& identifier ) ;


};

#endif