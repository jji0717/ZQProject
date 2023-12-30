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
// Ident : $Id: ContentUser.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : defines the user metadatas
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentUser.h $
// 
// 6     4/14/15 3:25p Li.huang
// 
// 5     4/13/15 9:48a Li.huang
// 
// 4     9/19/12 3:17p Hui.shao
// added METADATA_PersistentTill
// 
// 3     8/02/11 11:36a Li.huang
// fix bug 14340
// 
// 2     10-11-17 18:17 Hui.shao
// don't export system metadata in this file
// 
// 7     10-11-02 14:19 Hui.shao
// added Metadata for event mcast
// 
// 6     10-09-24 16:55 Li.huang
// 
// 5     10-09-24 16:15 Li.huang
// 
// 4     10-09-24 10:35 Li.huang
// add AugmentationPID
// 
// 3     10-06-23 16:47 Li.huang
// add metedata define
// 
// 2     10-03-03 16:09 Xia.chen
// add indextype
// 
// 1     08-12-25 18:01 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ContentUser_H__
#define __ZQTianShan_ContentUser_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"

#define METADATA_ProviderId                USER_PROP(ProviderId)          ///< the Provider ID, defined in CableLabs ADI, of the content</td></tr>
#define METADATA_ProviderAssetId           USER_PROP(ProviderAssetId)     ///< the Provider-Asset-ID, defined in CableLabs ADI, of the content</td></tr>
#define METADATA_nPVRCopy                  USER_PROP(nPVRCopy)            ///< non-zero if the content is a private nPVR copy</td></tr>
#define METADATA_SubscriberId              USER_PROP(SubscriberId)        ///< The owner's subscriberID if the Content is a private nPVR copy</td></tr>
#define METADATA_IndexType                 USER_PROP(IndexType)           ///< Index type use VVC</td></tr>>  
#define METADATA_ResponseURL               USER_PROP(ResponseURL)         ///  
#define METADATA_AugmentationPIDs          USER_PROP(AugmentationPids)    /// 
#define METADATA_Preencryption             USER_PROP(PreEncryption)
#define METADATA_WishedTrickSpeeds         USER_PROP(WishedTrickSpeeds)

#define METADATA_PersistentTill            USER_PROP(PersistentTill)      ///< value in format of ISO8601, different with "Expiration" is that PersistentTill doesn't require instantly destory if it has been met

#endif // __ZQTianShan_ContentUser_H__
