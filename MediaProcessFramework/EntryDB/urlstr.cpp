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
// Desc  : impl URL string
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/urlstr.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/urlstr.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     05-06-01 18:08 Bernie.zhao
// moved constructor to .h file, to avoid error in exe with /O2,  which
// uses this DLL
// 
// 3     4/13/05 6:33p Hui.shao
// changed namespace
// 
// 2     4/12/05 5:18p Hui.shao
// ============================================================================================

#include "URLStr.h"
#include <algorithm>
#include <locale>

ENTRYDB_NAMESPACE_BEGIN

const char* URLStr::encode(const void* source, int len)
{
	if (source==NULL)
		return NULL;

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
	
	return ret.c_str();
}

bool URLStr::decode(const char* source, void* target, int maxlen)
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

#define TOLOWER(_S) \
	std::transform(_S.begin(), _S.end(), _S.begin(), (int(*)(int)) tolower)

bool URLStr::parse(const char* urlstr)
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
			mHost = surl.substr(0, pos);
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

const char* URLStr::getVarname(int idx)
{
	int j =idx;
	for (urlvar_t::iterator i = mVars.begin(); i!= mVars.end() && j>0; i++, j--)
		;
	return (i== mVars.end()) ? NULL : i->first.c_str();
}

const char* URLStr::getVar(const char* var)
{
	if (var==NULL || mVars.find(var) == mVars.end())
		return "";

	return mVars[var].c_str();
}

void URLStr::setVar(const char* var, const char* value)
{
	if (var==NULL || *var==0x00)
		return;
	mVars[var]=(value==NULL)?"" : value;
}

void URLStr::setProtocol(const char* value)
{
	if (value==NULL)
		return;

	mProtocol=value;
}

void URLStr::setHost(const char* value)
{
	if (value==NULL)
		return;
	mHost=value;
}

void URLStr::setPath(const char* value)
{
	if (value==NULL)
		return;
	mPath=value;
}

const char* URLStr::generate()
{
	output_str = mProtocol + "://" +mHost +"/" + mPath +"?";

	for (urlvar_t::iterator i = mVars.begin(); i!= mVars.end(); i++)
	{
		output_str += encode((void*)i->first.c_str());
		output_str += "=";
		output_str += encode((void*)i->second.c_str());
	}

	return output_str.c_str();
}

void URLStr::clear()
{
	mProtocol=mHost=mPath="";
	mVars.clear();
}

const char* URLStr::getProtocol()
{
	return mProtocol.c_str();
}

const char* URLStr::getHost()
{
	return mHost.c_str();
}

const char* URLStr::getPath()
{
	return mPath.c_str();
}

ENTRYDB_NAMESPACE_END
