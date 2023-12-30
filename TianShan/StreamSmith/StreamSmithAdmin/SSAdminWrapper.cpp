// SSAdminWrapper.cpp: implementation of the SSAdminWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSAdminWrapper.h"
#include <iostream>
#include <fstream>
#include "InitInfo.h"
#include <conio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;
//ZQ::common::NativeThreadPool g_ThreadPool;
SSAdminWrapper::SSAdminWrapper()
{

}

SSAdminWrapper::~SSAdminWrapper()
{

}
BEGIN_CMDROUTE(SSAdminWrapper,SSAdmin)
	COMMAND(DestroyAll,DestroyAll)
	COMMAND(useCmdFile,useCmdFile)
	COMMAND(parseIni,parseIni)
END_CMDROUTE()
int SSAdminWrapper::useCmdFile(VARVEC& var)
{
	if(var.size()!=1)
	{
		log("usage:useCmdFile filepath");
		return OPERATION_INVALIDPARAMETER;
	}
	std::fstream is(var[0].c_str(),ios::in);
	char	szBuf[1024];
	while (1) 
	{
		is.getline(szBuf,1023,'\n');
		if(strlen(szBuf)<=1)
			break;
		if(RETERN_QUIT==ParseCommand(szBuf))
			break;
	}
	return OPERATION_SUCCESS;
}
int SSAdminWrapper::DestroyAll(VARVEC& var)
{
	ADMINCHECK();
	try
	{
		TianShanIce::Streamer::PlaylistIDs ids=m_AdminPrx->listPlaylists();
		string	strCmd;
		for(int i=0;i<(int)ids.size();i++)
		{
			strCmd="destroy "+ids[i];
			//destroy(ids[i]);
			ParseCommand(strCmd);
		}
	}
	catch (...) 
	{
		err("Unexpect error");
		return OPERATION_FAIL;
	}
	return OPERATION_SUCCESS;
}

