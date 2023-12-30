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
// Desc  : define an add-in framework
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/Addins.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/Addins.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 7     05-05-30 2:42p Daniel.wang
// 
// 6     05-04-14 23:04 Daniel.wang
// 
// 5     4/14/05 10:12a Hui.shao
// 
// 3     4/13/05 7:03p Hui.shao
// 
// 2     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 1     4/12/05 5:07p Hui.shao
// ============================================================================================

#ifndef __Addins_h__
#define __Addins_h__

#include "IEDB.h"
#include "DSO.h"
#include "Locks.h"

#include <vector>
#include <algorithm>
#include <locale>

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API AddinImage;
class ENTRYDB_API AddinManager;
template<class _AddinInterface> class ENTRYDB_API FactoryModule;

class AddinImage : public DSO
{
	friend class AddinManager;

	DECLARE_PROC_SO(AddinImage, DSO)
};

class AddinManager
{
public:
	AddinManager(const char* path=NULL, const char* ext=ADDIN_MOD_EXT);
	~AddinManager();

	bool populate(const char* path=NULL, const char* ext=ADDIN_MOD_EXT);
	void unpopulate();

	AddinImage* operator[](int index);
	const int size() const;

	typedef std::vector<AddinImage*> vaddin_t;
	vaddin_t vaddins;
	Mutex vaddins_lock;

private:
	bool bDestructor;
};

extern ENTRYDB_API AddinManager gblAddinManager;

template<class _AddinInterface>
class FactoryModule
{
public:

	FactoryModule();
	FactoryModule(AddinManager& admg);
	~FactoryModule();

	int size() { return vmods.size(); }
	_AddinInterface* operator[](int index) { return (index>=0 && index< size()) ? vmods[index]: NULL; }

protected:
	typedef std::vector<_AddinInterface*> vmods_t;
	vmods_t vmods;
	Mutex vmods_lock;
	AddinManager* pAddinManager;

};

template<class _AddinInterface>
FactoryModule<_AddinInterface>::FactoryModule()
           :pAddinManager(NULL)
{
}

template<class _AddinInterface>
FactoryModule<_AddinInterface>::FactoryModule(AddinManager& admg)
           :pAddinManager(&admg)
{

	if (pAddinManager==NULL)
		return;

	vmods_lock.enter();
	for (int i=0; i< pAddinManager->size(); i++)
	{
		AddinImage* pImage = (*pAddinManager)[i];
		if (pImage==NULL || !pImage->IsLoaded())
			continue;

		_AddinInterface *pModule = new _AddinInterface();
		if (pModule == NULL)
			continue;

		pModule->isInternal =false;
		pModule->MapFunc(pImage);

		if (!pModule->isValid())
		{
			pModule->free();
			continue; // next image
		}
		else vmods.push_back(pModule);
	}
	vmods_lock.leave();
}

template<class _AddinInterface>
FactoryModule<_AddinInterface>::~FactoryModule()
{
	vmods_lock.enter();
	for (vmods_t::iterator i = vmods.begin(); i <vmods.end(); i++)
	{
		(*i)->free();
		vmods.erase(i);
	}
	vmods_lock.leave();
}

ENTRYDB_NAMESPACE_END

#endif // __Addins_h__
