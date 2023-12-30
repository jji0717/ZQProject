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


#ifndef ZQTS_CPE_FTP_PROPAGATION_SOURCE_H
#define ZQTS_CPE_FTP_PROPAGATION_SOURCE_H


#include "FTPPropagation.h"


#define	SOURCE_TYPE_FTPPropagation	"FTPPropagationSrc"


namespace ZQTianShan {
	namespace ContentProvision {



class FTPPropSource : public FTPPropagation, public BaseSource
{	
protected:
	friend class SourceFactory;
public:
	FTPPropSource(FTPClientFactory* pFTPClientFactory, FileIoFactory* pFileIoFactory);
	
public:
	virtual bool Init();
	
	virtual void Stop();
	virtual bool Run();
	virtual void Close();

	virtual void endOfStream();
	virtual const char* GetName();
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}
	void setEnableResumeDownload(bool enableResume = false){_benbleResume = enableResume;}
	bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	bool seek(int64 offset, int pos);
protected:
	virtual void OnProgress( int64 nProcessBytes );
private:
	bool   _benbleResume;
};


}}

#endif

