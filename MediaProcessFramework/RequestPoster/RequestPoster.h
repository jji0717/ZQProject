
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
// Name  : RequestPoster.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-5-11
// Desc  : task request poster
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/RequestPoster/RequestPoster.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     05-05-30 2:47p Daniel.wang
// 
// 3     5/12/05 10:30p Daniel.wang
// 
// 2     5/12/05 1:54p Daniel.wang
// 
// 1     05-05-12 11:38 Daniel.wang
// ===========================================================================

#ifndef _ZQ_REQUESTPOSTER_H_
#define _ZQ_REQUESTPOSTER_H_

#include "comextra/ZqCommon.h"
#include "rpcwpvalue.h"

#define REQPOST_BEGIN namespace ZQ{namespace MPF{namespace REQPOST{
#define REQPOST_END }}}

#define POST_ERR_OK 0
#define POST_ERR_PARAMETER 1
#define POST_ERR_NETWORK 2
#define POST_ERR_USER 100

REQPOST_BEGIN

class DLL_PORT RequestPoster
{
public:
	///post task request 
	///@param strLocalUrl - local URL string
	///@param strRemoteUrl - remote URL string
	///@return - error code, 0 is ok
	virtual int post(rpc::RpcValue& result) = 0;
};

REQPOST_END

#endif//_ZQ_REQUESTPOSTER_H_
