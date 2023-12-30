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


#include "NPVRExtendFile.h"
#include "Log.h"
#include "FileIo.h"
#include "XMLPreferenceEx.h"

//////////////////////////////////////////////////////////////////////////
//  the xml format to storage 
//
//
//
//////////////////////////////////////////////////////////////////////////


#define MOLOG			(glog)
#define NPVRExt			"NPVRExt"



using namespace ZQ::common;


namespace ZQTianShan 
{
namespace ContentProvision
{



NPVRExtendFile::NPVRExtendFile(FileIoFactory* pFileIoFactory)
	:_pFileIoFactory(pFileIoFactory)
{
//	_log = &NullLogger;
}

NPVRExtendFile::~NPVRExtendFile()
{
}

void NPVRExtendFile::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);
}

void NPVRExtendFile::setFileName( const std::string& strFileName )
{
	_strFilename = strFileName;
}

std::string NPVRExtendFile::getFileName()
{
	return _strFilename;
}

void NPVRExtendFile::addSubFile( const std::string& strSubFile )
{
	SubFileList::iterator it = _subFileList.begin();
	for(;it!=_subFileList.end();it++)
	{
		if (*it == strSubFile)
			return;
	}

	_subFileList.push_back(strSubFile);
}

void NPVRExtendFile::setSubFiles( const SubFileList& strSubFiles )
{
	_subFileList = strSubFiles;
}

NPVRExtendFileI::SubFileList NPVRExtendFile::getSubFiles()
{
	return _subFileList;
}

void NPVRExtendFile::clearSubFiles()
{
	_subFileList.clear();
}

void NPVRExtendFile::addSubscriber( const std::string& strSubscriber )
{
	SubscriberList::iterator it = _subscriberList.begin();
	for(;it!=_subscriberList.end();it++)
	{
		if (*it == strSubscriber)
			return;
	}

	_subscriberList.push_back(strSubscriber);
}

void NPVRExtendFile::setSubscribers( const SubscriberList& strSubscriber )
{
	_subscriberList = strSubscriber;
}

NPVRExtendFileI::SubscriberList NPVRExtendFile::getSubscribers()
{
	return _subscriberList;
}

void NPVRExtendFile::clearSubscribers()
{
	_subscriberList.clear();
}

std::string NPVRExtendFile::getLastError()
{
	return _strError;
}

void NPVRExtendFile::setLastError( const std::string& strErr )
{
	_strError = strErr;
}


