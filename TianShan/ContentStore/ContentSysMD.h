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
// Ident : $Id: ContentSysMD.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : defines the system metadatas
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentSysMD.h $
// 
// 6     4/17/14 3:56p Hongquan.zhang
// 
// 5     9/18/12 2:07p Hui.shao
// enh#16995 - Serve the cached copy that is catching up the PWE copy on
// source storage
// 
// 4     3/12/12 3:20p Li.huang
// fix bug 15933
// 
// 3     4/11/11 4:43p Hui.shao
// added mono-provision test
// 
// 2     10-12-28 14:10 Hongquan.zhang
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 8     10-10-26 12:09 Hui.shao
// 
// 7     10-09-24 16:55 Li.huang
// 
// 6     10-09-24 16:15 Li.huang
// 
// 5     10-09-24 11:13 Li.huang
// 
// 4     09-07-28 10:59 Hongquan.zhang
// add mount path and index file name macro definition
// 
// 3     09-06-18 17:48 Hongquan.zhang
// 
// 3     09-06-05 11:39 Jie.zhang
// 
// 2     09-05-15 15:08 Jie.zhang
// 
// 1     08-12-25 18:37 Jie.zhang
// 
// ===========================================================================

#ifndef __ZQTianShan_ContentSysMD_H__
#define __ZQTianShan_ContentSysMD_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"

// the following are in the metaData map of the content object
#define METADATA_SourceUrl                SYS_PROP(SourceUrl)          ///< where the content was provisioned from
#define METADATA_ParentName               SYS_PROP(ParentName)         ///< asset if necessary 
#define METADATA_Comment                  SYS_PROP(Comment)            ///< string comment of the content
#define METADATA_FileSize                 SYS_PROP(FileSize)           ///< file size in bytes
#define METADATA_SupportFileSize          SYS_PROP(SupportFileSize)    ///< file size subtotal, in bytes, of supplemental files excluding the main file
#define METADATA_PixelHorizontal          SYS_PROP(PixelHorizontal)    ///< picture resoultion
#define METADATA_PixelVertical            SYS_PROP(PixelVertical)      ///< picture resoultion 
#define METADATA_BitRate                  SYS_PROP(BitRate)            ///< the encoded bitrate in bps
#define METADATA_PlayTime                 SYS_PROP(PlayTime)           ///< play time in msec
#define METADATA_FrameRate                SYS_PROP(FrameRate)          ///< framerate in fps
#define METADATA_SourceType               SYS_PROP(SourceType)         ///< source format type when the content is provisioned from
#define METADATA_LocalType                SYS_PROP(LocalType)          ///< local format type after save the content to local storage
#define METADATA_SubType                  SYS_PROP(SubType)            ///< sub type in addition to the local format type
#define METADATA_MD5CheckSum              SYS_PROP(MD5CheckSum)        ///< the MD5 checksum of the main file
#define METADATA_ScheduledProvisonStart   SYS_PROP(ScheduledProvisonStart)        ///< the MD5 checksum of the main file
#define METADATA_ScheduledProvisonEnd     SYS_PROP(ScheduledProvisonEnd)        ///< the MD5 checksum of the main file
#define METADATA_MaxProvisonBitRate       SYS_PROP(MaxProvisonBitRate)        ///< the MD5 checksum of the main file
#define METADATA_nPVRLeadCopy             SYS_PROP(nPVRLeadCopy)       ///< For nPVR copy only, to indicate the fullpath of the lead nPVR copy on the filesystem, no index file extname will be included
#define METADATA_StampLastUpdated         SYS_PROP(StampLastUpdated)	///< when the content has been newly updated
#define METADATA_MonoProvision            SYS_PROP(MonoProvision)	///< if is set to non-zero, the content would not become streamable during provisioning
#define METADATA_EstimatedStreamable      SYS_PROP(EstimatedStreamable)  ///< estimated playtime in msec to enter the state of ProvisioningStreamable

#define METADATA_SUBFILENAME	SYS_PROP(VstrmSubFileName)
#define METADATA_SUBFILECOUNT SYS_PROP(SubFileCount)
#define METADATA_FILENAME_INDEX           SYS_PROP(IndexFileName)
#define METADATA_MAIFILENAME()	SYS_PROP(MainFileName)
#define METADATA_MAIFILE_EXTNAME() SYS_PROP(MainFileExtentionName)
#define METADATA_FILENAME_MAIN            SYS_PROP(MainFileName)
#define METADATA_EXTNAME_MAIN    SYS_PROP(MainFileExtentionName)
#define METADATA_INDEXFILENAME() SYS_PROP(IndexFileName)
#define METADATA_MOUNTPATH() 	SYS_PROP(VolumeMountPath)

#define METADATA_IDX_GENERIC_INFO SYS_PROP(IdxGenericInfo)
#define METADATA_IDX_SUBFULE_INFO SYS_PROP(IdxSubfileInfo)

#define METADATA_AugmentationPids         SYS_PROP(AugmentationPids)    ///<   
#define METADATA_PreEncryption            SYS_PROP(PreEncryption)      ///<  
#define METADATA_OriginalBitRate          SYS_PROP(OriginalBitRate)       ///<
#define METADATA_AugmentedBitRate         SYS_PROP(AugmentedBitRate)	///<

#define METADATA_STAMPFilledBySS		  SYS_PROP(StampLastFilledBySS)
#define METADATA_ContentInPWE			  SYS_PROP(ContentInPwe)

#endif // __ZQTianShan_ContentSysMD_H__
