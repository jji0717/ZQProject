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
// Ident : $Id: MPFCommon.h,v 1.8 2004/05/26 09:32:35 mwang Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : common definitions of Media Process Framework
//
// Revision History: 
// ---------------------------------------------------------------------------
// Revision 1.1  2004/04/20 09:35:07  shao
// initial created
//
// ===========================================================================

#ifndef __MPFCommon_H__
#define __MPFCommon_H__

#include "ZQ_common_conf.h"

//define this for DLL file port
#ifndef DLL_PORT
#	ifdef AFX_DLL
#		define DLL_PORT __declspec(dllexport)
#	elif _WINDLL
#		define DLL_PORT __declspec(dllexport)
#	elif _USRDLL
#		define DLL_PORT __declspec(dllexport)
#	else
#		define DLL_PORT __declspec(dllimport)
#	endif//_DLL
#endif DLL_PORT

//id string length
#define MPF_SESSID_LEN			20
#define MPF_RESOURCEID_LEN		20
#define MPF_RESOURCETYPE_LEN	256
#define MPF_NODEID_LEN			256
#define MPF_TASKID_LEN			20
#define MPF_FULL_TASKID_LEN		MPF_NODEID_LEN+MPF_TASKID_LEN+1
#define MPF_TREQID_LEN			20
#define MPF_MAX_LENGTH			MAX_PATH

#define MPF_TASKTYPE_LEN		256
#define MPF_VERSION_LEN			256
#define MPF_OS_LEN				256

#define MPF_UDSESS_ACTION_LEN	256

//naming define
//MPF namespace
#define MPF_NAMESPACE_BEGIN  namespace ZQ { namespace MPF {
#define MPF_NAMESPACE_END    } }
//WorkNode namespace 
#define MPF_WORKNODE_NAMESPACE_BEGIN  MPF_NAMESPACE_BEGIN namespace WorkNode{
#define MPF_WORKNODE_NAMESPACE_END    MPF_NAMESPACE_END }
//SystemInformation namespace 
#define MPF_SYSTEMINFO_NAMESPACE_BEGIN MPF_NAMESPACE_BEGIN namespace SysInfo{
#define MPF_SYSTEMINFO_NAMESPACE_END MPF_NAMESPACE_END }
//Utility namespace
#define MPF_UTILITY_NAMESPACE_BEGIN MPF_NAMESPACE_BEGIN namespace utils{
#define MPF_UTILITY_NAMESPACE_END MPF_NAMESPACE_END }

//naming usage, you must use one of follow to using the special name space  
#define USE_MPF_NAMESPACE using namespace ZQ::MPF;
#define USE_MPF_SYSTEMINFO_NAMESPACE	  using namespace ZQ::MPF::SysInfo;
#define USE_MPF_WORKNODE_NAMESPACE   using namespace ZQ::MPF::WorkNode;

//network send time out (second)
#define NET_SEND_TIME_OUT 5

//task limit number
#define MAX_TASK_COUNT 3000

//reserved prefix for MPF only
#define MPF_RESERVED_PREFIX "ZQ.MPF."

////////////////////////////////
// ordinal string definitions
////////////////////////////////
//method name
#define HEARTBEAT_METHOD		MPF_RESERVED_PREFIX "WorkNode.Heartbeat"
#define QUERY_METHOD			MPF_RESERVED_PREFIX "WorkNode.Query"
#define TASKREQUEST_METHOD		MPF_RESERVED_PREFIX "ManageNode.TaskRequest"
#define UPDATESESSION_METHOD	MPF_RESERVED_PREFIX "WorkNode.UpdateSession"
#define GETINFO_METHOD			MPF_RESERVED_PREFIX "GetInformation"
	
//common method result 
#define ERROR_CODE_KEY			MPF_RESERVED_PREFIX "ErrorCode"
#define COMMENT_KEY				MPF_RESERVED_PREFIX "Comment"
#define ACTION_ID_KEY			MPF_RESERVED_PREFIX "ActionID"

//update session type
#define TASK_INIT_ACTION		MPF_RESERVED_PREFIX "TaskInit"
#define TASK_PROGRESS_ACTION	MPF_RESERVED_PREFIX "TaskProgress"
#define TASK_FINAL_ACTION		MPF_RESERVED_PREFIX "TaskFinal"

