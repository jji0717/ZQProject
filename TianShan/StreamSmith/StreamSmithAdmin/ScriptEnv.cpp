// ScriptEnv.cpp: implementation of the ScriptEnv class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptRequest.h"
#include "ScriptEnv.h"

#include <ice/Ice.h>
#include <iceutil/IceUtil.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ScriptEnv::ScriptEnv( ZQ::common::NativeThreadPool& pool):_pool(pool),_schedule(pool)
{
	int i=0;
	_ic = Ice::initialize(i,NULL);
}

ScriptEnv::~ScriptEnv()
{
	if ( _requests.size() > 0 ) 
	{
		for ( int i=0 ; i<(int)_requests.size() ; i++ )
		{
			delete _requests[i];
		}
	}
	try
	{
		_ic->destroy();
	}
	catch (...) 
	{
	}
}

bool ScriptEnv::init( const char* pIniFile )
{
	InitInfo ini;
	ini.init(pIniFile);
	ini.setCurrent("Script");
	
	ini.getValue("playlistcount",_playlistCount);

	ini.getValue("ServerEndpoint",_strServerEndpoint);
	try
	{
		if (_strServerEndpoint.find(":")==std::string::npos) 
		{
			std::string strTemp = "StreamSmith:";
			strTemp = strTemp + _strServerEndpoint;
			_strServerEndpoint = strTemp;
		}
		_adminPrx = StreamSmithAdminPrx::checkedCast(_ic->stringToProxy(_strServerEndpoint));
		if (!_adminPrx) 
		{
			return false;
		}
	}
	catch (const Ice::Exception& ex) 
	{
		printf("caught ice exception :[%s]\n",ex.ice_name().c_str() );
		return false;
	}

	int iCmdCount = 0;
	ini.getValue("commandcount",iCmdCount);

	ini.getValue("setupInterval",_setupInterval);

	ini.getValue("destinationIp",_strDestIP);

	ini.getValue("destinationPort",_iDestPort);

	ini.getValue("portStep",_iPortStep);
	
	ini.setCurrent("command");
	std::string	strCMD;
	char	szBuf[256];
	_listCmd.clear();

	int i=0;	
	/*for ( i = 0; i< iCmdCount ; i++ ) */
	while(1)
	{
		sprintf(szBuf,"command%d",i);
		if(!ini.getValue(szBuf,strCMD))
			break;
		if (strCMD.empty() )
			break;
		_listCmd.push_back(strCMD);
		i++;
	}

	//initialize all ScriptRequest instance
	
	_schedule.start();
	for ( i=0; i < _playlistCount ;i++ ) 
	{
		ScriptRequest* pRequest =  new ScriptRequest(_pool,_ic,*this);
	
		sprintf(szBuf,"%d", _iPortStep*i +_iDestPort );
		pRequest->CreatePlaylist( _strDestIP.c_str() , szBuf );

		_requests.push_back(pRequest);
		Sleep(_setupInterval);
		pRequest->start();
	}
	
	return true;
}