int SSAdminWrapper::parseIni(VARVEC& var)
{
	if(!(var.size()==1||var.size()==2))
	{
		log("usage:parseini filepath [baseport]");
		return OPERATION_INVALIDPARAMETER;
	}
	InitInfo ini;
	if(!ini.init(var[0].c_str()))
	{
		err("Can't initialize with file %s",var[0].c_str());
		return OPERATION_FAIL;
	}
	ini.setCurrent("conf");
	int		iterCount=0;
	int		timewait=0;
	int		itemCount=0;
	int		PlaylistCount=0;
	int		IteratorWait=0;
	int		interval=0;
	string	targetDest;
	string	serverAddress;
	string	strBandwidth="5000000";
	string	strMacAddress;
	if(!ini.getValue("server",serverAddress))
	{
		err("no server address info");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("iterator",iterCount))
	{
		err("Can't get value iterator");
		return OPERATION_FAIL;
	}
	
	if(!ini.getValue("timewait",timewait))
	{
		err("Can't get value timewait");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("interval",interval))
	{
		err("can't get interval value");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("itemcount",itemCount))
	{
		err("Can't get value itemcount");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("playlistCount",PlaylistCount))
	{
		err("Can't get value playlistCount");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("IteratorWait",IteratorWait))
	{
		err("Can't get value IteratorWait");
		return OPERATION_FAIL;
	}
	if(!ini.getValue("target",targetDest))
	{
		err("can't get target");
		return OPERATION_FAIL;
	}
	ini.getValue("bandwidth",strBandwidth);
	ini.getValue("macaddress",strMacAddress);
	ini.setCurrent("item");
	char	szBuf[256];
	std::vector<std::string>	vecItem;
	string		tempItem;
	int i = 0;
	for ( i=0;i<itemCount;i++)
	{
		ZeroMemory(szBuf,sizeof(szBuf));
		sprintf(szBuf,"item%d",i);
		if(!ini.getValue(szBuf,tempItem))
		{
			log("Can't get value %s",szBuf);
			return OPERATION_FAIL;
		}
		vecItem.push_back(tempItem);
	}	
	string	strCMD;
	
	int		totalErr=0;
	strCMD="connect \""+serverAddress;
	strCMD+="\"";
	if(ParseCommand(strCMD)!=OPERATION_SUCCESS)
	{
		return OPERATION_FAIL;
	}
	int basePort;
	if(var.size()==2)
	{
		basePort=atoi(var[1].c_str());
	}
	else
	{
		basePort=1000;
	}
	typedef std::vector<std::string>	VECGUID;
	VECGUID	_lastVecGuid;
	for(int iTempIter=0;iTempIter<iterCount;iTempIter++)
	{
		if((kbhit() && (getch()=='q')))
				break;	
		int iDestPort=basePort;
		int	Err=0;
		_lastVecGuid.clear();
		for(int iTemppl=0;iTemppl<PlaylistCount;iTemppl++)
		{	
			DWORD dwTimeStart =GetTickCount();
			if((kbhit() && (getch()=='q')))
				break;	
			//createplaylist
			strCMD="createplaylist "+targetDest;
			sprintf(szBuf,"%d",iDestPort++);
			strCMD+=" ";
			strCMD+=szBuf;

			if(ParseCommand(strCMD)!=OPERATION_SUCCESS)
			{
				Err++;
				continue;
			}
			_lastVecGuid.push_back(m_lastEffectGuid);
			//pushitem
			for(int iTempItem=0;iTempItem<itemCount;iTempItem++)
			{
				if((kbhit() && (getch()=='q')))
				break;	

				strCMD="pushitem "+vecItem[iTempItem];
				strCMD+=" ";
				sprintf(szBuf,"%d",iTempItem);
				strCMD+=szBuf;
				strCMD+=" ";
				if(ParseCommand(strCMD)!=OPERATION_SUCCESS)
				{
					Err++;					
				}
			}
			if(!strMacAddress.empty())
			{
				strCMD="setdestMac ";
				strCMD+=strMacAddress;
				if(ParseCommand(strCMD)!=OPERATION_SUCCESS)
				{
					Err++;
				}
			}
			//play
			strCMD="play ";
			strCMD+=strBandwidth;
			if(ParseCommand(strCMD)!=OPERATION_SUCCESS)
			{
				Err++;
			}
			log("playlist %s at %s:%d was ok[%d] with timeCount=%d",
				m_lastEffectGuid.c_str(),targetDest.c_str() ,iDestPort,iTemppl,GetTickCount()-dwTimeStart);

			for(int iPause=0;iPause<interval;iPause++)
			{
				if((kbhit() && (getch()=='q')))
					break;	
				strCMD="syspause ";
				sprintf(szBuf,"%d",100);
				strCMD+=szBuf;
				ParseCommand(strCMD);
			}
		}
		log("%d playlist (%d/%d) and %d err",PlaylistCount,iTempIter+1,iterCount, Err);
		for(int iPause=0;iPause<timewait;iPause++)
		{
			if((kbhit() && (getch()=='q')))
				break;	
			strCMD="syspause ";
			sprintf(szBuf,"%d",1000);
			strCMD+=szBuf;
			ParseCommand(strCMD);
		}
		log("destroy playlist");
		int i = 0;
		for(i=0; i < (int)_lastVecGuid.size();i++)
		{
			strCMD="destroy ";
			strCMD+=_lastVecGuid[i];
			ParseCommand(strCMD);
			for(int iPause=0;iPause<interval;iPause++)
			{
				if((kbhit() && (getch()=='q')))
					break;	
				strCMD="syspause ";
				sprintf(szBuf,"%d",100);
				strCMD+=szBuf;
				ParseCommand(strCMD);
			}
		}
		log("destroy ok");
//		strCMD="destroyall";
//		ParseCommand(strCMD);
		totalErr+=Err;
		for(int iTerWait=0;iTerWait<IteratorWait;iTerWait++)
		{
			if((kbhit() && (getch()=='q')))
				break;	
			strCMD="syspause ";
			sprintf(szBuf,"%d",1000);
			strCMD+=szBuf;
			ParseCommand(strCMD);
		}
	}
	log("destroy playlist");
	for( i=0 ; i < (int)_lastVecGuid.size() ; i++)
	{
		strCMD="destroy ";
		strCMD+=_lastVecGuid[i];
		ParseCommand(strCMD);
	}
	log("mission complete total err=%d",totalErr);
	
	return OPERATION_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//CommandHolderManager
//CommandHolderManager::CommandHolderManager()
//{
//	_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
//	_bQuit = false;
//	start();
//}
//CommandHolderManager::~CommandHolderManager()
//{
//	_bQuit = true;
//	SetEvent(_hEvent);
//	waitHandle(1000);
//	CloseHandle(_hEvent);
//}
//void CommandHolderManager::watchMe(CommandHolder* pHolder,int expiration)
//{
//	if( ! pHolder )
//	{
//		err("enter watchMe with NULL pointer");
//		return;
//	}
//	DWORD target = GetTickCount() + expiration ;
//	_holderList[pHolder]=target;
//	if( target <= _nextWakeup )
//	{
//		SetEvent(_hEvent);
//	}
//}
//int CommandHolderManager::run()
//{
//	DWORD	dwWaitTime = 0xFFFF;
//	_nextWakeup =dwWaitTime + GetTickCount();
//	DWORD	dwNow ;
//	std::vector<CommandHolder*>	execList;
//	while ( ! _bQuit)
//	{	
//		execList.clear();
//		dwWaitTime = _nextWakeup - GetTickCount();
//		dwWaitTime = dwWaitTime > 2 ? dwWaitTime :2 ;
//
//		WaitForSingleObject(_hEvent , dwWaitTime);
//		if(_bQuit)
//			break;
//		{
//			dwNow = GetTickCount( );
//			LIST::const_iterator it =_holderList.begin();
//			for( ; it != _holderList.end() ; it ++ )
//			{
//				DWORD dwTargetTime = (it->second);
//				if( dwTargetTime <= dwNow )
//				{
//					execList.push_back(it->first);
//				}
//				else
//				{
//					_nextWakeup =_nextWakeup > dwTargetTime ? dwTargetTime : _nextWakeup;
//				}
//			}
//			for(std::vector<CommandHolder*>::const_iterator itExec = execList.begin() ; itExec != execList.end() ;itExec ++)
//			{
//				_holderList.erase(*itExec);
//			}
//		}
//		for(int i=0;i < (int)execList.size() ;i++)
//		{
//			CommandRunner* pRunner =  new CommandRunner(*execList[i],g_ThreadPool);
//			if(pRunner)
//				pRunner->start();
//		}
//	}
//	return 1;
//}
//
//
////////////////////////////////////////////////////////////////////////////
////CommandRunner
//CommandRunner::CommandRunner(CommandHolder& holder,ZQ::common::NativeThreadPool& pool)
//:_holder(holder),ZQ::common::ThreadRequest(pool)
//{
//}
//CommandRunner::~CommandRunner()
//{
//}
//void CommandRunner::final(int retcode , bool bCancelled )
//{
//	delete this;
//}
//int CommandRunner::run()
//{
//	try
//	{
//		_holder.OnTimer();
//	}
//	catch (...)
//	{		
//	}
//	return 1;
//}
////////////////////////////////////////////////////////////////////////////
////CommandHolder
//CommandHolder::CommandHolder(CommandHolderManager& holderManager)
//:_holderManager(holderManager)
//{
//}
//CommandHolder::~CommandHolder()
//{
//}
//BEGIN_CMDROUTE(CommandHolder,SSAdmin)
//	COMMAND(syspause,syspause)
//END_CMDROUTE()
//int CommandHolder::PushCommand(const std::string& strCommand)
//{
//	if(strCommand.empty())
//	{
//		err("NULL command passed in");
//		return -1;
//	}
//	ZQ::common::MutexGuard gd(_listLocker);
//	_cmdList.push_back(strCommand);
//	return _cmdList.size()-1;
//}
//int CommandHolder::InsertCommand(const std::string& strCommand,int iIndex)
//{
//	if(strCommand.empty())
//	{
//		err("InsertCommand() NULL command passed in");
//		return -1;
//	}
//	ZQ::common::MutexGuard gd(_listLocker);
//	if(iIndex<0 || iIndex>=(int)_cmdList.size())
//	{
//		err("InsertCommand() out of range");
//	}
//	std::vector<std::string>::iterator it=_cmdList.begin();	
//	it+=iIndex;
//	std::vector<std::string>::iterator itNew= _cmdList.insert(it,strCommand);
//	return (int)(itNew-_cmdList.begin());
//}
//void CommandHolder::RemoveCommand(int iIndex)
//{
//	ZQ::common::MutexGuard gd(_listLocker);
//	if(iIndex<0 || iIndex>=(int)_cmdList.size())
//	{
//		err("RemoveCommand() out of range");
//	}
//	std::vector<std::string>::iterator it=_cmdList.begin();	
//	it+=iIndex;
//	_cmdList.erase(it);
//}
//int CommandHolder::syspause(VARVEC& var)
//{
//	if(var.size() != 1)
//	{
//		err("Usage:syspause interval");
//		return OPERATION_INVALIDPARAMETER;
//	}
//	int dwTime = atoi( var[0].c_str() );
//	_holderManager.watchMe(this,dwTime);
//	return OPERATION_SUCCESS;
//}
//void CommandHolder::OnTimer()
//{
//}
//void CommandHolder::InternalDoCommand()
//{
//	//get current command
//	{
//		ZQ::common::MutexGuard gd(_listLocker);
//		
//	}
//}