// for TaskRequest
//task request type
#define REQUEST_SETUP			MPF_RESERVED_PREFIX "RequestSetup"
#define REQUEST_PLAY			MPF_RESERVED_PREFIX "RequestPlay"
#define REQUEST_USER			MPF_RESERVED_PREFIX "RequestUser"
#define REQUEST_STOP			MPF_RESERVED_PREFIX "RequestStop"
//task request parameter
#define REQUEST_ATTR_KEY		MPF_RESERVED_PREFIX "RequestAttr"
#define TASK_TYPE_KEY			MPF_RESERVED_PREFIX "TaskType"
#define MGM_SESSION_URL_KEY		MPF_RESERVED_PREFIX "MgmSessionURL"

//use defined task request
#define	USER_ACTION_ID_KEY		MPF_RESERVED_PREFIX "UserActionID"
#define	USER_ATTR_KEY			MPF_RESERVED_PREFIX "UserAttr"

// for Heartbeat
#define WORKNODE_ID_KEY			MPF_RESERVED_PREFIX "WorkNodeID"
#define TASK_TYPES_KEY			MPF_RESERVED_PREFIX "TaskTypes"
#define TYPE_NAME_KEY			MPF_RESERVED_PREFIX "TypeName"
#define RUN_INSTANCE_KEY		MPF_RESERVED_PREFIX "RunningInstanceCount"
#define AVBLE_INSTANCE_KEY		MPF_RESERVED_PREFIX "AvailableInstanceCount"
#define WORKNODE_IP_KEY			MPF_RESERVED_PREFIX "WorkNodeIp"
#define WORKNODE_PORT_KEY		MPF_RESERVED_PREFIX "WorkNodePort"
#define WORKNODE_INFO_KEY		MPF_RESERVED_PREFIX "WorkNodeInformation"
#define CPU_KEY					MPF_RESERVED_PREFIX "CPU"
#define TOTAL_MEMORY_KEY		MPF_RESERVED_PREFIX "TotalMemory"
#define CPU_USAGE_KEY			MPF_RESERVED_PREFIX "CPUUsage"
#define MEMORY_USAGE_KEY		MPF_RESERVED_PREFIX "MemoryUsage"
#define MAX_TRAFFIC_KEY			MPF_RESERVED_PREFIX "MaxTraffic"
#define CURRENT_TRAFFIC_KEY		MPF_RESERVED_PREFIX "CurrentTraffic"
#define BAND_WIDTH_KEY			MPF_RESERVED_PREFIX "BandWidth"
#define MPF_VERSION_KEY			MPF_RESERVED_PREFIX "MPFVersion"
#define OS_KEY					MPF_RESERVED_PREFIX "OS"
#define USER_HBINFO_KEY			MPF_RESERVED_PREFIX "UserHeartbeatInfo"
//heartbeat return
#define LEASE_TERM_KEY			MPF_RESERVED_PREFIX "LeaseTerm"

// for UpdateSession
#define ATTR_KEY				MPF_RESERVED_PREFIX "Attr"
#define EXP_ATTR_KEY			MPF_RESERVED_PREFIX "ExpAttr"
#define WORKNODE_ATTR_KEY		MPF_RESERVED_PREFIX "WorkNodeAttr"
//update session return
#define SESSION_ATTR_KEY		MPF_RESERVED_PREFIX "SessionAttr"
#define TASK_ID_KEY				MPF_RESERVED_PREFIX "TaskID"

//for OnGetProgress 
// for default progress attribute
#define	BEGIN_POS_KEY			MPF_RESERVED_PREFIX "BeginPos"
#define CURRENT_POS_KEY			MPF_RESERVED_PREFIX "CurrentPos"
#define END_POS_KEY				MPF_RESERVED_PREFIX "EndPos"

//
#define MAX_URL_LEN 512

//RPC error code number
#define RPC_ERROR_SUCCESS 0
#define RPC_ERROR_EXCEPTION 5
#define RPC_ERROR_COMMENT_SUCCESS "success"

#include <string>
#include <map>
#include <algorithm>
#include <windows.h>

MPF_UTILITY_NAMESPACE_BEGIN

/// get node global UID
///param[out]	szNodeID	-the buffer for the id string
///param[in]	nSize		-the size of the buffer
///return					-True if success, False else
BOOL	GetNodeGUID(char *szNodeID,DWORD nSize);

/// get unique id string or number
class UniqueId
{
public:
	UniqueId();
	~UniqueId();

