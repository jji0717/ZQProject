// ScriptRequest.cpp: implementation of the ScriptRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptRequest.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


BEGIN_CMDROUTE(ScriptRequest,WeiwooAdmin)
	COMMAND(ScriptPause,ScriptPause)
	COMMAND(GotoCmd,GotoCmd)
END_CMDROUTE()

ScriptRequest::ScriptRequest(ZQ::common::NativeThreadPool& pool,Ice::CommunicatorPtr ptr,ScriptEnv& env)
:ZQ::common::ThreadRequest(pool),WeiwooAdmin(env._ic,env.m_sessManager),_env(env)
{
	_iCurCommand = 0;	
	_vecCommand = _env._listCmd;
}

ScriptRequest::~ScriptRequest()
{

}

int ScriptRequest::GotoCmd(VARVEC& var)
{
	if (var.size() != 1) 
	{
		err("Usage: GotoCmd timecmdIndex");
		return OPERATION_INVALIDPARAMETER;
	}
	int iIndex = atoi(var[0].c_str());
	
	_iCurCommand = iIndex ;
	return OPERATION_SUCCESS;
}

int ScriptRequest::ScriptPause(VARVEC& var)
{
	//把自己加入到Schedule里面
	if (var.size() != 1) 
	{
		err("Usage: ScriptPause time");
		return OPERATION_INVALIDPARAMETER;
	}
	DWORD dwNow = GetTickCount();
	long lTemp =atoi(var[0].c_str());
	if (lTemp < 0 )
		lTemp =0;	
	_env._schedule.watchme((void*)this,dwNow + lTemp);
	return OPERATION_FREECPU;
}

int ScriptRequest::run()
{
	if (_iCurCommand >= (int)_vecCommand.size()) 
	{
		printf("quit the request run\n");
		return 1;
	}
	std::string strCurrentCmd = _vecCommand[_iCurCommand];
	do
	{				
		try
		{
			printf("execute %d\n",_iCurCommand-1);
			int iRet = ParseCommand(strCurrentCmd);
			switch( iRet )
			{
			case OPERATION_FREECPU:
				{
					_iCurCommand ++;
					return 1;					
				}
				break;
			case OPERATION_SUCCESS:
				{
					_iCurCommand ++;					
					if (_iCurCommand >= (int)_vecCommand.size()) 
					{
						printf("quit the script run because invalid order\n");
						return 1;
					}
					strCurrentCmd = _vecCommand[_iCurCommand];
				}
				break;
			case TIANSHAN_INVALIDSTATE:
				{
					//Pause 5000 milliseconds
					strCurrentCmd = "Scriptpause 5000";
				}
				break;
			case RETERN_QUIT:
				{//////////////////////////////////////////////////////////////////////////////
				 //////////////////////////////////////////////////////////////////////////////
					return 1;
				}
				break;
			default:
				{//something wrong
					err("something wrong,can't go on with rest command");
					return -1;
				}
				break;
			}
		}
		catch ( ... ) 
		{
			printf("unknown exception\n");
		}

	}while (1);

	return 1;
}

void ScriptRequest::final(int retcode , bool bCancelled )
{

}

bool ScriptRequest::init()
{
	//copy all command from environment

//	std::string	strCreate = "CreatePlaylist ";
//	strCreate += _strDestIp;
//	strCreate += " ";
//	strCreate += _strPort;
//	_vecCommand.push_back(strCreate);

	_vecCommand.clear();
	for ( int i=0 ; i < (int)_env._listCmd.size() ; i++ )
	{
		_vecCommand.push_back( _env._listCmd[i] );
	}
	return true;
}
