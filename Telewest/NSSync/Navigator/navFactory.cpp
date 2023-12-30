#include "stdafx.h"
#include "navfactory.h"
#include "Log.h"
#include "ScLog.h"
#include "nsBuilder.h"
#include "navWorkerProvider.h"
#include "NavigationService.h."

#define		WTYPEID	( (_type==FactoryNAV_t)?L"  NAV":L"QANAV" )

navFactory::navFactory(nsBuilder& theBuilder, FactoryType type /* = FactoryNAV_t */)
:_theBuilder(theBuilder), _type(type)
{
	_isWorking	= false;
	_hTerminate = ::CreateEvent(0, FALSE, FALSE, NULL);
	_hWakeup	= ::CreateEvent(0, FALSE, FALSE, NULL);
}

navFactory::~navFactory(void)
{
	::CloseHandle(_hTerminate);
	::CloseHandle(_hWakeup);
}

int navFactory::run()
{
	if(_type != FactoryNAV_t && _type != FactoryQA_t)
	{
		glog(Log::L_ERROR, L"Unknown factory type: %d, factory stopped.", _type);
		return 1;
	}
	glog(Log::L_NOTICE, L"[%s] (tid=%d) navFactory started.", WTYPEID, ::GetCurrentThreadId());
	
	// figure out the target memory work queue first
	WQEntryList*	pList		= &(_theBuilder._navWQList);
	int*	pLastWQEntry		= &(_theBuilder._navLastCmpltEntry);
	ZQ::common::Mutex* pLock	= &(_theBuilder._navWQLock);
	if(isQA())
	{
		pList			= &(_theBuilder._qaWQList);
		pLastWQEntry	= &(_theBuilder._qaLastCmpltEntry);
		pLock			= &(_theBuilder._qaWQLock);
	}
	
	// these are references, should point to the same address of the real variables in nsBuilder
	WQEntryList&	targetList		= *pList;
	int&			targetLast		= *pLastWQEntry;
	Mutex&			targetLock		= *pLock;
	
	while(1)
	{
		try
		{
		
			HANDLE handles[] = {_hTerminate, _hWakeup};
			DWORD status=::WaitForMultipleObjects(2, handles, FALSE, 5000);
			if(status==WAIT_OBJECT_0) 
			{
				// should terminate thread
				break;
			}
			if(status==WAIT_OBJECT_0+1)
			{
				// wake up, we have work to do
			}
			else if(status==WAIT_TIMEOUT) 
			{
				// no signal
				continue;
			}
			else 
			{
				// error happened
				int err = ::GetLastError();
				glog(Log::L_ERROR, L"[%s] ::WaitForSingleObject() Error - %d", WTYPEID, err);
				return 1;
			}

			//////////////////////////////////////////////////////////////////////////
			// check QA first, if not enabled, or generating, we do nothing
			bool QAFunction = false;
			
			if(_type == FactoryQA_t)
			{
				QAFunction = true;
				if(!NavigationService::m_bQANavigationEnabled || !_theBuilder.getQAFlag(_T(DB_SP_NAV_GETQAENABLEFLAG)))
				{
					QAFunction = false;
				}
				else if( 0!=_theBuilder.getQAFlag(_T(DB_SP_NAV_GETQAGENERATINGSTATUS)) )
				{
					QAFunction = false;
				}

				if(!QAFunction)
					continue;
			}

			//////////////////////////////////////////////////////////////////////////
			WQ_Entry		currentEntry;
			navWorker		*pCurrWorker = NULL;

			// set status to working
			_boolGuard	workingGd(_isWorking);

			// here we begin to do the memory list scan
			// 1.lock list mutex
			// 2.scan for the next waiting entry in the list
			{
				// lock memory lists
				ZQ::common::MutexGuard	gd(targetLock);

				int listsize = (int)targetList.size();

				if(listsize==0)	// list is empty
				{
					continue;
				}

				if( /* listsize>MAX_WQ_HISTORY || */ targetLast>=listsize) // bad index
				{
					glog(Log::L_ERROR, L"[%s] Invalid indices,  size =%d, last index =%d", WTYPEID, listsize, targetLast);
					continue;
				}

				

				// ok we are good now, get the entry
				int i=0;
				bool bFound = false;
				for(i=targetLast+1; i<listsize; i++)
				{
					if(targetList[i].Status == wq_skipped)	// a skipped entry, delete from DB
					{
						if(_type==FactoryNAV_t || (_type==FactoryQA_t && targetList[i].Operation_type==98) )
						{
							glog(Log::L_DEBUG, L"[%s] (%s) Entry ignored", WTYPEID, (LPCTSTR)(targetList[i].Queue_UID) );
							_theBuilder.deleteDBWQ(targetList[i].Queue_UID);
						}
						targetLast=i;
					}
					else if(targetList[i].Status == wq_waiting)	// found a waiting entry
					{
						bFound = true;
						break;
					}
					else	// impossible. These new entries should only be 'skipped' or 'waiting'
					{
						glog(Log::L_ERROR, L"[%s] Found invalid status when parsing pending entries,  size =%d, last index =%d, entry found index =%d, status = %s", WTYPEID, listsize, targetLast, i, WQStatusToString(targetList[i]));
					}
				}

				if(!bFound)
					continue;

				// we have new entry, get a worker for it and update time stamp
				currentEntry = targetList[i];
				pCurrWorker = _theBuilder._pTheWorkerProvider->provide(currentEntry);

				if(!pCurrWorker)
				{
					// don't care this work queue, mark it skipped
					targetList[i].Status		= wq_skipped;
					targetLast = i;
					if(_type==FactoryNAV_t || (_type==FactoryQA_t && targetList[i].Operation_type==98) )
					{
						glog(Log::L_DEBUG, L"[%s] (%s) Entry invalid", WTYPEID, (LPCTSTR)(targetList[i].Queue_UID) );
						_theBuilder.deleteDBWQ(targetList[i].Queue_UID);
					}
					continue;
				}

				pCurrWorker->setIsQA( (_type==FactoryQA_t) );
				
				targetList[i].Start_time	= CTime::GetCurrentTime();
				targetList[i].Status		= wq_building;
				currentEntry = targetList[i];
			}

			//////////////////////////////////////////////////////////////////////////
			// we have entry and worker, so fire it to work
			int nRet = NS_SUCCESS;
			bool bWQDeleted = false;
			try
			{
				// add by KenQ, for new logic of object building, the workqueue data need to be deleted before handling
				// but there is problem that restart nssync before the work() complete.
				if(SP_FOLDER_UPDATE_TYPE_TARGET_SYNC == NavigationService::m_folderUpdateSPType && 0 == currentEntry.Source_type && 6 == currentEntry.Operation_type)
				{
					glog(Log::L_DEBUG, L"[%s] (%s) Entry ended", WTYPEID, (LPCTSTR)(currentEntry.Queue_UID) );
					_theBuilder.deleteDBWQ(currentEntry.Queue_UID);
					bWQDeleted = true;
				}

				glog(Log::L_DEBUG, L"[%s] (%s) Entry started: SourceType=%d, LocalEntryUID=%s, ParentHUID=%s, EntryName=%s, EntryType=%d, OperationType=%d", 
						WTYPEID, (LPCTSTR)currentEntry.Queue_UID, currentEntry.Source_type, (LPCTSTR)currentEntry.local_entry_UID, 
						(LPCTSTR)currentEntry.Parent_HUID, (LPCTSTR)currentEntry.Entry_Name, currentEntry.Entry_type, currentEntry.Operation_type);

				DWORD tickStart = GetTickCount();
				nRet = pCurrWorker->work();

				glog(Log::L_DEBUG, L"[%s] (%s) Entry completed: Spent %d ms to accomplish the navigation build", 
					WTYPEID, (LPCTSTR)currentEntry.Queue_UID, GetTickCount()-tickStart);
			}
			catch (CDBException* pDBexcep) 
			{
				glog(Log::L_ERROR, L"[%s] (tid=%d) Database exception caught when sending worker to work: %s", WTYPEID, ::GetCurrentThreadId(), pDBexcep->m_strError);
				pDBexcep->Delete();
				nRet = NS_ERROR;
			}
			catch(...)
			{
				glog(Log::L_ERROR, L"[%s] (tid=%d) Unknown exception caught when sending worker to work.", WTYPEID, ::GetCurrentThreadId());
				nRet = NS_ERROR;
			}

			if(pCurrWorker)
			{
				pCurrWorker->free();
				pCurrWorker = NULL;
			}

			//////////////////////////////////////////////////////////////////////////
			// 1.update time stamp and status
			// 2.delete from DB
			{
				ZQ::common::MutexGuard	gd(targetLock);
				for(size_t i=targetLast+1; i<targetList.size(); i++)
				{
					if(targetList[i].Queue_UID == currentEntry.Queue_UID && targetList[i].Status == wq_building)
					{
						targetList[i].End_time = CTime::GetCurrentTime();
						targetList[i].Status = (nRet)? wq_failed : wq_completed;
						targetLast = i;
						if(!bWQDeleted && (_type==FactoryNAV_t || (_type==FactoryQA_t && targetList[i].Operation_type==98)) )
						{
							glog(Log::L_DEBUG, L"[%s] (%s) Entry ended", WTYPEID, (LPCTSTR)(targetList[i].Queue_UID) );
							_theBuilder.deleteDBWQ(targetList[i].Queue_UID);
						}
					}
				}
			}
			// add by Ken 2008-12-24, the way to make it handle next wq if there is. 
			signalWakeup();
		}// try
		catch (...) 
		{
			glog(Log::L_ERROR, L"[%s] (tid=%d) An unknown exception was caught.", WTYPEID, ::GetCurrentThreadId());
		}

	}// while
	

	glog(Log::L_NOTICE, L"[%s] (tid=%d) navFactory terminated.", WTYPEID, ::GetCurrentThreadId());
	return 0;
}