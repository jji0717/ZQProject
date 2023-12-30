// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This was copied from enterprise domain object sys, edos's copyright is belong to Hui Shao
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
// Desc  : in-memory entry database
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBNil.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBNil.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 6     05-07-28 18:39 Daniel.wang
// 
// 5     05-05-10 16:00 Daniel.wang
// 
// 4     05-05-08 16:05 Daniel.wang
// 
// 3     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 2     4/12/05 5:38p Hui.shao
// ============================================================================================

#include "EDBNil.h"
#include "EDBImpl.h"
#include "urlstr.h"
#include "Timestamp.h"

ENTRYDB_NAMESPACE_BEGIN

EDBNil::EDBNil()
{
	mEDB.clear();
}

EDBNil::~EDBNil()
{
	disconnectDB();
}

int64 EDBNil::cInstance=0;

bool EDBNil::connectDB(const char* url, ITransaction* txn)
{
	if (url == NULL || isDBConnected())
		return false;

	std::string protocol, hostname, path;

	URLStr urlstr(url);
	protocol = urlstr.getProtocol();
	hostname = urlstr.getHost();
	path = urlstr.getPath();

	if (protocol.compare(EDBNILURLPROT)!=0)
		return false;

	if (path.empty())
		urlstr.setPath(newID());
	
	mUrl = urlstr.generate();

	mEDB[LOGIC_SEPS] = mAttrs;
	bool ret= openRoot();

	if (ret)
		cInstance++;

	return ret;
}

bool EDBNil::isDBConnected()
{
	return (!mEDB.empty());
}

void EDBNil::disconnectDB()
{
	if (isDBConnected())
		cInstance--;
	else
		mEDB.clear();
}

void EDBNil::free()
{
	delete this;
}

const char* EDBNil::getDBURL()
{
	return mUrl.c_str();
}

bool EDBNil::openEntry(const char* e, bool creatIfNotExist, ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL || strlen(e) <=0)
		return false;

	std::string entry =e;

	if (crntentry==e)
		return true;

	int ret =-1;

	entry_map_t::iterator i = mEDB.find(entry);

	if (i != mEDB.end())
	{
		crntentry =i->first;
		mAttrs = i->second;
		
		return true;
	}
	
	if (!creatIfNotExist)
		return false;

	if (entry.compare(LOGIC_SEPS)!=0)
	{
		// not a root, check if the parent exists
		int pos = entry.find_last_of(LOGIC_SEPC);
		std::string p = entry.substr(0, pos);
		if (!openEntry(p.c_str()))
			return false;
	}

	mAttrs.clear();
	crntentry = entry;
	mEDB[entry] = mAttrs;

	return true;
}

bool EDBNil::copyEntry(const char* eto, const char* efrom, bool overwrite, ITransaction* txn)
{
	if (eto==NULL || *eto==0x00)
		return false;

	if (efrom!=NULL && crntentry.compare(efrom)!=0)
	{
		if (!openEntry(efrom, false))
			return false;
	}

	std::string wk_entry = crntentry;
	attrs_t bkattr = mAttrs;

	if (wk_entry.length() <3)
		return false; // can not copy a root

	if (openEntry(eto, false))
	{
		if (!overwrite)
			return false;

		deleteEntry(eto);
	}

	if (!openEntry(eto, true))
		return false;

	mAttrs = bkattr;
	bkattr.clear();
	commitChanges();

	bool succ =true;
	openEntry(wk_entry.c_str(), true);
	for (bool hasChild =openFirstChild(); hasChild && succ;  hasChild=openNextSibling())
	{
		std::string targetentry=eto;
		targetentry += crntentry.substr(wk_entry.length());
		succ =copyEntry(targetentry.c_str(), NULL, true);
	}
	openEntry(wk_entry.c_str(), true);

	return succ;
}

bool EDBNil::openRoot(ITransaction* txn)
{
	return openEntry(LOGIC_SEPS, true);
}

bool EDBNil::openParent(ITransaction* txn)
{
	if (crntentry.length() <=2)
		return false;

	int pos=crntentry.find_last_of(LOGIC_SEPC);
	if (pos <1)
		return false;

	return openEntry(crntentry.substr(0, pos).c_str());
}

bool EDBNil::openNextSibling(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	std::string entry2backup = crntentry;

	int pos =crntentry.find_last_of(LOGIC_SEPC);
	if (pos<1)	return false; // root doesn't have Sibling

	std::string entry2try = crntentry + LOGIC_SEPS "~"; // append with a small ascii

	bool succ = openFirstGE(entry2try.c_str());
	succ = succ && (entry2backup.compare(0, pos+1, crntentry, 0, pos+1) ==0);

	if (!succ)
		openEntry(entry2backup.c_str());

	return succ;
}

