
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
// Ident : $Id: Filler.h
// Author: Kaliven Lee
// Desc  : Base Filler Class 
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/Filler/Filler.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 7     04-09-29 17:19 Kaliven.lee
// 
// 6     04-09-29 11:49 Kaliven.lee
// output point list
// 
// 5     04-09-29 9:05 Kaliven.lee
// compatible with aebuilder
// 
// 4     04-09-23 17:14 Kaliven.lee
// fill specify time duration fillers
// 
// 3     04-09-20 14:51 Kaliven.lee
// add comment of doxygen
// 
// 2     04-09-16 17:11 Kaliven.lee
// add head message

// Filler.h: interface for the Filler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILLER_H__D4AE4CC2_853A_4A9E_9569_661B839E3FBC__INCLUDED_)
#define AFX_FILLER_H__D4AE4CC2_853A_4A9E_9569_661B839E3FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FillerConf.h"



//////////////////////////////////////////////////////////////////////////
//Filler 
//
//////////////////////////////////////////////////////////////////////////



/// base filler class 

class Filler  
{
public:
	/// Constructor
	
	Filler();
	/// Deconstructor
	virtual ~Filler();
	/// set the filler list. 
	/// @param fillers		-IN		fillers list 	
	virtual void setFillers(ASSETS* fillers)
	{
		m_Fillers = fillers;
	}
	/// fill the play list by using fillerlist set by setfillers. Not implement here
	/// @param	playList	-IN/OUT	play list that will be filled in
	/// @param	type		-IN		the type of fill
	/// @param	duration	-IN		the time need to be filled(second)
	virtual void fill(PASSETLIST* filledList,DWORD duration,FILLTYPE type = FILLTYPE_SERIAL) {};
	
protected:
	/// filler list 
	ASSETS* m_Fillers;	
};

#endif // !defined(AFX_FILLER_H__D4AE4CC2_853A_4A9E_9569_661B839E3FBC__INCLUDED_)
