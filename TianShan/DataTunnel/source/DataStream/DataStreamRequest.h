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

// Branch: $Name:DataStreamRequest.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/DataStream/DataStreamRequest.h $
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
#ifndef __ZQDataStreamRequest_H__
#define __ZQDataStreamRequest_H__

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Ice/Identity.h>
#include <TsStreamer.h>
#include "DataStreamImpl.h"
namespace TianShanIce  {
namespace Streamer     {
namespace DataOnDemand {
     class DataStreamImpl;
	class PlayStreamRequest : public ZQ::common::ThreadRequest
	{
	public: 
		PlayStreamRequest(const TianShanIce::Streamer::AMD_Stream_playPtr &amdCB, DataStreamImpl& datastream);
		virtual ~PlayStreamRequest();
		
	protected: // 
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
	   TianShanIce::Streamer::AMD_Stream_playPtr _amdCB;
       DataStreamImpl& _datastream;
	}; // class PlayStreamRequest

} // namespace DataOnDemand
}
}

#endif // __ZQDataStreamRequest_H__



