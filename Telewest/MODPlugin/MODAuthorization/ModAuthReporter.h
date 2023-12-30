// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: ModAuthReporter.cpp,v 1.1 2004/12/13 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Implement the Log writting.
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/MODPlugin/MODAuthorization/ModAuthReporter.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     04-12-13 17:43 Ken.qian
// Revision 1.1  2004/12/13 Ken Qian
//   definition and implemention



#if !defined(AFX_MODAUTHREPORTER_H__D56B9463_D167_448A_9A2F_42877DC756AB__INCLUDED_)
#define AFX_MODAUTHREPORTER_H__D56B9463_D167_448A_9A2F_42877DC756AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Reporter.h"

class CModAuthReporter : public CReporter
{
public:
	CModAuthReporter();
	virtual ~CModAuthReporter();

public:
	/// report the log to file
	RPTSTATUS ReportLog(REPORTLEVEL dwLevel,WCHAR *pwszFormat, ...);

	/// report the log to event
	RPTSTATUS ReportEvent(REPORTLEVEL dwLevel, DWORD dwEventId, WCHAR *pwszArgTypes, ... );

	/// register the reporter
	RPTSTATUS Register(LPCTSTR lpszRegistrationName, LPCTSTR lpszSourceName=NULL);

   // Global function for retrieving THE Reporter registration id
   inline DWORD GetReporterRegId() { return (m_ReporterRegId); };

private:
	DWORD      m_ReporterRegId;
};

#endif // !defined(AFX_MODAUTHREPORTER_H__D56B9463_D167_448A_9A2F_42877DC756AB__INCLUDED_)
