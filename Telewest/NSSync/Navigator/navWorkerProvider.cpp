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
// Name  : navWorkerProvider.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : implementation of the navWorkerProvider class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navWorkerProvider.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 7     08-09-19 10:57 Li.huang
// 
// 6     08-09-19 10:36 Ken.qian
// add logic that if new NAV building, no REBUILD for asset/bundle
// metadata update
// 
// 5     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 4     5/25/06 11:32p Bernie.zhao
// changed work type of PM schedule delete from 'sample update' to
// 'remove'
// 
// 3     06-01-12 21:11 Bernie.zhao
// added logic to support PM
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================

#include "stdafx.h"
#include "navWorkerProvider.h"
#include "nsBuilder.h"
#include ".\navigationservice.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

navWorkerProvider::navWorkerProvider(nsBuilder* theBuilder)
{
	_pTheBuilder = theBuilder;

	_connectStr = _pTheBuilder->_connectStr;
}

navWorkerProvider::~navWorkerProvider()
{
	_pTheBuilder = NULL;
}

navWorker* navWorkerProvider::provide(WQ_Entry wqentry)
{
	navWorker* pRet=NULL;
	switch(navWorkerProvider::getCategory(wqentry.Source_type,
										  wqentry.Entry_type,
										  wqentry.Operation_type)) 
	{
	case WORKER_REBUILD:
		pRet = new navRebuilder(wqentry,_connectStr);
		glog(Log::L_DEBUG, L"Provided a worker for Rebuild");
		break;
	case WORKER_SAMPLEUPDATE:
		pRet = new navSplUpdater(wqentry,_connectStr);
		glog(Log::L_DEBUG, L"Provided a worker for Sample Space Update");
		break;
	case WORKER_REMOVE:
		pRet = new navRemover(wqentry,_connectStr);
		glog(Log::L_DEBUG, L"Provided a worker for Remove");
		break;
	case WORKER_RENAME:
		pRet = new navRenamer(wqentry,_connectStr);
		glog(Log::L_DEBUG, L"Provided a worker for Rename");
		break;
	case WORKER_NONE:
		glog(Log::L_DEBUG, L"No worker provided");
		break;
	}

	if(pRet)
		pRet->setTimeout(_pTheBuilder->getTimeout());

	return pRet;
}

int navWorkerProvider::getCategory(int source_type, int entry_type, int operation_type)
{
	int nRet=WORKER_NONE;

	if(source_type==0)
	{	// work queue is from DBSync
		switch(operation_type) {
		case 1:	// ADD
			nRet = WORKER_NONE;
			break;
		case 2: // DELETE
			nRet = WORKER_REMOVE;
			break;
		case 3: // LINK
			if(entry_type==1 || entry_type==11 ) { // only care asset or bundle link
				nRet = WORKER_SAMPLEUPDATE;
			}
			break;
		case 4: // UNLINK
			if(entry_type==1) { // only care asset unlink
				nRet = WORKER_SAMPLEUPDATE;
			}
			else if(entry_type==11) {	// bundle unlink=delete
				nRet = WORKER_REMOVE;
			}
			break;
		case 5: // RENAME
			nRet = WORKER_NONE;
			break;
		case 6: // UPDATE
			if(entry_type==1 || entry_type==11) {	// only care asset or bundle meta-data change
				// KenQ 2008-9-18 if new Nav building logic, there is no rebuild for metadata update
				if(SP_FOLDER_UPDATE_TYPE_TARGET_SYNC == NavigationService::m_folderUpdateSPType)
					nRet = WORKER_SAMPLEUPDATE;
				else
					nRet = WORKER_REBUILD;
			}
			break;
		case 99: // !!! SPECAIL operation, signal that should rebuild the whole NAV Tree
			nRet = WORKER_REBUILD;
			break;
		default:
			break;
		}
	}
	else if(source_type==1)
	{	// work queue is from Navigation
		switch(operation_type) {
		case 1:	// add or link
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 2: // update
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 3: // delete or unlink
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 4: // pre-search condition update(only for folder)
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 5: // target update(only for folder)
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 98: // !!! SPECIAL operation, signal that should rebuild the whole QA Tree
			nRet = WORKER_REBUILD;
			break;
		case 99: // !!! SPECAIL operation, signal that should rebuild the whole NAV Tree
			nRet = WORKER_REBUILD;
			break;
		default:
			break;
		}
	}
	else if(source_type==2)
	{	// work queue is from Program Manager
		switch(operation_type) {
		case 1:	// add schedule
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 2: // delete schedule
			nRet = WORKER_REMOVE;
			break;
		case 3: // update schedule
			nRet = WORKER_SAMPLEUPDATE;
			break;
		case 4: // update program meta-data
			nRet = WORKER_REBUILD;
			break;
		case 98: // !!! SPECIAL operation, signal that should rebuild the whole QA Tree
			nRet = WORKER_REBUILD;
			break;
		case 99: // !!! SPECAIL operation, signal that should rebuild the whole NAV Tree
			nRet = WORKER_REBUILD;
			break;
		default:
			break;
		}
	}

	return nRet;
}