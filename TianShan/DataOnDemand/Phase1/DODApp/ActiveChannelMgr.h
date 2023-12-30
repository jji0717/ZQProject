// ActiveChannelMgr.h: interface for the ActiveChannelMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACTIVECHANNELMGR_H__6252A679_DFC4_44BA_9B66_AF87D8155B42__INCLUDED_)
#define AFX_ACTIVECHANNELMGR_H__6252A679_DFC4_44BA_9B66_AF87D8155B42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ActiveChannel.h"
#include "DODAppEx.h"

class ActiveChannelMgr  
{
public:
	ActiveChannelMgr();
	virtual ~ActiveChannelMgr();

	ActiveChannel* create(
		DataOnDemand::ChannelPublishPointPrx& ch);
	
	bool remove(const std::string& name, bool destoryObj = true);

	ActiveChannel* get(const std::string& name);

protected:
	typedef std::map<std::string, ActiveChannel* > ActiveChannelMap;
	ActiveChannelMap	_activeChannels;
};

#endif // !defined(AFX_ACTIVECHANNELMGR_H__6252A679_DFC4_44BA_9B66_AF87D8155B42__INCLUDED_)
