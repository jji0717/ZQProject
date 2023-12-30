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


#include "VolumeInfoCache.h"


void VolumeInfoCache::update(const std::string& volume, unsigned int freeMB, unsigned int totalMB)
{
	ZQ::common::Guard<ZQ::common::Mutex> op(_lock);

	DataSet::iterator it = _datas.find(volume);
	if (it!=_datas.end())
	{
		it->second.freeMB = freeMB;
		it->second.totalMB = totalMB;
	}
	else
	{
		data da;
		da.freeMB = freeMB;
		da.totalMB = totalMB;
		_datas[volume] = da;
	}
}

bool VolumeInfoCache::get(const std::string& volume, unsigned int& freeMB, unsigned int& totalMB)
{
	ZQ::common::Guard<ZQ::common::Mutex> op(_lock);

	DataSet::iterator it = _datas.find(volume);
	if (it!=_datas.end())
	{
		freeMB = it->second.freeMB;
		totalMB = it->second.totalMB;
		return true;
	}

	return false;
}

