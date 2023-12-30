// BoostRegHandler.h: interface for the BoostRegHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOOSTREGHANDLER_H__6C648631_C55F_4DB4_BE4C_5D40EA594383__INCLUDED_)
#define AFX_BOOSTREGHANDLER_H__6C648631_C55F_4DB4_BE4C_5D40EA594383__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KeyDefine.h"
#include "BaseMessageHandler.h"
#include <boost/regex.hpp>

class BoostRegHandler : public BaseMessageHandler  
{
public:
	BoostRegHandler();
	virtual ~BoostRegHandler();

	virtual bool init(int channelID, const char* syntax, ChannelMessageQueue* channelQueue);

	virtual bool handleMessage(const char* msg);

	static const char* getTypeInfo()
	{
		return KD_KV_HANDLERTYPE_BOOSTREG;
	}

protected:

	boost::regex	_regSyntax;
};

#endif // !defined(AFX_BOOSTREGHANDLER_H__6C648631_C55F_4DB4_BE4C_5D40EA594383__INCLUDED_)
