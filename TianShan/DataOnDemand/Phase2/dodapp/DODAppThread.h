#pragma once
#include <Ice/Object.h>
#include <Ice/LocalException.h>
#include <NativeThread.h>
#include <set>
#include <TianShanIce.h>
#include <tsSrm.h>
#include <dodapp.h>
#include <dodappex.h>
#include <global.h>
class DODAppThread: public ZQ::common::NativeThread {

public:
	DODAppThread();
	virtual ~DODAppThread();
	virtual void stop() = 0;
};
//////////////////////////////////////////////////////////////////////////
class SessionRenewThread: public DODAppThread {
public:
	SessionRenewThread(::TianShanIce::SRM::SessionPrx sessionprx,
		                 std::string destname)
	{
		_stopped = false;
		_sessionprx = sessionprx;
		_destname = destname;
	}

	virtual int run()
	{
		long _ltime = gDODAppServiceConfig.lRenewtime * 1000;
		long waittime = gDODAppServiceConfig.lRenewtime;

		waittime = waittime / 2;

		while(!_stopped)
		{
			// check it after 5 minute
			for (int i = 0; i < waittime; i++) 
			{
				if (_stopped)
					break;

				Sleep(1000);
			}
			if (_stopped)
				break;
			try
			{				
				_sessionprx->renew(_ltime);	
				
				glog(ZQ::common::Log::L_INFO, 
					"SessionRenewThread::run() [DestName: %s] Session renew  %d (ms)", 
					_destname.c_str(), _ltime);
			}
			catch (Ice::ObjectNotExistException &ex)
			{
				glog(ZQ::common::Log::L_INFO, 
					"SessionRenewThread::run() [DestName: %s] Session renew  Ice::ObjectNotExistException!"
					"errorcode = %s ", 
					_destname.c_str(), ex.ice_name().c_str());
				return -1;	
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_INFO, 
					"SessionRenewThread::run() [DestName: %s] Session renew  unknown exception! ", 
					_destname.c_str());
				return -1;	
			}
		}

		glog(ZQ::common::Log::L_INFO, 
			"SessionRenewThread::run() [DestName: %s] Session renew  Exit!", 
			_destname.c_str());
		return 0;
	}

	virtual void stop()
	{
		_stopped = true;
		waitHandle(INFINITE);
	}

protected:
     TianShanIce::SRM::SessionPrx _sessionprx;
	 bool					_stopped;
	 std::string			_destname;
};

class CreatDestionTrd : public DODAppThread
{
public:
   CreatDestionTrd()
   {
	   	InitializeCriticalSection(&m_CriticalSection);
	   _hWaitHandle = CreateEvent(NULL,true, false,NULL);
	   _bStop = false;
	   _createDestmap.clear();
   }
   ~CreatDestionTrd()
   {
	   	if (_hWaitHandle)
		CloseHandle(_hWaitHandle);

    	DeleteCriticalSection(&m_CriticalSection);
   }

   void stop()
   {
	   _bStop = true;
	   SetEvent(_hWaitHandle);
   }

   bool AddCreatMap(std::string destname, DataOnDemand::DestinationExPrx destprx)
   {
	   CreatDestmap::iterator itor;
     	EnterCriticalSection(&m_CriticalSection);

        itor = _createDestmap.find(destname);

		if(itor != _createDestmap.end())
		{
			glog( ZQ::common::Log::L_INFO,
				"[DestName: %s]CreatDestionTrd::AddCreatMap().Destination Exist in Map",
				destname.c_str());
			LeaveCriticalSection(&m_CriticalSection);
			return false;
		}
		_createDestmap[destname] = destprx;

		glog( ZQ::common::Log::L_INFO,
			"[DestName: %s]CreatDestionTrd::AddCreatMap().Add Destination in Map",
			 destname.c_str());

		if(_createDestmap.size() ==1)
		{
			SetEvent(_hWaitHandle);
		}
		LeaveCriticalSection(&m_CriticalSection);

	 return true;
   }

   int run()
   {
	   while(!_bStop)
	   {
		   WaitForSingleObject(_hWaitHandle,INFINITE);
		   for(int i = 0; i < 300; i++)
		   {
			   if(_bStop)
			   {
				   return -1;
			   }
			   sleep(100);
		   }
		   
		   if(_bStop)
		   {
			   return -1;
		   }
		   
		   CreatDestmap::iterator itor = _createDestmap.begin();
		   
		   while(itor != _createDestmap.end() && !_bStop)
		   {			      
			   EnterCriticalSection(&m_CriticalSection);
			   try
			   {
				   itor->second->activate();
				   glog( ZQ::common::Log::L_ERROR,
					   "[DestName: %s]CreatDestionTrd::run() activate destination success",
					   itor->first.c_str());
			   }
			   catch(const DataOnDemand::StreamerException& ex)
			   {
				   glog( ZQ::common::Log::L_ERROR,
					   "[DestName: %s]CreatDestionTrd::run()  errorMsg = %s",
					   itor->first.c_str(),ex.ice_name().c_str());
				   
				   itor = _createDestmap.begin();
				   sleep(1000);
				   LeaveCriticalSection(&m_CriticalSection);
				   continue;
			   }
			   catch(...)
			   {
				   glog( ZQ::common::Log::L_ERROR,
					   "[DestName: %s]CreatDestionTrd::run()  unknow exception",
					   itor->first.c_str());
				   itor = _createDestmap.begin();
				   sleep(1000);
				   LeaveCriticalSection(&m_CriticalSection);
			   }
			   _createDestmap.erase(itor);
			   itor = _createDestmap.begin();
			   LeaveCriticalSection(&m_CriticalSection);
			   sleep(1000);
		   }

		   ResetEvent(_hWaitHandle);
	   }
	   return 0;
   }
public:
   HANDLE _hWaitHandle;
   typedef std::map<std::string , DataOnDemand::DestinationExPrx>CreatDestmap;
   CreatDestmap _createDestmap;
	// process mutex flag for message_vector
	CRITICAL_SECTION	m_CriticalSection;	
   
   bool _bStop;
};
