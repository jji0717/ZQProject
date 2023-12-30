#include <FileLog.h>
#include <NativeThread.h>
#include "ControlVLCService.h"
#include <Psapi.h>
#include "Telnet.h"

using namespace ZQTianShan::VSS::VLC;

ControlVLCService::ControlVLCService(::ZQ::common::FileLog& filelog, 
									 const std::string szServerName, 
									 const std::string strServerIP,
									 uint16 uServerPort,
									 const std::string strServerPwd,
									 int nInterval)
: _filelog(filelog),_szServiceName(szServerName), _strServerIP(strServerIP), _uServerPort(uServerPort),
_strServerPwd(strServerPwd), _strVLCCmd(""), _nInterval(nInterval), _pid(-1), _vlcHandle(NULL), _quitHandle(NULL)
{
	_nInterval *= 1000;
	char szPort[6];
	sprintf_s(szPort, sizeof(szPort), "%d", uServerPort);
	_strVLCCmd = "\"" + szServerName + "\"" + " -I telnet --telnet-host " + strServerIP + " --telnet-port ";
	_strVLCCmd += std::string(szPort) + " --telnet-password " + strServerPwd;// +  " --extraintf telnet";
}

ControlVLCService::~ControlVLCService()
{
	// stop run method
	if (_quitHandle)
	{
		SetEvent(_quitHandle);
		waitHandle(INFINITE); // wait run method is terminated
		CloseHandle(_quitHandle);
	}
}

bool ControlVLCService::init()
{
	if (_szServiceName.empty())
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, "init() : vlc server name is empty"));
		return false;
	}
	_quitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == _quitHandle)
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, 
			"init() : Fail to create event handle.Error code[%d]"), GetLastError());
		return false;
	}
	return true;
}

int ControlVLCService::run()
{
	// test start and stop
	//test();
	//return 0;
	if (!startVLC())
	{
		return 1;
	}
	DWORD dwResult = 0;
	while (true)
	{
		dwResult = WaitForSingleObject(_quitHandle, _nInterval);
		if (WAIT_OBJECT_0 == dwResult)
		{
			stopVLC();
			_filelog(::ZQ::common::Log::L_INFO, CLOGFMT(ControlVLCService, 
				"run() : Stream Service is terminated"));
			break;
		}
		if (!isVLCRunning())
		{
			_filelog(::ZQ::common::Log::L_WARNING, CLOGFMT(ControlVLCService, 
				"run() : VLC server[%d] is stopped"), _pid);
			_pid = -1;
			if (!startVLC())
			{
				return 1;
			}
			_filelog(::ZQ::common::Log::L_WARNING, CLOGFMT(ControlVLCService, 
				"run() : VLC server is restarted"));
		}
	}
	return 0;
}

bool ControlVLCService::startVLC()
{
	STARTUPINFO siStartupInfo;
	ZeroMemory( &siStartupInfo, sizeof(siStartupInfo) );
	siStartupInfo.cb = sizeof(siStartupInfo);

	PROCESS_INFORMATION piVlcProcInfo;
	ZeroMemory( &piVlcProcInfo, sizeof(piVlcProcInfo) );

	LPSTR pVlcCmd = const_cast<LPSTR>(_strVLCCmd.c_str());
	if (!CreateProcess(NULL, pVlcCmd, NULL, NULL, FALSE, 0, NULL, NULL, &siStartupInfo, &piVlcProcInfo))
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, 
			" startVLC() : Fail to create vlc process.Error code[%d]"), GetLastError());
		return false;
	}
	WaitForInputIdle(piVlcProcInfo.hProcess, 1000);
	_pid = piVlcProcInfo.dwProcessId;
	CloseHandle(piVlcProcInfo.hProcess);
	CloseHandle(piVlcProcInfo.hThread);
	if (0 == _pid)
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, 
			" startVLC() : Fail to get vlc process id .Error code[%d]"), GetLastError());
		return false;
	}
	_filelog(::ZQ::common::Log::L_INFO, CLOGFMT(ControlVLCService, 
		" startVLC() : success to create vlc process [%d]"), _pid);
	return true;
}

bool ControlVLCService::stopVLC()
{
	_vlcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _pid);
	if (NULL == _vlcHandle)
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, 
			" stopVLC() : Fail to get vlc process handle [%d].Error code[%d]"), _pid, GetLastError());
		return false;
	}
	if (!TerminateProcess(_vlcHandle, 4))
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, 
			" stopVLC() : Fail to stop vlc process[%d].Error code[%d]"), _pid, GetLastError());
		return false;
	}
	CloseHandle(_vlcHandle);
	_filelog(::ZQ::common::Log::L_INFO, CLOGFMT(ControlVLCService, 
		" stopVLC() : sucess to stop vlc process[%d]"), _pid);
	return true;
}

bool ControlVLCService::isVLCRunning()
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
	{
		_filelog(::ZQ::common::Log::L_ERROR, CLOGFMT(ControlVLCService, 
			" isVLCRunning() : Fail to list system process.Error code[%d]"), GetLastError());
		return false; 
	}

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);
	for ( i = 0; i < cProcesses; i++ )
	{
		if (aProcesses[i] == _pid)
		{
			return true;
		}
	}
	return false;
}

void ControlVLCService::test()
{
	ZQ::common::InetAddress addr;
	addr.setAddress(_strServerIP.c_str());
	ZQ::common::Telnet vlcTelnet(addr, _uServerPort);
	vlcTelnet.setPWD(_strServerPwd.c_str());
	std::string strMsgTemp;
	for (int i = 0; i < 10; i++)
	{
		if (startVLC())
		{
			WaitForSingleObject(_quitHandle, 30000);
			if (vlcTelnet.connectToServer(1))
			{
				if (vlcTelnet.sendCMD("logout", strMsgTemp, 1))
				{
					_filelog(::ZQ::common::Log::L_NOTICE, CLOGFMT(ControlVLCService, 
						"test() : success to log out"));
				}
				else
				{
					_filelog(::ZQ::common::Log::L_NOTICE, CLOGFMT(ControlVLCService, 
						"test() : fail to log out"));
				}
			}
			else
			{
				_filelog(::ZQ::common::Log::L_NOTICE, CLOGFMT(ControlVLCService, 
					"test() : fail to log on server"));
			}
			if (!stopVLC())
			{
				_filelog(::ZQ::common::Log::L_NOTICE, CLOGFMT(ControlVLCService, 
					"test() : sucess to stop vlc server"));
			}
			else
			{
				_filelog(::ZQ::common::Log::L_NOTICE, CLOGFMT(ControlVLCService, 
					"test() : fail to stop vlc server"));
			}
		}
		else
		{
			_filelog(::ZQ::common::Log::L_NOTICE, CLOGFMT(ControlVLCService, 
				"test() : fail to start vlc server"));
		}
	}
}