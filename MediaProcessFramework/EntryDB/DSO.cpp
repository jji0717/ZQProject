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
// Desc  : impl dynamic shared object
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/DSO.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/DSO.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     4/14/05 10:11a Hui.shao
// 
// 2     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 1     4/12/05 5:11p Hui.shao
// ============================================================================================

#include "DSO.h"

ENTRYDB_NAMESPACE_BEGIN

DSO::DSO(const char* filename)
    :mItCount(0), library(NULL)
{
	mFilename[0]=mFilename[1]=0x00;
}

DSO::~DSO()
{
	if(IsLoaded()) 
		Free();
}

bool DSO::Load(const char* filename)
{ 
	if(IsLoaded()) 
		return false;
	
	library = ::LoadLibrary(filename);
	
	if (library == NULL)
		return false;
	
	strcpy(mFilename, filename);
	if (mapExternFunc(true))
	{
		mItCount ++;
		return true;
	}
	else return false;
}

void DSO::Free()
{
	if(library != NULL)
		::FreeLibrary(library);
	library = NULL;
	
	mapExternFunc(false);
}

bool DSO::IsLoaded()
{
	return library != NULL;
}

const char* DSO::getImageName() const
{
	return mFilename;
}

DSOInterface::DSOInterface(DSO* pdso)
             : pDso(pdso), bMapped(false), cInstance(0)
{
	if (pdso==NULL || !pDso->IsLoaded())
		return;

	if (bMapped || pDso==NULL && !pDso->IsLoaded())
		return;
	
	MapFunc(pdso);
}

DSOInterface::~DSOInterface()
{
	if (!isValid())
		return;

	if (mapExternFunc(false))
		pDso->mItCount --;
}

bool DSOInterface::MapFunc(DSO* pdso)
{
	if (isValid())
		return false;

	pDso=pdso;

	if (pDso==NULL && !pDso->IsLoaded())
		return false;

	if (mapExternFunc(true))
	{
		pDso->mItCount ++;
		bMapped = true;

		return true;
	}
	return false;
}

int64 DSOInterface::instanceCount()
{
	return cInstance;
}

const char* DSOInterface::getImageName()
{
	return (isValid()) ? pDso->getImageName() : NULL;
}

bool DSOInterface::isValid()
{
	return (bMapped && pDso!=NULL && pDso->IsLoaded());
}

ENTRYDB_NAMESPACE_END
