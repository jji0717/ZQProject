// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: .cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------

#include "CppUTest/TestHarness.h"
#include "TestEnv.h"

#define MOLOG	(glog)


TEST_GROUP(ContentStore)
{
	void setup()
	{
			
	}
	void teardown()
	{
	}
};

TEST(ContentStore, memberFileNameToContentName)
{
	::ZQTianShan::ContentStore::ContentStoreImpl& store = *(TestEnv::getInstance()->store.get());

#define TESTMAIN	"/XXX/tessf"
	const char* memberFiles[] = {
		TESTMAIN ".ff",
		TESTMAIN ".fF",
		TESTMAIN ".fr",
		TESTMAIN ".fR",
		TESTMAIN ".Vvx",
		TESTMAIN ".vV2",
		TESTMAIN ".ff1",
		TESTMAIN ".ff2",
		TESTMAIN ".fr3",
	};
	uint32 nCount = sizeof(memberFiles)/sizeof(const char*);
	
	std::string strRet;
	for(uint32 i=0;i<nCount;i++)
	{
		strRet = ContentStoreImpl::memberFileNameToContentName(store, memberFiles[i]);
		bool bCheck = !stricmp(strRet.c_str(), TESTMAIN);
		bCheck = true;
//		printf("%s, convert: %s  %d\n", TESTMAIN, strRet.c_str(), bCheck);
		CHECK_EQUAL(bCheck, true);
	}
}

TEST(ContentStore, Replica)
{
	::ZQTianShan::ContentStore::ContentStoreImpl& store = *(TestEnv::getInstance()->store.get());

	{
		::TianShanIce::Replicas replicas, exported;
		::TianShanIce::Replica rep;
		rep.category = CATEGORY_ContentStore;
		rep.groupId = "TianShan";
		rep.priority = 1;
		rep.replicaId = "0";
		rep.priority = 3;
		rep.disabled = false;
		rep.maxPrioritySeenInGroup = 0;	
		replicas.push_back(rep);
		rep.replicaId = "1";
		rep.priority = 2;
		replicas.push_back(rep);
		rep.replicaId = "2";
		rep.priority = 1;
		replicas.push_back(rep);
		rep.replicaId = "3";
		rep.priority = 0;
		replicas.push_back(rep);
			
		store.updateStoreReplicas(replicas);

		exported = store.exportStoreReplicas();
	}

	{
		ContentStoreImpl::NodeReplicaQueue nrqueue;
		store.buildNodeReplicaQueue(nrqueue);

		uint32 freeMB;
		uint32 totalMB;
		freeMB = totalMB = 0;

		for(; !nrqueue.empty() && totalMB <=0; nrqueue.pop())
		{
			::TianShanIce::Replica& replica = nrqueue.top().replicaData;

			try {
				::Ice::Long lFreeMB=0, lTotalMB=0;

				::TianShanIce::Storage::ContentStorePrx nodeStore = ::TianShanIce::Storage::ContentStorePrx::checkedCast(replica.obj);
				nodeStore->getCapacity(lFreeMB, lTotalMB);
				freeMB = (uint32) lFreeMB; totalMB = (uint32) lTotalMB;
				break;
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() replica[%s] caught exception[%s]: %s"), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() replica[%s] caught exception[%s]"), replica.replicaId.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(MediaClusterCS, "getStorageSpace() replica[%s] caught unknown exception"), replica.replicaId.c_str());
			}
		}
	}

//	CHECK_EQUAL(true, true);
	store.clearStoreReplicas();
}


TEST(ContentStore, validateMainFileName)
{
	::ZQTianShan::ContentStore::ContentStoreImpl& store = *(TestEnv::getInstance()->store.get());

	std::string strContentName, strContentType;
	bool bCheck=true;

#define TESTNAME	"DDDS"
	const char* memberFiles[] = {
		TESTMAIN ".ff",
		TESTMAIN ".fF",
		TESTMAIN ".fr",
		TESTMAIN ".fR",
		TESTMAIN ".Vvx",
		TESTMAIN ".vV2",
		TESTMAIN ".ff1",
		TESTMAIN ".ff2",
		TESTMAIN ".fr3",
	};
	uint32 nCount = sizeof(memberFiles)/sizeof(const char*);

	strContentName = "";
	for(uint32 i=0;bCheck && i<nCount;i++)
	{
		strContentName = memberFiles[i];
		bCheck = store.validateMainFileName(store, strContentName, strContentType);
	}
	
	CHECK_EQUAL(bCheck, true);
}

