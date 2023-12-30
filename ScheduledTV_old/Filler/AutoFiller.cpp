
// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: AutoFiller.CPP
// Author: Kaliven Lee
// Desc  : AutoFiller Class 
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/Filler/AutoFiller.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 6     04-09-29 17:19 Kaliven.lee
// 
// 5     04-09-29 11:49 Kaliven.lee
// output point list
// 
// 4     04-09-29 9:05 Kaliven.lee
// compatible with aebuilder
// 
// 3     04-09-23 17:14 Kaliven.lee
// fill specify time duration fillers
// 
// 2     04-09-16 17:13 Kaliven.lee

// AutoFiller.cpp: implementation of the AutoFiller class.
//
//////////////////////////////////////////////////////////////////////

#include "AutoFiller.h"
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AutoFiller::AutoFiller():
Filler()
{
	
}

AutoFiller::~AutoFiller()
{

}


void AutoFiller::fill(PASSETLIST* filledList,DWORD dwDuration,FILLTYPE type)
{
	DWORD fillerCount = m_Fillers->size();
	switch(type) {
	case FILLTYPE_RANDOM:
		{
			srand((unsigned)time( NULL ));
			DWORD dwFilledTime = 0;
			int i;

			while(dwFilledTime < dwDuration)
			{	
				i = rand()%fillerCount;
				dwFilledTime  += m_Fillers->at(i).dwPlayTime;
				filledList->push_back(&m_Fillers->at(i));				
			}
			break;
		}		
	case FILLTYPE_SERIAL:
		{
			DWORD dwFilledTime = 0;
			DWORD i = 0;
			while(dwFilledTime < dwDuration)
			{							
				dwFilledTime  += m_Fillers->at(i%fillerCount).dwPlayTime;
				filledList->push_back(&m_Fillers->at(i++%fillerCount));
			}
			break;
		}
	default:
		break;
	}		
}


