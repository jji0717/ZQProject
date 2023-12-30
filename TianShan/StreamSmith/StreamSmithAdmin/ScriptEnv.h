// ScriptEnv.h: interface for the ScriptEnv class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTENV_H__8ED1A7DA_9611_4C73_BF3A_5FB43CCF65DE__INCLUDED_)
#define AFX_SCRIPTENV_H__8ED1A7DA_9611_4C73_BF3A_5FB43CCF65DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)

#include <string>
#include <vector>
#include "TaskScheduler.h"
#include "InitInfo.h"
#include "StreamSmithAdmin.h"


using namespace TianShanIce::Streamer;

class ScriptRequest;
class ScriptEnv  
{
public:
	ScriptEnv( ZQ::common::NativeThreadPool& pool);
	virtual ~ScriptEnv( );

public:
	bool	init(const char* pIniFile);

public:
	StreamSmithAdminPrx _adminPrx;
	typedef std::vector<std::string>	COMMANDLIST;
	COMMANDLIST			_listCmd;
	int					_playlistCount;
	int					_setupInterval;
	std::string			_strServerEndpoint;

	std::string			_strDestIP;
	int					_iDestPort;
	int					_iPortStep;


	TaskScheduler		_schedule;
	Ice::CommunicatorPtr _ic;
	NativeThreadPool&	_pool;
private:
	std::vector<ScriptRequest*>	_requests;
};

#endif // !defined(AFX_SCRIPTENV_H__8ED1A7DA_9611_4C73_BF3A_5FB43CCF65DE__INCLUDED_)
