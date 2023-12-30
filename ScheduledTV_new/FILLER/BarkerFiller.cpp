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
// Ident : $Id: BarkerFiller.cpp
// Author: Kaliven Lee
// Desc  : Fill the play list in barker mode
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_new/FILLER/BarkerFiller.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-08-30 18:29 Bernie.zhao
// 
// 1     05-08-30 18:28 Bernie.zhao
// 
// 14    05-02-05 21:43 Bernie.zhao
// fixed barker filler random problem(stop after 2 circles of random)
// 
// 13    04-10-18 14:17 Kaliven.lee
// and serial mode in barker system
// 
// 12    04-09-30 12:17 Bernie.zhao
// 
// 11    04-09-29 17:22 Kaliven.lee
// 
// 10    04-09-29 17:19 Kaliven.lee
// 
// 9     04-09-29 16:07 Kaliven.lee
// 
// 7     04-09-29 11:49 Kaliven.lee
// output point list
// 
// 6     04-09-29 9:05 Kaliven.lee
// compatible with aebuilder
// 
// 5     04-09-23 17:14 Kaliven.lee
// fill specify time duration fillers
// 
// 4     04-09-16 17:00 Kaliven.lee
// BarkerFiller.cpp: implementation of the BarkerFiller class.
//
//////////////////////////////////////////////////////////////////////

#include "BarkerFiller.h"
#include "Time.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BarkerFiller::BarkerFiller()

{

}

BarkerFiller::~BarkerFiller()
{
}

void BarkerFiller::fill(PASSETLIST *filledList,DWORD dwDuration,FILLTYPE type )
{
	//push point to filledList
	
	if(type == FILLTYPE_RANDOM)
	{
		DWORD dwPlayListCount = m_Fillers->size();
		
		DWORD sumweight = 0;
		DWORD i = 0;
		
		BarkerValidList listvalid;

		// compute the sum of all weights
		// and build listvalid
		for(i = 0; i < dwPlayListCount; i++)
		{
			sumweight += m_Fillers->at(i).dwWeight;
			for(int j =0; j < m_Fillers->at(i).dwWeight; j ++)
			{
				bool tmpBool = true;
				listvalid.push_back(tmpBool);
			}
		}
		
		int randnum =0;
		DWORD dwFilledTime = 0;
		int currindex =0;
		int preindex = 0;
		// set random seed
		srand((unsigned)time(NULL));


		int validCount = 0;
		while(dwFilledTime < dwDuration)
		{
			randnum = rand()%sumweight;	
			while(!listvalid.at(randnum))
			{
				randnum = (randnum+1)%sumweight;
			}
			
			currindex=searchNodeByProbability(randnum);
			preindex = currindex;

			listvalid[randnum] = false;
			validCount ++;
			if(validCount == sumweight)
			{
				for(int j =0; j < sumweight; j ++)
				{
					listvalid[j] = true;
				}
				validCount=0;
			}

					
			filledList->push_back(&m_Fillers->at(currindex));
			dwFilledTime += m_Fillers->at(currindex).dwPlayTime;
		}
	}
	else if(type == FILLTYPE_SERIAL)
	{
		DWORD dwFilledTime = 0;
		DWORD i = 0;
		DWORD fillerCount = m_Fillers->size();
		while(dwFilledTime < dwDuration)
		{							
			dwFilledTime  += m_Fillers->at(i%fillerCount).dwPlayTime;
			filledList->push_back(&m_Fillers->at(i++%fillerCount));
		}
	}	
}


int BarkerFiller::searchNodeByProbability(int rdnum)
{	// find the original item with probability of (weight/sumweight)
	int tmprd=rdnum;

	for(int i=0; i<m_Fillers->size(); i++) {
		tmprd-=m_Fillers->at(i).dwWeight;
		if(tmprd<=0)
			return i;
	}
	return -1;
}
