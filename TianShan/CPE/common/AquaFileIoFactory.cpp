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

#include "AquaFileIoFactory.h"
#include "AquaFileIo.h"
#include "CdmiFuseOps.h"
//#include "strHelper.h"
#include "Log.h"
#include <errno.h>
#ifdef ZQ_OS_MSWIN
#include "direct.h"
#include "io.h"
#else
extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>
}
#endif

#define MOLOG			         (*_log)
#define JSON_HAS(OBJ, CHILDNAME) (OBJ.isObject() && OBJ.isMember(CHILDNAME))

#ifdef ZQ_OS_LINUX
extern int64  getInt64FromMetadata( const Json::Value& metadata, const std::string& key );
extern time_t getTimeFromMetadata( const Json::Value& metadata, const std::string& key );
#endif

namespace ZQTianShan 
{
namespace ContentProvision
{

AquaFileIoFactory::AquaFileIoFactory()
{
	_pCdmiOps = NULL;
}

AquaFileIoFactory::~AquaFileIoFactory()
{
	uninitialize();
}

ZQ::common::Log* AquaFileIoFactory::getLog()
{
	return _log;
}

void AquaFileIoFactory::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

FileIo* AquaFileIoFactory::create()
{
	 AquaFileIo* pAquaFileIo =  new AquaFileIo(this);
	 return pAquaFileIo;
}

bool AquaFileIoFactory::deleteFile(const char* szFileName)
{
    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaFileIoFactory, "deleteFile() file[%s]"), szFileName);

    int64 lStart =  ZQ::common::now(); 
    std::string contentType = "";
    std::string uri = _pCdmiOps->pathToUri(szFileName);
    CdmiFuseOps::CdmiRetCode retCode = _pCdmiOps->nonCdmi_DeleteDataObject(uri);

    if(!CdmiRet_SUCC(retCode))
    {
        MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIoFactory, "deleteFile() failed to delete file[%s] with errorCode[%d]"), szFileName, retCode);
        return false;
    }
    MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIoFactory, "deleteFile() delete file[%s] successfully took %dms"),
        szFileName, (int)(ZQ::common::now() - lStart));
    return true;
}

bool AquaFileIoFactory::moveFile(const char* szFileName, const char* szNewFileName)
{
    if( !szNewFileName || !szNewFileName[0] )
        return false;

    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(moveFile, "move file from [%s] to [%s]"), szFileName, szNewFileName );

    // test if the source file is a dir or not
    bool bIsDir = false;
    std::string srcuri = _pCdmiOps->pathToUri(szFileName);

    Json::Value srcResult;
    CdmiFuseOps::CdmiRetCode cdmirc = _pCdmiOps->cdmi_ReadContainer(srcResult, srcuri); // +"?metadata");

    if (CdmiRet_SUCC(cdmirc))
    {
        MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MoveFile, "source[%s] is known as a folder: %s(%d)"), srcuri.c_str(), _pCdmiOps->cdmiRetStr(cdmirc), cdmirc);
        if (srcuri.length() >1 && LOGIC_FNSEPC!= srcuri[srcuri.length() -1])
            srcuri += LOGIC_FNSEPS;

        bIsDir = true;
    }

    Json::Value destResult;
    CdmiFuseOps::Properties metadata;
    //if (srcResult.isMember("metadata"))
    //	metadata = srcResult["metadata"];

    std::string desturi = _pCdmiOps->pathToUri(szNewFileName);
    if (bIsDir)
    {
        Json::Value exports;
        MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MoveFile, "calling cdmi_CreateContainer(%s) to create a new container"), desturi.c_str());
        cdmirc = _pCdmiOps->cdmi_CreateContainer(destResult, desturi, metadata, exports, "", "", "", srcuri);
        if (!CdmiRet_SUCC(cdmirc))
        {
            MOLOG(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(MoveFile, "cdmi_CreateContainer(%s) failed: %s(%d)"), desturi.c_str(), _pCdmiOps->cdmiRetStr(cdmirc), cdmirc);
            return false;
        }
    }
    else
    {
        CdmiFuseOps::StrList valuetransferencoding;
        MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MoveFile, "calling cdmi_CreateDataObject(%s) to create a new file"), desturi.c_str());
        cdmirc = _pCdmiOps->cdmi_CreateDataObject(destResult, desturi, "", metadata, "", valuetransferencoding, "", "", "", "", srcuri);

        if (!CdmiRet_SUCC(cdmirc))
        {
            MOLOG(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(MoveFile, "cdmi_CreateDataObject(%s) failed: %s(%d)"), desturi.c_str(), _pCdmiOps->cdmiRetStr(cdmirc), cdmirc);
            return false;
        }
    }

    MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MoveFile, "moved %s[%s] to [%s]: srcuri[%s]=>dest[%s]"), (bIsDir ? "dir" : "file"), szFileName, szNewFileName, srcuri.c_str(), desturi.c_str());
    return true;
}

