
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
// Dev  : Microsoft Developer Studio
// Name  : SRMDaemon.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-10
// Desc  : daemon
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/SRMDaemon.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 26    05-08-10 10:55 Jie.zhang
// 
// 25    05-06-24 5:11p Daniel.wang
// 
// 24    05-06-20 3:11p Daniel.wang
// 
// 23    05-06-14 6:59p Daniel.wang
// 
// 22    05-06-14 4:58p Daniel.wang
// ===========================================================================

#include "SRMDaemon.h"

#define DEFAULT_MN_PORT	8250

SRM_BEGIN

SRMDaemon::SRMDaemon(const char* strIp, int nPort)
:m_nPort(nPort),m_strIp(strIp), m_si(*this, strIp)
{
	MPFLog(MPFLogHandler::L_NOTICE, "[SRMDaemon::SRMDaemon]\tstart deamon in: IP-%s, port-%d", strIp, nPort);
}

SRMDaemon::SRMDaemon(const char* strUrl)
:m_si(*this, "0.0.0.0")
{	
	ZQ::MPF::utils::URLStr validUrl(strUrl);

	if (validUrl.getHost()==0)
	{
		MPFLog(MPFLogHandler::L_ERROR, "[SRMDaemon::SRMDaemon]\terror ip address in URL");
		m_strIp = "0.0.0.0";
	}
	else
	{
		m_strIp = validUrl.getHost();
	}

	if(validUrl.getPort()==0)
	{
		validUrl.setPort(DEFAULT_MN_PORT);
		MPFLog(MPFLogHandler::L_WARNING, "[SRMDaemon::SRMDaemon]\tno port info in url, use default port %d", validUrl.getPort());
	}
	else
	{
		m_nPort = validUrl.getPort();
	}

	m_si.setInterface(m_strIp.c_str());
	
	MPFLog(MPFLogHandler::L_NOTICE, "[SRMDaemon::SRMDaemon]\tstart deamon in: IP-%s, port-%d", m_strIp.c_str(), m_nPort);
}

SRMDaemon::~SRMDaemon()
{
	terminate();
	MPFLog(MPFLogHandler::L_NOTICE, "[SRMDaemon::~SRMDaemon]\tleave subscriber process cycle");
}

int SRMDaemon::run()
{
	MPFLog(MPFLogHandler::L_NOTICE, "[SRMDaemon::run]\trun deamon in: IP-%s, port-%d", m_strIp.c_str(), m_nPort);

	if (!bindAndListen(m_nPort, m_strIp.c_str(), 10))
	{
		MPFLog(MPFLogHandler::L_CRIT, "[SRMDaemon::run]\tcan not bind port");
		return -1;
	}

	MPFLog(MPFLogHandler::L_ERROR, "[SRMDaemon::run]\tbind ip and port");

	enableIntrospection(true);

	MPFLog(MPFLogHandler::L_ERROR, "[SRMDaemon::run]\tstart work");
	work(-1.0);
	
	MPFLog(MPFLogHandler::L_ERROR, "[SRMDaemon::run]\tend work");

	return 0;
}

SRM_END
