#include "StdAfx.h"
#include "FileParser.h"
#include <TimeUtil.h>
#include <boost/regex.hpp>
#include <algorithm>
const char* strRegex = "\"\\d+\",\"([0-9.]+)\",\"[^\"]+\",\"[^\"]+\",\"SMB\",\"(.+)Request, FID:([^,]+)(.*)\"";
const char* strCreate = "NT Create AndX ";
const char* strRead   = "Read AndX ";
const char* strWrite  = "Write AndX ";
const char* strClose  = "Close ";

#define FLOG if(_log)(*_log)

FileParser::FileParser(std::string& filepath, ZQ::common::FileLog* log):
_filepath(filepath), _log(log)
{
  
}
FileParser::~FileParser(void)
{

}
void addFileInfo(FileInfos& fileinfos, std::string fid, OPITEM& opitem)
{
	FileInfos::iterator itor;
	for(itor = fileinfos.begin(); itor != fileinfos.end(); itor++)
	{
		if(itor->fid == fid)
			break;
	}
	if(itor != fileinfos.end())
	{
		itor->opitems.push_back(opitem);
	}
	else
	{
		FileInfo fileinfo;
		fileinfo.fid = fid;
		fileinfo.opitems.push_back(opitem);
		fileinfos.push_back(fileinfo);
	}
}
char* FileOperator(int op)
{
	switch(op)
	{
	case 0: 
		return "Open File";
	case 1: 
		return "Read File";
	case 2: 
		return "Write File";
	case 3: 
		return "Close File";
	default: 
		return "Unknown Operator";
	}
}
bool FileParser::parserFile()
{
  int64 t1 = ZQ::common::now();
  FILE* fp = NULL;
  
  fp = fopen(_filepath.c_str(), "r");
  if(fp == NULL)
  {
	  printf("failed to open the file %s\n", _filepath.c_str());
	  FLOG(::ZQ::common::Log::L_ERROR,  CLOGFMT(FileParser,"failed to open the file %s"), _filepath.c_str());
	  return false;
  }
  printf("open the file %s successful\n", _filepath.c_str());
  FLOG(::ZQ::common::Log::L_INFO,  CLOGFMT(FileParser, "open the file %s successful"), _filepath.c_str());

  int  ncount = 0;
  char strBuf[4096*2] = "";
  std::string strFID = "";
  OPITEMS opitems;
  FileInfos fileinfos;
  while(!feof(fp))
  {
	  memset(strBuf, 0, sizeof(strBuf));
	  fgets(strBuf, sizeof(strBuf)-1, fp);
	  if(!strBuf[0])
		  continue;
     try
     {
		 OPITEM  opitem;
		 boost::regex SMBRegex(strRegex);
		 boost::cmatch matchResult; 
		 if(!boost::regex_search(strBuf, matchResult , SMBRegex))
		 {  
			 continue;
         }
         double timeStamp = atof(matchResult.str(1).c_str());
         opitem.stampMs = (__int64)(timeStamp * 1000);
         strFID = matchResult.str(3);
		 if(!stricmp(matchResult.str(2).c_str(), strCreate))
		 {
		    opitem.opID =  opCreate;
 			opitem.offset = 0;
 			opitem.opLen = 0;
            char strPath[256] = "";
			sscanf(matchResult.str(4).c_str(), ", Path: %s\"", &strPath);
 			opitem.filepath = strPath;
		 }
		 else if(!stricmp(matchResult.str(2).c_str(), strRead))
		 {
			 opitem.opID =  opRead;
			 __int64 offset = 0;
			 __int64 opByte = 0;
			 sscanf(matchResult.str(4).c_str(), ", %lld bytes at offset %lld\"", &opByte, &offset);
			 opitem.offset = offset;
			 opitem.opLen = opByte;
			 opitem.filepath = "";
		 }
		 else if(!stricmp(matchResult.str(2).c_str(), strWrite))
		 {
			 opitem.opID =  opWrite;

			 __int64 offset = 0;
			 __int64 opByte = 0;
			 sscanf(matchResult.str(4).c_str(), ", %lld bytes at offset %lld\"", &opByte, &offset);
			 opitem.offset = offset;
			 opitem.opLen = opByte;
			 opitem.filepath = "";
		 }
		 else if(!stricmp(matchResult.str(2).c_str(), strClose))
		 {
			 opitem.opID =  opClose;
			 opitem.offset = 0;
			 opitem.opLen = 0;
			 opitem.filepath = "";
		 }
		 else
			 continue;

		 FLOG(::ZQ::common::Log::L_DEBUG,  CLOGFMT(FileParser, "FID: %s, Operate %s, timestamp %lld, offset %lld, lenth %lld, filepath %s"), 
			 strFID.c_str(), FileOperator(opitem.opID), opitem.stampMs, opitem.offset, opitem.opLen, opitem.filepath.c_str());
		 addFileInfo(fileinfos, strFID, opitem);
     }
     catch (...)
     {
     	
     }	
  }

  for(FileInfos::iterator itor  = fileinfos.begin(); itor != fileinfos.end(); itor++)
  {
//	  printf("FID: %s, operate count %d\n", itor->fid.c_str(), itor->opitems.size());
	  FLOG(::ZQ::common::Log::L_INFO,  CLOGFMT(FileParser, "FID: %s, operate count %d"), itor->fid.c_str(), itor->opitems.size());
  }
  _fileitems.swap(fileinfos);
  printf("\nParseFile cost: %dmsec\n", (int)(ZQ::common::now() - t1));
  FLOG(::ZQ::common::Log::L_INFO,  CLOGFMT(FileParser, "ParseFile cost: %dmsec"), 
	  (int)(ZQ::common::now() - t1));

  return true;
}