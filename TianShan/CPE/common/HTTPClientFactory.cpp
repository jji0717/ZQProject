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
#include "HTTPClientFactory.h"
#include "HttpClient.h"

using namespace ZQTianShan::ContentProvision;

namespace ZQTianShan 
{
	namespace ContentProvision
	{

		HTTPClientFactory::HTTPClientFactory( void )
		{

		}

		HTTPClientFactory::~HTTPClientFactory( void )
		{
			uninitialize();
		}

		ZQ::common::Log* HTTPClientFactory::getLog()
		{
			return _log;
		}

		void HTTPClientFactory::setLog( ZQ::common::Log* pLog )
		{
			_log = pLog;
		}

		C2HttpClient* HTTPClientFactory::create()
		{
//			return new C2HttpClient();
			return NULL;
		}

		bool HTTPClientFactory::initialize()
		{
			return true;
		}

		void HTTPClientFactory::uninitialize()
		{

		}
	}
}
