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
// Ident : $Id: NoiseSim.h,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : main header file for NoiseSim
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/NoiseSim.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     05-05-19 12:09 Lin.ouyang
// by: lorenzo(lin ouyang)
// add comment for doxgen
// 
// 2     05-05-17 16:15 Lin.ouyang
// share /common/getopt.h /common/getopt.cpp
// 
// 1     05-05-17 15:48 Lin.ouyang
// init version
// 
// Revision 1.1  2005-05-17 15:30:26  Ouyang
// initial created
// ===========================================================================
//
//////////////////////////////////////////////////////////////////////

// NoiseSim.h: Main header file for project NoiseSim.
//
//////////////////////////////////////////////////////////////////////


/// @file "NoiseSim.h
/// @brief the header file for NoiseSim project
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0

/// @mainpage
///
/// Generating noise to simulating a noisy network
///

#ifndef __NoiseSim_H_
#define __NoiseSim_H_

#include <iostream>
#include <fstream>
#include "Socket.h"
#include "Filter.h"
#include "Noiser.h"
#include "FileCfg.h"
#include "getopt.h"

using namespace std;

#endif // !defined(__NoiseSim_H_)