int64 AquaFileIoFactory::getFileSize(const char* szFileName)
{
    FSUtils::fileInfo_t fileInfo;
    getFileStats((char*)szFileName, &fileInfo);

    return fileInfo.size;
}

bool AquaFileIoFactory::initialize()
{
	return true;
}

void AquaFileIoFactory::uninitialize()
{

}

void AquaFileIoFactory::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

void AquaFileIoFactory::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

std::string AquaFileIoFactory::getErrorText( unsigned int errCode )
{
	const int MAX_SYS_ERROR_TEXT = 256;
	char sErrorText[MAX_SYS_ERROR_TEXT+50]={0};

#ifdef ZQ_OS_MSWIN
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, 
		errCode,
		0,
		sErrorText, 
		MAX_SYS_ERROR_TEXT, 
		NULL);

	if (sErrorText[0])
	{
		char* pPtr = sErrorText + strlen(sErrorText) - 1;
		while(pPtr>=sErrorText && (*pPtr == 0x0d || *pPtr == 0x0a))
		{
			*pPtr = '\0';
			pPtr--;
		}
	}
	else
	{
		sprintf(sErrorText, "error code = [%d]", errCode);
	}
#else
	char* perr = strerror_r(errCode, sErrorText,sizeof(sErrorText));
	std::string strErr = perr;
	return strErr;
#endif
	return sErrorText;
}

void AquaFileIoFactory::setCdmiOps(CdmiFuseOps* pCdmiOps)
{
    _pCdmiOps = pCdmiOps;
}

CdmiFuseOps* AquaFileIoFactory::getCdmiOps()
{
    return _pCdmiOps;
}

std::string AquaFileIoFactory::getLastError()
{
	int nErrorNum = 0;

#ifdef ZQ_OS_MSWIN
	nErrorNum = GetLastError();
#else
	nErrorNum = errno;
#endif

	return getErrorText(nErrorNum);
}
std::string AquaFileIoFactory::getPathOfFile(const std::string& strFilename)
{
	std::string strPath;
	std::string::size_type pos = strFilename.find_last_of(FNSEPS);
	if (pos != std::string::npos)
		strPath.assign(strFilename, 0, pos);
	
	return strPath;
}