	operator const char* () const { return _idstr.c_str();}
	operator const uint64 () const { return _id; }

	static std::string convertToStr(const uint64& id);
	
private:
	uint64 _id;
	std::string _idstr;
};


////////////////////////////////
// URL related info
////////////////////////////////
#define URL_PROTOCOL_MPF		"MPF"

#define URL_PATH_SESSION		"session"
#define URL_PATH_WORKNODE		"worknode"
#define URL_PATH_TASK			"task"

#define URL_VARNAME_SESSION_ID		"sid"
#define URL_VARNAME_WORKNODE_ID		"wid"
#define URL_VARNAME_TASK_ID			"tid"
#define URL_VARNAME_TASK_TYPE		"tasktype"
#define URL_VARNAME_ACTION_ID		"actionid"

#define	URL_VAR_SESSION_DUMMY		"0"

#define	DUMMY_SESSION_URL	(URL_PROTOCOL_MPF"//localhost:10000/" URL_PATH_SESSION "?" URL_VARNAME_SESSION_ID "=0")

#define TOLOWER(_S) \
	std::transform(_S.begin(), _S.end(), _S.begin(), (int(*)(int)) tolower)

inline	char* sfstrncpy(char* dest, const char* src, size_t destbufsize)
{
	if (0 == dest)
	{
		return 0;
	}

	if (0 == src)
		return 0;

	size_t i = 0;
	for (; i < destbufsize-1; ++i)
	{
		if ('\0' == src[i])
			break;
		dest[i] = src[i];
	}
	dest[i] = '\0';

	return dest;
}

//URL string productor
class URLStr
{
public:
	URLStr(const char* urlstr=NULL, bool casesensitive=false): bCase(casesensitive)
	{
		mPort=0;
		parse(urlstr);
	}
		  	
	bool parse(const char* urlstr)
	{
		if (urlstr==NULL)
			return false;

		std::string wkurl = urlstr;
		if (wkurl.empty())
			return true;

		int qpos=wkurl.find('?');
		int cpos=wkurl.find(':');
		int spos=wkurl.find('/');
		int epos=wkurl.find('=');

		std::string surl, searchurl;
		if (cpos>0 && cpos <spos)
		{
			// has protocol and server
			mProtocol = wkurl.substr(0, cpos);
			surl = wkurl.substr(cpos+1, qpos-cpos-1);
		}
		else searchurl = wkurl;

		if (qpos>=0)
			searchurl = wkurl.substr(qpos+1);

		if (!surl.empty())
		{
			int pos = surl.find_first_not_of("/");
			surl = (pos>=0) ? surl.substr(pos): surl;

			pos = surl.find_first_of("/");
			if (pos>0)
			{
				std::string tmpStr;
				tmpStr = surl.substr(0, pos);
				int portpos = tmpStr.find_first_of(":");
				
				mHost = (portpos>=0)? tmpStr.substr(0,portpos) : tmpStr;
				mPort = (portpos>=0)? atoi(tmpStr.substr(portpos+1).c_str()) : 0;
				
				mPath = (pos<surl.length()-1) ? surl.substr(pos+1) :"";
			}
			else
			{
				mHost = surl;
				mPath = "";
			}
		}

		if (!bCase)
		{
			TOLOWER(mHost);
			TOLOWER(mProtocol);
		}

		searchurl +="&";
		for (int pos = searchurl.find("&");
			 pos>=0 && pos <searchurl.length();
			 searchurl = searchurl.substr(pos+1), pos = searchurl.find("&"))
		{
			std::string wkexpress = searchurl.substr(0, pos);
			if (wkexpress.empty())
				continue;

			int qpos=wkexpress.find("=");
			std::string var, val;

			var = (qpos>0) ? wkexpress.substr(0, qpos):wkexpress;
			val = (qpos>0) ? wkexpress.substr(qpos+1) : "";

			char* buf = new char[wkexpress.length()+2];
			decode(var.c_str(), buf);
			var = buf;

			decode(val.c_str(), buf);
			val = buf;

			delete [] buf;

			if (!bCase)
				TOLOWER(var);
			mVars[var] = val;
		}

		return true;
	}	

	const char* getProtocol()
	{
		return mProtocol.c_str();
	}
	
	const char* getHost()
	{
		return mHost.c_str();
	}
	
	const char* getPath()
	{
		return mPath.c_str();
	}
	
