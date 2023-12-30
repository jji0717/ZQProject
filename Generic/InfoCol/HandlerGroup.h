// HandlerGroup.h: interface for the HandlerGroup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HANDLERGROUP_H__7F370A83_2BBA_4015_B464_E29CAF7010C8__INCLUDED_)
#define AFX_HANDLERGROUP_H__7F370A83_2BBA_4015_B464_E29CAF7010C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class BaseMessageHandler;

class HandlerGroup  
{
public:
	HandlerGroup();
	virtual ~HandlerGroup();

	/// add a message handler into the tail collector
	///@param mh  a message handler
	void addHandler(BaseMessageHandler* mh);

	int getHanderCount()
	{
		return _msghandlers.size();
	}

	/// Entry to handle a detected message
	///@param msg  the NULL-terminated detected message
	void handleMessage(const char* msg);

private:
	typedef std::vector < BaseMessageHandler* > msghandler_v;
	msghandler_v		_msghandlers;		
};

#endif // !defined(AFX_HANDLERGROUP_H__7F370A83_2BBA_4015_B464_E29CAF7010C8__INCLUDED_)
