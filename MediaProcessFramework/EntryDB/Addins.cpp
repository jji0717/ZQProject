// ============================================================================================
// Copyright (c) 1997, 1998 by
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
// Author: Hui Shao
// Desc  : impl an add-in framework
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/Addins.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/Addins.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 6     05-06-16 2:12p Daniel.wang
// 
// 5     05-05-26 16:11 Daniel.wang
// 
// 4     4/14/05 10:10a Hui.shao
// 
// 2     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 1     4/12/05 5:07p Hui.shao
// ============================================================================================

#include "Addins.h"
#include "EDBNil.h"

extern "C"
{
	#include <io.h>
};

ENTRYDB_NAMESPACE_BEGIN

AddinManager gblAddinManager;

AddinManager::AddinManager(const char* path, const char* ext)
			 :bDestructor(false)
{
	if (path!=NULL)
		populate(path, ext);
	else
	{
		char buf[MAX_PATH];
		::GetModuleFileName(NULL, buf, MAX_PATH-2);
		char*p = strrchr(buf, FNSEPC);
		if (p!=NULL)
			*p=0x00;
		populate(buf, ext);
	}
}

AddinManager::~AddinManager()
{
	bDestructor =true;
	unpopulate();
}

bool AddinManager::populate(const char* path, const char* ext)
{
	std::string wkpath = (path==NULL) ? "" : path;
	if (wkpath.empty())
		wkpath = ".";
	else
	{
		std::string testwkpath =wkpath +PHSEPS;
		std::string searchpath =getenv("PATH");
		searchpath +=PHSEPS;
#ifdef WIN32
		std::transform(testwkpath.begin(), testwkpath.end(), testwkpath.begin(), (int(*)(int)) tolower);
		std::transform(searchpath.begin(), searchpath.end(), searchpath.begin(), (int(*)(int)) tolower);
#endif
		int pos =searchpath.find(testwkpath);
		if (pos<0)
		{
			searchpath = "PATH=";
			searchpath += getenv("PATH");
			searchpath += PHSEPS;
			searchpath += wkpath;
			putenv(searchpath.c_str());
		}
	}

	if (wkpath[wkpath.length()-1] != FNSEPC)
		wkpath +=FNSEPS;

	std::string filespec = wkpath;
	filespec += "*";
	
	if (ext!=NULL && *ext!=0x00)
	{
		filespec += ".";
		filespec += ext;
	}
	
	long hFile;
	struct _finddata_t c_file;
	bool ret =false;
	
	vaddins_lock.enter();
	for (bool moreAddin= ((hFile = _findfirst(filespec.c_str(), &c_file)) !=-1L);
		 moreAddin;
		 moreAddin =(_findnext(hFile, &c_file) ==0))
	{
		std::string wkfile = wkpath;
		wkfile += c_file.name;

		vaddin_t::iterator i = vaddins.begin();

		bool aleadyLoaded=false;
		for (; !aleadyLoaded && i <vaddins.end(); i++)
		{
			if ((*i)==NULL || stricmp((*i)->getImageName(), wkfile.c_str())==0)
				aleadyLoaded =true; // for
		}
		
		if (aleadyLoaded)
			continue; // moreAddin
		
		AddinImage* pImage = new AddinImage();

		if (pImage ==NULL)
			break;
		
		if (!pImage->Load(wkfile.c_str()))
		{
			delete pImage;
			continue; // moreAddin
		}
		
		vaddins.push_back(pImage);
		ret =true;
		
		 } // moreAddin

	vaddins_lock.leave();
	
	_findclose(hFile);
	return ret;
}

void AddinManager::unpopulate()
{
	vaddins_lock.enter();

	/*
	for (vaddin_t::iterator i = vaddins.begin(); i <vaddins.end(); i++)
	{
		if (bDestructor)
		{
			delete (*i);
			vaddins.erase(i);
		}
	}
	*/

	//todo: test it is memory safe
	if (bDestructor)
	{ 
		int size = vaddins.size();
		for (int i = size-1; i >= 0; --i)
		{
			delete (vaddins[i]);
			vaddins.erase(vaddins.begin() + i);
		}
	}

	vaddins_lock.leave();
}

AddinImage* AddinManager::operator[](int index)
{
	if (index<0 || index>=vaddins.size())
		return NULL;
	else return vaddins[index];
}

const int AddinManager::size() const
{
	return vaddins.size();
}

ENTRYDB_NAMESPACE_END