bool EDBNil::openFirstChild(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	std::string crnt2backup = crntentry;
	std::string entry2try = crntentry;
	int pos =entry2try.find_last_of(LOGIC_SEPC);
	if (pos == std::string::npos || pos<1)
		entry2try = LOGIC_SEPS;

	entry2try += LOGIC_SEPS;
	pos =entry2try.find_last_of(LOGIC_SEPC);
	
	bool ret = openFirstGE((entry2try+"!").c_str());
	ret = ret && (entry2try.compare(0, pos+1, crntentry,0, pos+1) ==0);

	if (!ret)
		openEntry(crnt2backup.c_str());

	return ret;
}

bool EDBNil::openFirstGE(const char* e, ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL || *e==0x00)
		return false;

	int ret = -1;
	mAttrs.clear();
	entry_map_t::iterator i = mEDB.lower_bound(e);

	if (i==mEDB.end())
		return false;

	crntentry = i->first;
	mAttrs = i->second;
	return true;
}

bool EDBNil::openNext(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	int ret = -1;
	mAttrs.clear();

	entry_map_t::iterator i = mEDB.find(crntentry);
	if (i++ == mEDB.end() || i ==mEDB.end())
		return false;

	crntentry = i->first;
	mAttrs = i->second;
	return true;
}

bool EDBNil::commitChanges(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());
	
	mEDB[crntentry] = mAttrs;
	return true;
}

bool EDBNil::deleteEntry(const char* e, ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL)
		e=crntentry.c_str();

	if (strlen(e)<2)
		return false; // cann't del the root;

	std::string crnt2backup = crntentry;
	std::string crnt2delete = e;

	if (crnt2backup.compare(crnt2delete) !=0 && !openEntry(crnt2delete.c_str()))
	{
		openEntry(crnt2backup.c_str());
		return false;
	}

	for(bool succ = openFirstChild();
		succ; succ = succ && openNextSibling())
	{
		if (!deleteEntry(NULL))
			return false;
	}

	int ret =0;
	if (openEntry(crnt2delete.c_str()))
	{
		entry_map_t::iterator i = mEDB.find(crntentry);
		mEDB.erase(i);
	}

	if (crnt2backup.compare(crnt2delete) !=0)
		openEntry(crnt2backup.c_str());
//	else openParent();

	return true;
}

const char* EDBNil::getCurrentEntry()
{
	return crntentry.c_str();
}

// const char* EDBNil::getRealEntry()
// {
// 	return realentry.c_str();
// }

const char* EDBNil::getAttrName(const int index, ITransaction* txn)
{
	int j =index;
	for (attrs_t::iterator i = mAttrs.begin(); i!= mAttrs.end() && j>0; i++, j--)
		;
	return (i== mAttrs.end()) ? NULL : i->first.c_str();
}

const char* EDBNil::getAttribute(const char* attrname, ITransaction* txn)
{
	if (attrname ==NULL)
		return NULL;
	std::string& v =mAttrs[attrname];
	return v.c_str();
}

bool EDBNil::setAttribute(const char* attrname, const char*value, ITransaction* txn)
{
	std::string n=(attrname ==NULL)? "": attrname, v =(value ==NULL)? "":value;

	if (n.empty())
		return false;

	mAttrs[n]=v;
	return true;
}

bool EDBNil::setReference(const char* e, ITransaction* txn)
{
	if (e==NULL || strlen(e) <=0)
		return false;

	std::string entry2backup = crntentry;

	bool ret = openEntry(e);

	openEntry(entry2backup.c_str());

	if (ret)
	{
		mAttrs.clear();
		setAttribute(ENTRY_TYPE, "ref");
		setAttribute("ref", e);
	}

	return ret;
}

bool EDBNil::isReference()
{
	return (mAttrs[ENTRY_TYPE].compare("ref") ==0);
}
	
bool EDBNil::associateReference()
{
	if (!isReference())
		return true;

	//TODO: implement later
	return false;
}

void EDBNil::createTxn(ITransaction*& txn)
{
	txn = NULL;
}

void EDBNil::deleteTxn(ITransaction*& txn)
{
	txn = NULL;
}

IEDB* EDBNilConnect(const char* url)
{
	EDBNil* db = new EDBNil;
	if (!db->connectDB(url))
	{
		delete db;
		return NULL;
	}

	return db;
}

const char* EDBNilURLHelp(int i)
{
	return EDBNILURLPROT "://localhost/";
}

const char* EDBNilURLProtocol(int i)
{
	return EDBNILURLPROT;
}

const char* EDBNilType(int i)
{
	return "EntryDB-NIL";
}

const char* EDBNilDesc(int i)
{
	return "Entry Database, Memory-Only Edition";
}

const int64 EDBNilCount(void)
{
	return EDBNil::cInstance;
}

ENTRYDB_NAMESPACE_END
