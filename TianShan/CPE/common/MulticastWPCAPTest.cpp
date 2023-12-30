// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#include "CppUTest/TestHarness.h"
#include "windows.h"
#include "MulticastWPCAP.h"


#pragma comment(lib, "ws2_32.lib")

TEST_GROUP(MultiWPCAP)
{
	MulticastWPCAP* pWAPCAP;

	void setup()
	{
		pWAPCAP = new MulticastWPCAP();
	}
	void teardown()
	{
		delete pWAPCAP;
	}
};

TEST(MultiWPCAP, Test1)
{
	int nCaptureTimeInMs = 60000;

	bool bRet = pWAPCAP->init();

	DWORD dwStart = GetTickCount();
	int nTotal = 0;
	DWORD dwLast = dwStart;
	while(1)
	{
		const u_char* pIPPacket;
		int nLen = pWAPCAP->capture(pIPPacket);
		if(nLen > 0)
		{
			nTotal += nLen;
		}

		DWORD dwNow = GetTickCount();
		if (dwNow-dwStart>nCaptureTimeInMs)
			break;

		if (dwNow - dwLast > 2000)
		{
			printf("process %d MB, rate %d bps\n", int(nTotal/(1024*1024)), int(nTotal*8000.0/(dwNow-dwLast)));
			dwLast = dwNow;
			nTotal = 0;
		}
	}
	
	printf("call close winpcap\n");
	pWAPCAP->close();
	printf("winpcap closed\n");
}

