// HandlerGroup.cpp: implementation of the HandlerGroup class.
//
//////////////////////////////////////////////////////////////////////

#include "HandlerGroup.h"
#include "BaseMessageHandler.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HandlerGroup::HandlerGroup()
{

}

HandlerGroup::~HandlerGroup()
{
	for (msghandler_v::iterator it = _msghandlers.begin(); it < _msghandlers.end(); it++)
	{
		if ((*it))
			delete (*it);		
	}
}

void HandlerGroup::addHandler(BaseMessageHandler* mh)
{
	_msghandlers.push_back(mh);
}

void HandlerGroup::handleMessage(const char* msg)
{
	for (msghandler_v::iterator it = _msghandlers.begin(); it < _msghandlers.end(); it++)
		(*it)->handleMessage(msg);
}

