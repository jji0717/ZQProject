// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : define a timer for polling
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/PollingTimer.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/PollingTimer.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 5     4/14/05 10:12a Hui.shao
// 
// 4     4/13/05 6:33p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:20p Hui.shao
// ============================================================================================

#ifndef __PollingTimer_h__
#define __PollingTimer_h__

#include "MPFCommon.h"

class PollingTimer
{
public:

	PollingTimer();

	void setTimer(timeout_t timeout = 0);

	void incTimer(timeout_t timeout);

	void endTimer(void);

	timeout_t getTimer(void);

	timeout_t getElapsed(void);

private:

#ifndef WIN32
	struct timeval timer;
#else
	DWORD timer;
#endif
	bool active;

};

#endif // ! __PollingTimer_h__



/*
-------------------------------------------------------------
 Revision history since 2003/03/20:
 $Log: /ZQProjs/MediaProcessFramework/EntryDB/PollingTimer.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 5     4/14/05 10:12a Hui.shao
// 
// 4     4/13/05 6:33p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:20p Hui.shao
// 
// 2     05-03-02 18:02 Feng.wang
// 
// 1     3/02/05 5:53p Hui.shao
// Entry Database Architecture
 Revision 1.3  2003/06/04 23:38:11  shao
 no message

 Revision 1.2  2003/06/03 22:49:34  shao
 no message

 Revision 1.1.1.1  2003/05/30 23:00:21  shao
 Enterprise Domain Objects by Shao Hui

 Revision 1.3  2003/03/20 04:18:51  shao
 Copy to Infraworks VSS

-------------------------------------------------------------
*/

