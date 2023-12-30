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


#include "HTTPPropSource.h"
#include "Log.h"


#define MOLOG				(*_pLog)
#define HTTPPropSrc	        "HTTPPropSrc"


using namespace ZQ::common;



namespace ZQTianShan {
	namespace ContentProvision {


HTTPPropSource::HTTPPropSource( HTTPClientFactory* _pHttpClientFactory, FileIoFactory* pFileIoFactory ):HTTPPropagation(_pHttpClientFactory, pFileIoFactory)
{
	_benbleResume = false;
}

void HTTPPropSource::OnProgress( int64 nProcessBytes )
{
	GetGraph()->OnProgress(nProcessBytes);
}

bool HTTPPropSource::Init()
{
	bool result = true;
	//call prepare
	result = HTTPPropagation::prepare();
	if (!result)
	{
		std::string err;
		int errcode;
		HTTPPropagation::getLastError(err,errcode);
		SetLastError(err,errcode);
	}
    GetGraph()->setTotalBytes(_totalBytes);
	return result;
}

void HTTPPropSource::Stop()
{
	HTTPPropagation::stop();
}

bool HTTPPropSource::Run()
{
	bool result = true;
	// call propagate
	MediaInfo mInfo;
	HTTPPropagation::getMediaInfo(mInfo);
	GetGraph()->OnMediaInfoParsed(mInfo);

	if (_benbleResume)
	{
		result = HTTPPropagation::resumePropagate();
	}
	else
	{
		result =  HTTPPropagation::propagate();
	}
	if (!result)
	{
		std::string err;
		int errcode;
		HTTPPropagation::getLastError(err,errcode);
		SetLastError(err,errcode);
	}

	GetGraph()->setTotalBytes(_totalBytes);
	return result;
}

void HTTPPropSource::Close()
{
	HTTPPropagation::close();
}

void HTTPPropSource::endOfStream()
{
}

const char* HTTPPropSource::GetName()
{
	return SOURCE_TYPE_HTTPPropagation;
}
bool HTTPPropSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool HTTPPropSource::seek(int64 offset, int pos)
{
	return false;
}
	
}}

