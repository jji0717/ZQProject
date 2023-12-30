// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : define URL string
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/urlstr.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/urlstr.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 6     05-06-01 18:07 Bernie.zhao
// 
// 5     4/14/05 10:10a Hui.shao
// 
// 4     4/13/05 6:33p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:18p Hui.shao
// ============================================================================================

#ifndef _URLStr_H_
#define _URLStr_H_

#include "EntryDB.h"

#include <string>
#include <map>

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API URLStr;

class URLStr
{
  public:
	  URLStr(const char* urlstr=NULL, bool casesensitive=false)
	  {
		bCase=casesensitive;
		parse(urlstr);
	  }
	  
	  bool parse(const char* urlstr);
	  
	  const char* getProtocol();
	  const char* getHost();
	  const char* getPath();

	  void  setProtocol(const char* value);
	  void  setHost(const char* value);
	  void  setPath(const char* value);

	  const char* getVarname(int idx=0);
	  const char* getVar(const char* var);
	  void  setVar(const char* var, const char* value);
	
	  const char* generate();

	  void  clear();

  public:
		static const char* encode(const void* source, int len=-1);
		bool decode(const char* source, void* target, int maxlen=-1);

  private:

	  std::string mProtocol, mHost, mPath;
	  typedef std::map<std::string, std::string> urlvar_t;
	  urlvar_t mVars;
	  bool bCase;
	  std::string output_str;
}; 

ENTRYDB_NAMESPACE_END

#endif // _URLStr_H_