	int		  getPort()
	{
		return mPort;
	}		
	
	void  setProtocol(const char* value)
	{
		if (value==NULL)
			return;
		
		mProtocol=value;
	}
	
	void  setHost(const char* value)
	{
		if (value==NULL)
			return;
		mHost=value;
	}
	
	void  setPath(const char* value)
	{
		if (value==NULL)
			return;
		mPath=value;
	}
	
	void	setPort(const int value)
	{
		if(value==0)
			return;
		mPort=value;
	}
	
	const char* getVarname(int idx=0)
	{
		int j =idx;
		for (urlvar_t::iterator i = mVars.begin(); i!= mVars.end() && j>0; i++, j--)
			;
		return (i== mVars.end()) ? NULL : i->first.c_str();
	}
	
	const char* getVar(const char* var)
	{
		if (var==NULL || mVars.find(var) == mVars.end())
			return "";
		
		return mVars[var].c_str();
	}		
	
	void  setVar(const char* var, const char* value)
	{
		if (var==NULL || *var==0x00)
			return;
		mVars[var]=(value==NULL)?"" : value;
	}
	
	const char* generate()
	{
		char portBuff[16];
		itoa(mPort, portBuff, 10);
		output_str = mProtocol + "://" +mHost + ":" + portBuff +"/" + mPath +"?";
		
		char varBuff[256];
		for (urlvar_t::iterator i = mVars.begin(); i!= mVars.end(); i++)
		{
			ZeroMemory(varBuff, 256);
			encode((void*)i->first.c_str(), varBuff, 256);
			output_str += varBuff;
			
			output_str += "=";
			
			ZeroMemory(varBuff, 256);
			encode((void*)i->second.c_str(), varBuff, 256);
			output_str += varBuff;
		}
		
		return output_str.c_str();
	}
	
	void  clear()
	{
		mProtocol=mHost=mPath="";
		mVars.clear();
	}
	
public:
	static bool encode(const void* source, char* target, int outlen, int len=-1)
	{
		if (source==NULL || outlen==0)
			return false;
		
		if (len<0)
			len = strlen((char*) source);
		
		const __int8 *sptr=(const __int8 *)source;
		
		std::string ret;
		
		for(int i=0; i<len; i++)
		{
			// The ASCII characters digits or letters, and ".", "-", "*", "_"
			// remain the same
			if (isdigit(sptr[i]) || isalpha(sptr[i])
				|| sptr[i]=='.' || sptr[i]=='-' ||	sptr[i]=='*' || sptr[i]=='_' )
			{
				ret += (char) sptr[i];
				continue;
			}
			
			// The space character ' ' is converted into a plus	sign '+'
			if (sptr[i]==' ')
			{
				ret += '+';
				continue;
			}
			
			//All other characters are converted into the 3-character string "%xy",
			// where xy is the two-digit hexadecimal representation of the lower
			// 8-bits of the character
			unsigned int hi, lo;
			hi= ((unsigned int)sptr[i] & 0xf0) / 0x10;
			lo= (unsigned int) sptr[i] % 0x10;
			
			hi+=(hi<10)? '0' : ('a' -10);
			lo+=(lo<10)? '0' : ('a' -10);
			
			ret += '%';
			ret += (char) (hi &0xff);
			ret += (char) (lo &0xff);
		}
		
		sfstrncpy(target, ret.c_str(), outlen);
		
		return true;
	}
	
	static bool decode(const char* source, void* target, int maxlen=-1)
	{
		int slen=strlen(source);
		unsigned __int8 *targ = (unsigned __int8 *)target;
		
		if (targ ==NULL)
			return false;
		
		int s, t;
		for(s=0, t=0; s<slen && (t<maxlen || maxlen<0); s++, t++)
		{
			// a plus sign '+' should be convert back to space ' '
			if (source[s]=='+')
			{
				targ[t]=' ';
				continue;
			}
			
			// the 3-character string "%xy", where xy is the
			// two-digit hexadecimal representation should be char
			
			if (source[s]=='%')
			{
				unsigned int hi, lo;
				
				hi=(unsigned int) source[++s];
				lo=(unsigned int) source[++s];
				
				hi -=(isdigit(hi) ? '0' : ('a' -10));
				lo -=(isdigit(lo) ? '0' : ('a' -10));
				
				if ((hi & 0xf0)|| (lo &0xf0))
					return false;
				
				targ[t]=(hi*0x10 +lo) &0xff;
				continue;
			}
			
			// The ASCII characters 'a' through 'z', 'A' through
			// 'Z', '0' through '9', and ".", "-", "*", "_" remain the same
			targ[t]= source[s];
		}
		
		if (t<maxlen || maxlen<0)
			targ[t]=0x00;
		
		return true;
	}		  
	
private:
	
