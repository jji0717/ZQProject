// BaseInfoCol.cpp: implementation of the BaseInfoCol class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseInfoCol.h"
#include "HandlerGroup.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BaseInfoCol::BaseInfoCol()
{
	_pHandlerGroup = 0;
}

BaseInfoCol::~BaseInfoCol()
{

}

void BaseInfoCol::OnNewMessage(const char* line)
{
	if (_pHandlerGroup)
		_pHandlerGroup->handleMessage(line);
}
