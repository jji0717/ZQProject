// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: VolumeInfoCache.h $
// Branch: $Name:  $
// Author: 
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------

#ifndef ZQTS_CS_VOLUMEINFOCACHE_H
#define ZQTS_CS_VOLUMEINFOCACHE_H


#include <string>
#include <map>
#include "Locks.h"


class VolumeInfoCache
{
public:	
	void update(const std::string& volume, unsigned int freeMB, unsigned int totalMB);
	bool get(const std::string& volume, unsigned int& freeMB, unsigned int& totalMB);

protected:
	struct data
	{
		unsigned int freeMB;
		unsigned int totalMB;
	};	

	typedef std::map<std::string, data> DataSet;

	DataSet		_datas;
	
	ZQ::common::Mutex	_lock;
};


#endif
