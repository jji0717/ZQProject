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



#include "VstrmNPVRTarget.h"

#define NPVRFsTarget			"NPVRFsTarget"

using namespace ZQ::common;

#define MOLOG (*_pLog)
#define SMOLOG (*(pThis->_pLog))

namespace ZQTianShan 
{

namespace ContentProvision 
{

VstrmNPVRTarget::VstrmNPVRTarget(FileIoFactory* pFileIoFac)
:FilesetTarget(pFileIoFac)
{
	_pNotify = NULL;
}

int VstrmNPVRTarget::decideOpenFileFlag(bool bIndexFile)
{
	if (bIndexFile)
	{
		return FilesetTarget::decideOpenFileFlag(bIndexFile);
	}
	else
	{
		return FileIo::ATTRIB_NPVR;
	}
}


bool VstrmNPVRTarget::writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen)
{
	if (!FilesetTarget::writeFile(pFileIo, pBuf, dwBufLen))
		return false;

	if (_pNotify)
	{
		std::string strFileExt = pFileIo->getFileExtension();
		if (stricmp(strFileExt.c_str(), ".vvx"))	// not on index file
			_pNotify->notifyWrite(strFileExt, pBuf, (DWORD)dwBufLen);
	}

	return true;
}

void VstrmNPVRTarget::processOutPut()
{
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

		if(subFile.strFilename.empty())
			continue;

		if (i == 1) 
		{
			if (_pFileIoFac->getFileSize(subFile.strFilename.c_str()))
				continue;				

			MOLOG(Log::L_INFO, CLOGFMT(NPVRFsTarget, "[%s] size is 0, prepare to delete"), subFile.strFilename.c_str());
		}

		deleteFile(subFile.strFilename.c_str());
	}
}

}}