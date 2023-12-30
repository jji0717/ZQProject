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

// Branch: $Name:DataStreamRequest.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/DataStream/DataStreamRequest.cpp $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-08-31 15:25 Li.huang
// 
// 1     09-08-06 15:11 Li.huang
// ===========================================================================
#include "DataStreamRequest.h"

extern ZQ::common::NativeThreadPool* pThreadPool;

#define PlayStreamRMacro    "PlayStreamRequest"
namespace TianShanIce  {
	namespace Streamer     {
		namespace DataOnDemand {

	PlayStreamRequest::PlayStreamRequest(const TianShanIce::Streamer::AMD_Stream_playPtr& amdCB,
		DataStreamImpl& datastream):_amdCB(amdCB), _datastream(datastream), ZQ::common::ThreadRequest(*pThreadPool)               
	{

	}

	PlayStreamRequest::~PlayStreamRequest()
	{
	}

	bool PlayStreamRequest::init()
	{
		return true;
	}

	int PlayStreamRequest::run(void)
	{
		try
		{
			_datastream.play();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(PlayStreamRMacro, "play() caught %s: %s"), 
				ex.ice_name().c_str(), ex.message.c_str());
			_amdCB->ice_exception(::TianShanIce::ServerError("DataStream", 501, ex.message.c_str()));
		}
		catch (const Ice::Exception& ex)
		{
			std::string lastError;
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(PlayStreamRMacro, "play() caught %s"), ex.ice_name().c_str());
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "play() caught exception[%s]", ex.ice_name().c_str());
			lastError = buf;
			_amdCB->ice_exception(::TianShanIce::ServerError("DataStream", 502, lastError));
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(PlayStreamRMacro, "play() caught unexpect exception"));
			_amdCB->ice_exception(::TianShanIce::ServerError("DataStream", 503, "play() caught unexpect exception"));
		}

		return 0;
	}

	void PlayStreamRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
} // namespace DataOnDemand
}
}
