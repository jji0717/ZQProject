// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Name  : nsOptimizer.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsOptimizer.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 8     08-12-26 17:49 Build
// 
// 7     08-12-26 17:46 Build
// If there is "delete" wq, othe operation type to same entry should be
// ingored besides rebuild.
// 
// 6     08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 5     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 4     06-10-19 11:13 Ken.qian
// Change the Optimizer
// 
// 3     06-04-20 16:18 Bernie.zhao
// moved exception catch from nsBuild to every single 'exec'
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#include "stdafx.h"
#include "nsOptimizer.h"
#include "navWorkerProvider.h"
#include "nsBuilder.h"

#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

nsOptimizer::nsOptimizer(nsBuilder* pBuilder)
{
	_pTheBuilder = pBuilder;
}

nsOptimizer::~nsOptimizer()
{

}

int nsOptimizer::optimize()
{
	// lock mutex
	ZQ::common::MutexGuard  navGd(_pTheBuilder->_navWQLock);	
	ZQ::common::MutexGuard	qaGd(_pTheBuilder->_qaWQLock);
	
	std::vector<CString> removeList;
	bool bNeedRebuild = false;  // flag indicating need 'rebuild' or not
	bool bNeedRemove = false; // flag indicating need 'delete' or not

	//////////////////////////////////////////////////////////////////////////
	// standard NAV work queue
	//////////////////////////////////////////////////////////////////////////
	
	int i = 0;
	// scan from begining, to determine if we have a 'rebuild'
	for(i=_pTheBuilder->_navLastCmpltEntry+1; i<(int)_pTheBuilder->_navWQList.size(); i++)
	{
		if(_pTheBuilder->_navWQList[i].Status!=wq_waiting)
			continue;

		int workType = navWorkerProvider::getCategory(	_pTheBuilder->_navWQList[i].Source_type,
			_pTheBuilder->_navWQList[i].Entry_type,
			_pTheBuilder->_navWQList[i].Operation_type);
		if(workType == WORKER_REBUILD)
		{
			bNeedRebuild = true;
			break;
		}
	}

	bool lastRebuild = true;
	// scan from end, to determine each entry's status, original should all be 'wq_waiting'
	for(i=(int)_pTheBuilder->_navWQList.size()-1; i>=_pTheBuilder->_navLastCmpltEntry+1; i--) 
	{	
		int workType = navWorkerProvider::getCategory(	_pTheBuilder->_navWQList[i].Source_type,
														_pTheBuilder->_navWQList[i].Entry_type,
														_pTheBuilder->_navWQList[i].Operation_type);
		switch(workType) 
		{
		case WORKER_NONE:	
			_pTheBuilder->_navWQList[i].Status = wq_skipped;  // ignore this operation
			break;
		case WORKER_REBUILD:	
			if(bNeedRebuild && lastRebuild) 
			{
				_pTheBuilder->_navWQList[i].Status = wq_waiting; // last rebuild, do it, and ignore other rebuilds
				lastRebuild = false;
			}
			else if(bNeedRebuild && !lastRebuild)
			{
				_pTheBuilder->_navWQList[i].Status = wq_skipped; // not last rebuild, ignore
			}
			break;
		case WORKER_SAMPLEUPDATE:
			if(bNeedRebuild) 
			{
				_pTheBuilder->_navWQList[i].Status = wq_skipped; // need 'rebuild' later, so ignore
			}
			else if(bNeedRemove) 
			{
				if(_pTheBuilder->_navWQList[i].Source_type==0 
					// && _pTheBuilder->_navWQList[i].Operation_type==4  // remove this condition by Ken @ 2008-12-26 since if there is asset/bundle deletion, any operation type wq can be ignored
					&& (_pTheBuilder->_navWQList[i].Entry_type==1)||(_pTheBuilder->_navWQList[i].Entry_type==11) )
				{
					// is asset/bundle 'unlink' from DBSync, scan remove list to check existed 'delete' of this entry 
					for(size_t removeIndex=0; removeIndex<removeList.size(); removeIndex++) 
					{
						if( _pTheBuilder->_navWQList[i].local_entry_UID==removeList[removeIndex]) 
						{
							_pTheBuilder->_navWQList[i].Status = wq_skipped; // match, need 'delete' later, ignore 'unlink'
							break;
						}
					}
				}
			}
			break;
		case WORKER_REMOVE:
			// add by Ken on 2006-10-19
			// the latest rebuild operation has covered remove operation
			if(bNeedRebuild) 
			{
				_pTheBuilder->_navWQList[i].Status = wq_skipped; // need 'rebuild' later, so ignore
			}
			else
			{
				bNeedRemove = true;
				removeList.push_back(_pTheBuilder->_navWQList[i].local_entry_UID);
			}
			break;
		case WORKER_RENAME:
			_pTheBuilder->_navWQList[i].Status = wq_skipped; // currently ignore rename
			break;
		default:
			break;
		}

	}
	removeList.clear();

	//////////////////////////////////////////////////////////////////////////
	// QA work queue
	//////////////////////////////////////////////////////////////////////////
	bNeedRebuild = false;
	bNeedRemove = false;

	// scan from begining, to determine if we have a 'rebuild'
	for(i=_pTheBuilder->_qaLastCmpltEntry+1; i<(int)_pTheBuilder->_qaWQList.size(); i++)
	{
		if(_pTheBuilder->_qaWQList[i].Status!=wq_waiting)
			continue;

		int workType = navWorkerProvider::getCategory(	_pTheBuilder->_qaWQList[i].Source_type,
			_pTheBuilder->_qaWQList[i].Entry_type,
			_pTheBuilder->_qaWQList[i].Operation_type);
		if(workType == WORKER_REBUILD)
		{
			bNeedRebuild = true;
			break;
		}
	}

	lastRebuild = true;
	// scan from end, to determine each entry's status, original should all be 'wq_waiting'
	for(i=(int)_pTheBuilder->_qaWQList.size()-1; i>=_pTheBuilder->_qaLastCmpltEntry+1; i--) 
	{	
		int workType = navWorkerProvider::getCategory(	_pTheBuilder->_qaWQList[i].Source_type,
			_pTheBuilder->_qaWQList[i].Entry_type,
			_pTheBuilder->_qaWQList[i].Operation_type);
		switch(workType) 
		{
		case WORKER_NONE:	
			_pTheBuilder->_qaWQList[i].Status = wq_skipped;  // ignore this operation
			break;
		case WORKER_REBUILD:
			if(bNeedRebuild && lastRebuild) 
			{
				_pTheBuilder->_qaWQList[i].Status = wq_waiting; // last rebuild, do it, and ignore other rebuilds
				lastRebuild = false;
			}
			else if(bNeedRebuild && !lastRebuild)
			{
				_pTheBuilder->_qaWQList[i].Status = wq_skipped; // not last rebuild, ignore
			}
			break;
		case WORKER_SAMPLEUPDATE:
			if(bNeedRebuild) 
			{
				_pTheBuilder->_qaWQList[i].Status = wq_skipped; // need 'rebuild' later, so ignore
			}
			else if(bNeedRemove) 
			{
				if(_pTheBuilder->_qaWQList[i].Source_type==0 
					&& _pTheBuilder->_qaWQList[i].Operation_type==4
					&& (_pTheBuilder->_qaWQList[i].Entry_type==1)||(_pTheBuilder->_qaWQList[i].Entry_type==11) )
				{
					// is asset/bundle 'unlink' from DBSync, scan remove list to check existed 'delete' of this entry 
					for(size_t removeIndex=0; removeIndex<removeList.size(); removeIndex++) 
					{
						if( _pTheBuilder->_qaWQList[i].local_entry_UID==removeList[removeIndex]) 
						{
							_pTheBuilder->_qaWQList[i].Status = wq_skipped; // match, need 'delete' later, ignore 'unlink'
							break;
						}
					}
				}
			}
			break;
		case WORKER_REMOVE:
			// add by Ken on 2006-10-19
			// the latest rebuild operation has covered remove operation
			if(bNeedRebuild) 
			{
				_pTheBuilder->_qaWQList[i].Status = wq_skipped; // need 'rebuild' later, so ignore
			}
			else
			{
				bNeedRemove = true;
				removeList.push_back(_pTheBuilder->_qaWQList[i].local_entry_UID);
			}
			break;
		case WORKER_RENAME:
			_pTheBuilder->_qaWQList[i].Status = wq_skipped; // currently ignore rename
			break;
		default:
			break;
		}

	}
	removeList.clear();

	glog(Log::L_DEBUG, L"WorkQueue optimization done");
	return NS_SUCCESS;
}