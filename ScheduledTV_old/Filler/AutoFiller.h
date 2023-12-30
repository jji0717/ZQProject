
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
// Ident : $Id: AutoFiller.h
// Author: Kaliven Lee
// Desc  : AutoFiller Class 
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/Filler/AutoFiller.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
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
// 2     04-09-16 17:14 Kaliven.lee
// AutoFiller.h: interface for the AutoFiller class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOFILLER_H__93F6388F_21D2_4BF1_B944_E6D6419E917F__INCLUDED_)
#define AFX_AUTOFILLER_H__93F6388F_21D2_4BF1_B944_E6D6419E917F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Filler.h"

//////////////////////////////////////////////////////////////////////////
//AutoFiller
//
//////////////////////////////////////////////////////////////////////////


class AutoFiller: public Filler
{
public:
	/// Constructor
	AutoFiller();
	/// Deconstructor
	virtual ~AutoFiller(void);
	/// fill the play list with fillers

	/// @param type		- IN		describle in FILLTYPE.Now FILLTYPE_SERIAL or 
	///								FILLTYPE_RANDOM
	/// @param duration - IN		the time autofiller will fill in.(second)
	virtual void fill(PASSETLIST* filledList,DWORD duration,FILLTYPE type);
};

#endif // !defined(AFX_AUTOFILLER_H__93F6388F_21D2_4BF1_B944_E6D6419E917F__INCLUDED_)
