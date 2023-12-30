// ScriptRequest.h: interface for the ScriptRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTREQUEST_H__254E3B7C_344A_4242_ABC2_D72BEBB24BDA__INCLUDED_)
#define AFX_SCRIPTREQUEST_H__254E3B7C_344A_4242_ABC2_D72BEBB24BDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <NativeThreadPool.h>
#include "weiwooadmin.h"
#include "ScriptEnv.h"


class ScriptRequest : public ThreadRequest,public WeiwooAdmin
{
public:

	ScriptRequest(ZQ::common::NativeThreadPool& pool,Ice::CommunicatorPtr ptr,ScriptEnv& env);

	virtual ~ScriptRequest();

public:
	
	bool	init();
	int		run(void);
	void	final(int retcode /* =0 */, bool bCancelled /* =false */);

	FUNCDEF(ScriptPause)

	FUNCDEF(GotoCmd)
	
private:
	ScriptEnv&					_env;
	int							_iCurCommand;
	std::vector<std::string>	_vecCommand;	
protected:
	DECLEAR_CMDROUTE();
};

#endif // !defined(AFX_SCRIPTREQUEST_H__254E3B7C_344A_4242_ABC2_D72BEBB24BDA__INCLUDED_)