int AquaFileIoFactory::getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr)
{
    bool bIsACertainDir = false;
    std::string strFilePath = filepath;
    if (strFilePath.empty() || (FNSEPC == strFilePath[strFilePath.length() -1]))
        bIsACertainDir = true; // it is certainly a directory

    Json::Value result;
    std::string uri = _pCdmiOps->pathToUri(filepath); // + LOGIC_FNSEPS;
    CdmiFuseOps::CdmiRetCode cdmirc = CdmiFuseOps::cdmirc_BadRequest;

    if (bIsACertainDir)
    {
        MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(getFileStats, "path[%s] trying cdmi_ReadContainer(%s) to touch"), filepath, uri.c_str());
        cdmirc = _pCdmiOps->cdmi_ReadContainer(result, uri);

        if (CdmiRet_SUCC(cdmirc))
        {
            bIsACertainDir = true; // this is confirmed as an existing container on the server
            infoptr->mode |= _S_IFDIR;
            //fi.filestat.st_mode |= _S_IFDIR;
            //			FileInfoByHandle->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
            if (JSON_HAS(result, "metadata"))
            {
                Json::Value& metadata = result["metadata"];
                dataObjMetadataToFstat(metadata, infoptr);
                infoptr->mode |= _S_IFDIR;
            }

            return CdmiFuseOps::cdmirc_OK;
        } // if (CdmiRet_SUCC(cdmirc)
    }

    if (bIsACertainDir)
    {
        MOLOG(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(getFileStats, "path[%s] cdmi_ReadContainer(%s) as folder failed: %s(%d)"), filepath, uri.c_str(), CdmiFuseOps::cdmiRetStr(cdmirc), cdmirc);
        return cdmirc;
    }

    // step 1. query CDMI for the attr of the source file
    uri = _pCdmiOps->pathToUri(filepath);

    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(getFileStats, "path[%s] calling cdmi_ReadDataObject(%s) to read attributes"), filepath, uri.c_str());
    std::string tmp;
    cdmirc = _pCdmiOps->cdmi_ReadDataObject(result, uri +"?metadata", tmp);

    if (!CdmiRet_SUCC(cdmirc))
    {
        MOLOG(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(getFileStats, "path[%s] cdmi_ReadDataObject(%s) as file failed: %s(%d)"), filepath, uri.c_str(), CdmiFuseOps::cdmiRetStr(cdmirc), cdmirc);
        return cdmirc;
    }

    if (!JSON_HAS(result, "metadata"))
    {
        MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(getFileStats, "path[%s] cdmi_ReadDataObject(%s) no metadata received"), filepath, uri.c_str());
        return CdmiFuseOps::cdmirc_ServerError;
    }

    if (!dataObjMetadataToFstat(result["metadata"], infoptr))
    {
        MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(getFileStats, "path[%s] cdmi_ReadDataObject(%s) failed to convert fstat"), filepath, uri.c_str());
        return CdmiFuseOps::cdmirc_ServerError;
    }

    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaFileIoFactory, "getFileStats() successful, file[%s]"), filepath);
    
  return true;
}


