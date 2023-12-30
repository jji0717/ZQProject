// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================


#include "Log.h"
#include "CaptureInterface.h"



namespace ZQTianShan 
{
namespace ContentProvision
{


MulticastCaptureInterface* MulticastCaptureInterface::_pInstance = NULL;


void MulticastCaptureInterface::addNIC( const std::string& strLocalIp, int nBandwidth /*= 1000000000*/ )
{
	NICInfo nic;
	nic.nBandwidth = nBandwidth;
	nic.strLocalIp = strLocalIp;

	_vNICInfo.push_back(nic);
}

void MulticastCaptureInterface::clearNIC()
{
	_vNICInfo.clear();
}

void MulticastCaptureInterface::setLog( ZQ::common::Log* pLog )
{
	_log = pLog;
}

std::string MulticastCaptureInterface::getLastError()
{
	return _strLastErr;
}

void MulticastCaptureInterface::setLastError( const std::string& strError )
{
	_strLastErr = strError;
}

void MulticastCaptureInterface::setInstance( MulticastCaptureInterface* pInterface )
{
	if (_pInstance)
	{
		destroyInstance();
	}

	_pInstance = pInterface;
}

MulticastCaptureInterface* MulticastCaptureInterface::instance()
{
	return _pInstance;
}

void MulticastCaptureInterface::destroyInstance()
{
	if (_pInstance)
	{
		_pInstance->close();
		delete _pInstance;
		_pInstance = NULL;
	}
}

MulticastCaptureInterface::MulticastCaptureInterface()
{
	_log = &glog;
}

MulticastCaptureInterface::~MulticastCaptureInterface()
{

}


}
}

