

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
// Ident : $Id: BarkerFiller.h
// Author: Kaliven Lee
// Desc  : Fill the play list in barker mode
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_new/FILLER/BarkerFiller.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 9     04-10-18 14:55 Kaliven.lee
// 
// 8     04-09-29 17:19 Kaliven.lee
// 
// 7     04-09-29 11:49 Kaliven.lee
// output point list
// 
// 6     04-09-29 9:05 Kaliven.lee
// compatible with aebuilder
// 
// 5     04-09-23 17:58 Kaliven.lee
// check for new comment
// 
// 4     04-09-23 17:14 Kaliven.lee
// fill specify time duration fillers
// 
// 3     04-09-20 14:51 Kaliven.lee
// add comment of doxygen
// 
// 2     04-09-16 17:02 Kaliven.lee
// add head message

// BarkerFiller.h: interface for the BarkerFiller class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BARKERFILLER_H__4022CFF4_4441_4C59_A7B5_8493E1E38DCB__INCLUDED_)
#define AFX_BARKERFILLER_H__4022CFF4_4441_4C59_A7B5_8493E1E38DCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "filler.h"

/// BarkerValidList 

typedef std::vector<bool>	BarkerValidList;

//////////////////////////////////////////////////////////////////////////
//Class Barker Filler
//
//////////////////////////////////////////////////////////////////////////
/// Filler the play list by the way of barker confuse
/// 
class BarkerFiller :public Filler 
{
public:
	/// Constructor


	BarkerFiller();
	virtual ~BarkerFiller();

	/// fill the play list using fillers
	/// @param	dwDuration	-IN		Time that will be filled with fillers
	/// @param	type		-IN		Compatible with filler reserved
	/// @param	filledList	-OUT		
	/// @return void
	void fill(PASSETLIST* filledList,DWORD dwDuration,FILLTYPE type);
private:
	/// search node by probability
	/// @param	probability	-the probability searched by 
	/// @return				-the index of the node which owns the probability
	int searchNodeByProbability(int probability);
	
};

#endif // !defined(AFX_BARKERFILLER_H__4022CFF4_4441_4C59_A7B5_8493E1E38DCB__INCLUDED_)
