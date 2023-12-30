// SSAdminWrapper.h: interface for the SSAdminWrapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSADMINWRAPPER_H__28DD06D6_5DB3_415F_BD55_93A6B2F9E865__INCLUDED_)
#define AFX_SSADMINWRAPPER_H__28DD06D6_5DB3_415F_BD55_93A6B2F9E865__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <map>
#include <string>
#include <vector>
#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <locks.h>
#include <pollingtimer.h>
#include "SSAdmin.h"

class SSAdminWrapper : public SSAdmin  
{
public:
	SSAdminWrapper();
	virtual ~SSAdminWrapper();
public:
	FUNCDEF(DestroyAll)
	FUNCDEF(useCmdFile)
	FUNCDEF(parseIni)
private:
	DECLEAR_CMDROUTE()

};

class CommandHolderManager;
class CommandRunner;

class CommandHolder  : public SSAdmin
{
public:
	CommandHolder(CommandHolderManager& holderManager);
	~CommandHolder();
public:
	int				PushCommand(const std::string& strCommand);
	int				InsertCommand(const std::string& strCommand,int iIndex);
	void			RemoveCommand(int iIndex);
	void			DoCommand();
	void			OnTimer();
public:
	FUNCDEF(syspause)	//re define the syspause
protected:
	void			InternalDoCommand();
	
private:	
	std::vector<std::string>	_cmdList;
	ZQ::common::Mutex			_listLocker;
	CommandHolderManager&		_holderManager;
	DECLEAR_CMDROUTE()
};

class CommandHolderManager : public ZQ::common::NativeThread , public CCmdParser
{
public:
	CommandHolderManager();
	~CommandHolderManager();
	int		run(void);
	void	watchMe(CommandHolder* pHolder,int expiration);
private:
	typedef std::map<CommandHolder*,DWORD>	LIST;
	LIST						_holderList;
	ZQ::common::Mutex			_listLocker;
	bool						_bQuit;
	HANDLE						_hEvent;
	DWORD						_nextWakeup;
};
class CommandRunner : public ZQ::common::ThreadRequest
{
public:
	CommandRunner(CommandHolder& holder,ZQ::common::NativeThreadPool& pool);
	~CommandRunner();
public:
	int		run();
	void	final(int retcode /* =0 */, bool bCancelled /* =false */);
protected:
private:
	CommandHolder&	_holder;
};

#endif // !defined(AFX_SSADMINWRAPPER_H__28DD06D6_5DB3_415F_BD55_93A6B2F9E865__INCLUDED_)