	std::string mProtocol, mHost, mPath;
	int	mPort;
	typedef std::map<std::string, std::string> urlvar_t;
	urlvar_t mVars;
	bool bCase;
	std::string output_str;
}; 



#define ConvertToStrId UniqueId::convertToStr

//database entry path
#define DB_SPEC "/"
#define DB_ROOT DB_SPEC

//the path of the berkeley database node
class NodePath
{
public:
	//get berkeley database root path
	static std::string getRootPath()
	{
		return DB_ROOT;
	}

	//get the sub of current path
	static std::string getSubPath(const std::string& strParentPath, const std::string& strSubNodeName)
	{
		std::string strResult;
		if (*(strParentPath.rbegin()) == DB_SPEC[0])
		{
			strResult = strParentPath + strSubNodeName;
		}
		else
		{
			strResult = strParentPath + DB_SPEC + strSubNodeName;
		}
		return strResult;
	}
		

	//get the parent of current path
	static std::string getParentPath(const std::string& strPath)
	{
		if (strPath == DB_ROOT)
		{
			return DB_ROOT;
			//throw SRMException("NodePath::getParentPath : can not get the parent path when current path is root");
		}
		int nDevisionPosition = strPath.rfind(DB_SPEC);
		if (std::string::npos == nDevisionPosition ||
			0 == nDevisionPosition)
		{
			return "";
			//throw SRMException("NodePath::getParentPath: get error style of the path name: %s", strPath.c_str());
		}
		std::string strResult = strPath.substr(0, nDevisionPosition);

		return strResult;
	}
	
	//get the pure name
	static std::string getPureName(const std::string& strPath)
	{
		if (strPath == DB_ROOT)
		{
			return DB_ROOT;
			//throw SRMException("NodePath::getPureName : can not get the pure name when current path is root");
		}
		int nDevisionPosition = strPath.rfind(DB_SPEC);
		std::string strResult;
		if (std::string::npos == nDevisionPosition)
		{
			strResult = strPath;
		}
		else
		{
			strResult = strPath.substr(nDevisionPosition+1);
		}

		return strResult;
	}

	//
	static bool isRootPath(const std::string& strPath)
	{
		return strPath[0] == DB_ROOT[0];
	}		
};

#define FILE_DEVISION '\\'

//the path of the file system[Micorsoft Windows(R)]
class FilePath
{
public:
	//get the sub of current path
	static std::string getSubPath(const std::string& strPath, const std::string& strSubNodeName)
	{
		std::string strResult;
		if (*(strPath.rbegin()) == FILE_DEVISION)
		{
			strResult = strPath + strSubNodeName;
		}
		else
		{
			strResult = strPath + FILE_DEVISION + strSubNodeName;
		}
		return strResult;
	}

	//get the parent of current path
	static std::string getParentPath(const std::string& strPath)
	{
		int nDevisionPosition = strPath.rfind(FILE_DEVISION);
		if (std::string::npos == nDevisionPosition ||
			0 == nDevisionPosition)
		{
			return "";
			//throw SRMException("FilePath::getParentPath: get error style of the path name: %s", strPath.c_str());
		}
		std::string strResult = strPath.substr(0, nDevisionPosition);

		return strResult;
	}

	//get current module name
	static std::string getModuleName()
	{
		char strTemp[MAX_PATH] = {0};
		if (0 == GetModuleFileNameA(NULL, strTemp, MAX_PATH))
		{
			return "";
			//throw SRMException("FilePath::getMouduleName : can not get module file name with error code %d", GetLastError());
		}

		return strTemp;
	}

	//get current module path
	static std::string getModulePath()
	{
		std::string strModuleName = getModuleName();
		std::string strResult = getParentPath(strModuleName);
		
		return strResult;
	}


};

MPF_UTILITY_NAMESPACE_END

///print message string to screen
void print_screen(const char* message);
//get time string from time_t number
std::string getTimeStr(time_t tml);

#endif // __MPFCommon_H__