bool NPVRExtendFile::load()
{
	//char szXmlData[64*1024];
	//unsigned int	readByte = 0;
	//bool bParseOk = true;

	//std::auto_ptr<FileIo> pFileHandle(_pFileIoFactory->create());
	//if(!pFileHandle->openfile(_strFilename.c_str(),
	//	FileIo::ACCESS_READ,
	//	FileIo::ShareMode(FileIo::SHARE_WRITE|FileIo::SHARE_READ),
	//	FileIo::WAY_OPEN_EXISTING,
	//	FileIo::ATTRIB_CACHE))
	//{	
	//	MOLOG(Log::L_WARNING, CLOGFMT(NPVRExt, "failed to open %s for read"), _strFilename.c_str());
	//	return false;		
	//}	

	//ZQ::common::XMLPreferenceDocumentEx		xmlDoc;
	//do 
	//{
	//	if (!pFileHandle->readfile(szXmlData,sizeof(szXmlData),readByte))
	//	{
	//		return false;
	//	}

	//	try
	//	{
	//		if(!xmlDoc.read(szXmlData, readByte, readByte == sizeof(szXmlData) ? 0 : 1 ))
	//		{
	//			return false;
	//		}
	//	}
	//	catch(ZQ::common::XMLException& e)
	//	{
	//		return false;
	//	}

	//	if(readByte < 0 )
	//	{
	//		return false;
	//	}
	//} while ( readByte == sizeof(szXmlData) );

	//ZQ::common::XMLPreferenceEx* preRoot = (ZQ::common::XMLPreferenceEx*)xmlDoc.getRootPreference();
	//if(!preRoot )
	//{
	//	return false;
	//}

	//ZQ::common::XMLPreferenceEx* pNode = NULL;

	//char	szTemp[1024];
	//for( pNode = preRoot->firstChild("SubFile") ; preRoot->hasNextChild() ; pNode=preRoot->nextChild() )
	//{
	//	if( pNode )
	//	{
	//		//get data
	//		memset( szTemp , 0 , sizeof(szTemp));	
	//		pNode->get("name",szTemp,"",sizeof(szTemp));
	//		if(strlen(szTemp) > 0 )
	//		{
	//			_subFileList.push_back(std::string(szTemp));
	//		}
	//		memset( szTemp , 0 , sizeof(szTemp));
	//		pNode->get("requiredInLeadCopy",szTemp,"0",sizeof(szTemp));
	//		if (strlen(szTemp)>0)
	//		{
	//			_leadCopy.push_back(atoi(szTemp));
	//		}

	//	}
	//	else
	//	{
	//		// no data
	//		bParseOk = false;
	//		break;
	//	}
	//	pNode->free();
	//}

	//do
	//for( pNode = preRoot->firstChild("ReferenceCopy") ; preRoot->hasNextChild() ; pNode=preRoot->nextChild() )
	//{
	//	pNode = preRoot->firstChild("ReferenceCopy") ;
	//	if(pNode )
	//	{
	//		memset( szTemp , 0 , sizeof(szTemp));
	//		pNode->get("contentName",szTemp,"",sizeof(szTemp));
	//		if( strlen(szTemp) > 0 )
	//		{
	//			_subscriberList.push_back(std::string(szTemp));
	//		}
	//	}
	//	else
	//	{
	//		bParseOk = false;
	//	}
	//}

	//pFileHandle->closefile();
	//return bParseOk;
	return false;
}

bool NPVRExtendFile::save()
{
	std::string strcontent;
	char buf[1024];
	if (_strFilename.empty())
	{
		return false;
	}

	std::auto_ptr<FileIo> pFileHandle(_pFileIoFactory->create());
	if (!pFileHandle->openfile(_strFilename.c_str(),FileIo::ACCESS_WRITE,
	      FileIo::ShareMode(FileIo::SHARE_READ|FileIo::SHARE_WRITE),
		  FileIo::WAY_CREATE_ALWAYS,
		  FileIo::ATTRIB_NONE))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NPVRExt, "failed to open %s for write"), _strFilename.c_str());
		return false;
	}

	strcontent = "\<TianShanContentIndex\>\r\n";
	for (SubFileList::iterator iter = _subFileList.begin();iter != _subFileList.end();iter++)
	{
		int copyFlag=0;
		std::string::size_type pos = iter->find_last_of('.');
		if( std::string::npos != pos )
		{
			if (_stricmp( (*iter).c_str() + pos , ".vvx" )== 0)
				copyFlag = 1;
		}
		
		
		sprintf_s(buf,"\<SubFile name=\"%s\" requiredInLeadCopy=\"%d\" /\>\r\n",(*iter).c_str(),copyFlag);
		strcontent = strcontent + std::string(buf);
	}

	for (SubFileList::iterator iter = _subscriberList.begin(); iter != _subscriberList.end();iter++)
	{
		sprintf_s(buf,"\<ReferenceCopy  contentName=\"%s\" /\>\r\n",(*iter).c_str());
		strcontent = strcontent + std::string(buf);
	}

	strcontent += "\</TianShanContentIndex\> \r\n";

	unsigned int writenlen;
	if (!pFileHandle->writefile((char*)strcontent.c_str(), strcontent.size(),writenlen))
	{
		return false;
	}

	pFileHandle->closefile();
	return true;
}


}
}