bool AquaFileIoFactory::dataObjMetadataToFstat(const Json::Value& metadata, FSUtils::fileInfo_t *infoptr)
{
    std::string tmpstr;

    if (!metadata.isObject())
        return false;

#ifdef ZQ_OS_MSWIN
    //	COPY_METADATA_VAL(fileInfo.filestat.st_size, metadata, cdmi_size, Int);
    if (JSON_HAS(metadata, "cdmi_size"))
    {
        if (metadata["cdmi_size"].isIntegral())
            infoptr->size = metadata["cdmi_size"].asInt64();
        else
            infoptr->size = _atoi64(metadata["cdmi_size"].asString().c_str());
    }
    // COPY_METADATA_VAL(fileInfo.filestat.st_uid,  metadata, cdmi_owner, Int);

    COPY_METADATA_TIMET(infoptr->timecreate, metadata, cdmi_ctime);
    COPY_METADATA_TIMET(infoptr->timeaccess, metadata, cdmi_atime);
    COPY_METADATA_TIMET(infoptr->timemod, metadata, cdmi_mtime);
#elif defined ZQ_OS_LINUX
    infoptr->size	= (size_t)getInt64FromMetadata(metadata, "cdmi_size");
    infoptr->timecreate	= getTimeFromMetadata( metadata, "cdmi_ctime");
    infoptr->timeaccess	= getTimeFromMetadata( metadata, "cdmi_atime");
    infoptr->timemod	= getTimeFromMetadata( metadata, "cdmi_mtime");
#endif//ZQ_OS

    if (JSON_HAS(metadata, "cdmi_acl")) // about 16.1 Access Control
    {
        // "cdmi_acl" : [
        //      {
        //        "acetype" : "0x00",
        //        "identifier" : "EVERYONE@",
        //        "aceflags" : "0x00",
        //        "acemask" : "0x00020089"
        // }]

        const Json::Value& metadataACL = metadata["cdmi_acl"];
        for (Json::Value::const_iterator it = metadataACL.begin(); it != metadataACL.end(); it++)
        {
            if (!(*it).isObject())
                continue;

            // about aceflags
            uint32 aceflags = 0x00; // inherit=1, container_inherit=2, no_propagate_inherit=0x4, inherit_only=0x8

            if (JSON_HAS((*it), "aceflags"))
                aceflags = _pCdmiOps->readFlags((*it)["aceflags"], CdmiFuseOps::ACE_FlagFlags, aceflags);

            // about acemask
            uint32 acemask = 0x00;
            if (JSON_HAS((*it), "acemask"))
                acemask = _pCdmiOps->readFlags((*it)["acemask"], CdmiFuseOps::ACE_FlagMasks, acemask);

            // about acetype
            uint32 acetype = 0x00; // allow=0x00, deny=0x01, audit=0x02
            if (JSON_HAS((*it), "acetype"))
                acetype = _pCdmiOps->readFlags((*it)["acetype"], CdmiFuseOps::ACE_FlagTypes, acetype);

            // convert the (acetype, flag, mask) to fileInfo.filestat.st_mode flags per identifier
            // S_IRUSR(S_IREAD) 00400 
            // S_IWUSR(S_IWRITE)00200 
            // S_IXUSR(S_IEXEC) 00100 
            // S_IRGRP 00040 
            // S_IWGRP 00020 
            // S_IXGRP 00010  
            // S_IROTH 00004
            // S_IWOTH 00002 
            // S_IXOTH 00001 
            int leftMvBits = -1;
            COPY_METADATA_VAL(tmpstr, (*it), identifier, String);
            if (0 == tmpstr.compare("OWNER@"))
                leftMvBits = ST_MODE_BIT_OFFSET_USER;
            else if (0 == tmpstr.compare("GROUP@"))
                leftMvBits = ST_MODE_BIT_OFFSET_GROUP;
            else if (0 == tmpstr.compare("EVERYONE@"))
                leftMvBits = ST_MODE_BIT_OFFSET_OTHER;  // treated as others
            else if (0 == tmpstr.compare("ANONYMOUS@"))
                leftMvBits = ST_MODE_BIT_OFFSET_OTHER;  // treated as others
            else if (0 == tmpstr.compare("AUTHENTICATED@"))
                leftMvBits = ST_MODE_BIT_OFFSET_OTHER;  // treated as others
            else if (0 == tmpstr.compare("ADMINISTRATOR@"))
                leftMvBits = ST_MODE_BIT_OFFSET_GROUP;  // treated as group users
            else if (0 == tmpstr.compare("ADMINUSERS@"))
                leftMvBits = ST_MODE_BIT_OFFSET_GROUP;  // treated as group users

            uint32 perms =0;
            if (acemask & CdmiFuseOps::CDMI_ACE_READ_ALL) // read permission
                perms |= 0x4;
            if (acemask & (CdmiFuseOps::CDMI_ACE_RW_ALL & ~CdmiFuseOps::CDMI_ACE_READ_ALL)) // write permission
                perms |= 0x2;
            if (acemask & CdmiFuseOps::CDMI_ACE_EXECUTE) // execute permission
                perms |= 0x1;
            if (leftMvBits >=0 && leftMvBits < (sizeof(infoptr->mode)*8 -4))
            {
                perms <<= leftMvBits;
                if (CdmiFuseOps::CDMI_ACE_ACCESS_ALLOWED_TYPE == acetype)
                    infoptr->mode |= perms;
                else if (CdmiFuseOps::CDMI_ACE_ACCESS_DENIED_TYPE == acetype)
                    infoptr->mode &= ~perms;
            }
        }
    }

    if (JSON_HAS(metadata, "cdmi_owner")) // The name of the principal that has owner privileges for the object
    {
#pragma message ( __MSGLOC__ "??TODO: how to convert the string owner into numeric fileInfo.filestat.st_uid")
    }

    return true;
}
#ifdef ZQ_OS_MSWIN
void AquaFileIoFactory:: findClose(HANDLE hfile)
{

}
bool AquaFileIoFactory::findNextFile(HANDLE hfile,WIN32_FIND_DATAA& w)
{
	return false;
}
HANDLE AquaFileIoFactory::findFirstFile(char* name, WIN32_FIND_DATAA& w)
{
	return NULL;
}
#endif
}}

