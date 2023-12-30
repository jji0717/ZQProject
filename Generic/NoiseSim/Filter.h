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
//
// Ident : $Id: Filter.h,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : define Filter class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/Filter.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 2     05-05-19 12:09 Lin.ouyang
// by: lorenzo(lin ouyang)
// add comment for doxgen
// 
// 1     05-05-17 15:48 Lin.ouyang
// init version
// 
// Revision 1.1  2005-05-17 15:30:26  Ouyang
// initial created
// ===========================================================================
//
//////////////////////////////////////////////////////////////////////


/// @file "Filter.h"
/// @brief the header file for Filter class
///
/// This class is not implemented now, 
/// in an other word, Filtor filtrates all messages in current version.
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0
// Filter.h: interface for the Filter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILTER_H__07DC8CEE_8550_47DB_99B4_EF986DB0B3CE__INCLUDED_)
#define AFX_FILTER_H__07DC8CEE_8550_47DB_99B4_EF986DB0B3CE__INCLUDED_

/// @class Filter "Filter.h"
/// @brief Filter class
///
/// To filtrate messages
class Filter  
{
public:
	/// Filtrate the message of szBuff
	/// @param[in,out] szBuff the message to be dealed with
	/// @param[in] iLen the length of szBuff
	void Filtrate(char *szBuff, int iLen);
	
	/// Constructor
	Filter();
	
	/// Destructor
	virtual ~Filter();
};

#endif // !defined(AFX_FILTER_H__07DC8CEE_8550_47DB_99B4_EF986DB0B3CE__INCLUDED_)
