// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : Define the Vstrm file IO Render
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/PropertyKeyDef.h 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/PropertyKeyDef.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     08-10-30 16:45 Ken.qian
// Move files from /ZQProjs/Generic/ContentProcess to local folder, since
// files at ContentProcess were never used by others components. And
// remove the pacing codes from NTFSIORender to indepent on Vstrm DLL
// 
// 2     08-03-12 10:41 Fei.huang
// 
// 2     08-03-11 14:23 Ken.qian
// 
// 1     07-08-09 17:10 Ken.qian
// 


#ifndef __ZQ_PropertyKeyDef_H__
#define __ZQ_PropertyKeyDef_H__

const std::string CNTPRY_FILESIZE              = "FileSize";      // type - _int64, in Bytes
const std::string CNTPRY_SUPPORT_FILESIZE      = "SupportFileSize";// type - _int64, in Bytes
const std::string CNTPRY_CREATION_TIME         = "CreationTime";  // type - std::std::string, in format of xxxx-xx-xx xx:xx:xx
const std::string CNTPRY_LAST_WRITE_TIME       = "LastWriteTIme"; // type - std::std::string, in format of xxxx-xx-xx xx:xx:xx
const std::string CNTPRY_MD5_CHECKSUM          = "MD5Checksum";   // type - std::std::string

const std::string CNTPRY_SEAC_SUBTYPE          = "SubType";       // type - std::std::string
const std::string CNTPRY_FILE_COUNT            = "FileCount";     // type - int
const std::string CNTPRY_FILE_NAME             = "FileName";      // type - std::std::string, 
                                                                  // The real property name is FileName<index> with number, the number is depends on FileCount, index is from 1
const std::string CNTPRY_ACCESS_USERNAME       = "UserName";      // type - std::std::string
const std::string CNTPRY_ACCESS_PASSWORD       = "Password";      // type - std::std::string

const std::string CNTPRY_BITRATE               = "Bitrate";       // type - std::std::string
const std::string CNTPRY_VIDEO_BITRATE         = "VideoBitrate";  // type - DWORD
const std::string CNTPRY_VIDEO_HORIZONTAL      = "VideoHorizontal";// type -DWORD
const std::string CNTPRY_VIDEO_VERTICAL        = "VideoVertical"; // type - DWORD
const std::string CNTPRY_VIDEO_FRAMERATE       = "FrameRate";     // type - float
const std::string CNTPRY_VIDEO_PLAYTIME        = "Playtime";      // type - __int64, in millisecond




#endif // __ZQ_PropertyKeyDef_H__
