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
// Desc  : entry db implementation with berkeley database 2
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 5     4/14/05 10:13a Hui.shao
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 8:34p Hui.shao
// 
// 2     4/12/05 5:59p Hui.shao
// ============================================================================================

#include "EDBb2.h"
#include "EDBb2Mgr.h"
#include "EDBImpl.h"
#include "urlstr.h"
// #include "Timestamp.h"

ENTRYDB_NAMESPACE_BEGIN

EDBb2::EDBb2(const char* dbpath, const char*errf)
		 :wk_data(NULL)
{
	mECB.dbp = NULL;
	mECB.cur = NULL;
	mECB.errf = NULL;
	initworkdata();

	std::string url = EBDB2URLPROT "://localhost/";
	url += (dbpath !=NULL && *dbpath !=0x00) ? dbpath : "ebdb2";

	if (!connectDB(url.c_str()))
		throw EDBException("could not connect to database");
}

EDBb2::EDBb2()
		  :wk_data(NULL)
{
	mECB.dbp = NULL;
	mECB.cur = NULL;
	mECB.errf = NULL;
	initworkdata();
}

EDBb2::~EDBb2()
{
	disconnectDB();
}

bool EDBb2::connectDB(const char* url)
{
	if (url == NULL || isDBConnected())
		return false;

	std::string protocol, hostname, path;

	URLStr urlstr(url);
	protocol = urlstr.getProtocol();
	hostname = urlstr.getHost();
	path = urlstr.getPath();

	if (protocol.compare(EBDB2URLPROT)!=0 || path.empty())
		return false;

	if (hostname.empty() || (hostname.compare("localhost")!=0 && hostname.compare(0, 4,"127.", 0, 4)!=0))
		return false;

	for (int i=0; i< path.length(); i++)
		if (path[i]=='/') path[i] = FNSEPC;
		
	int dpos =path.find_last_of('.');
	if (dpos<0 || path.find_last_of(FNSEPC) >= dpos)
		path += ".ed2";
		
	mECB.dbpath= path;
		
	bool ret =edb2mgr.connect(mECB);
		
	if (ret)
	{
		mUrl = url;
		openRoot(); // create the root entry
		_nInstance_in_EDM++;
	}
	
	return ret;
}

bool EDBb2::isDBConnected()
{
	return (mECB.dbp !=NULL);
}

void EDBb2::disconnectDB()
{
	if (wk_data!=NULL)
		delete []wk_data;
	wk_data = NULL;

	if (isDBConnected())
	{
		edb2mgr.disconnect(mECB);
		_nInstance_in_EDM--;
	}
}

void EDBb2::free()
{
	delete this;
}

const char* EDBb2::getDBURL()
{
	return mUrl.c_str();
}

void EDBb2::initworkdata(int size)
{
	if (wk_data !=NULL)
		delete []wk_data;

	if (size <=0)
	{
		wk_data =NULL;
		wk_datasize=0;
	}
	else
	{
		wk_datasize =size;
		wk_data = new uint8[wk_datasize];
	}
}

bool EDBb2::openEntry(const char* e, bool creatIfNotExist)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL || strlen(e) <=0)
		return false;

	std::string entry =e;

	if (crntentry==entry)
		return true;

	mAttrs.clear();

	int ret =-1;
	try
	{
		Dbt Key((void*)entry.c_str(), entry.length()), Data;
		ret = mECB.cur->get(&Key, &Data, DB_SET);

		if (ret ==0)
		{
			crntentry ="" ; //realentry="";
			crntentry.append((const char*)Key.get_data(), Key.get_size());

			initworkdata(Data.get_size());
			memcpy(wk_data, Data.get_data(), Data.get_size());
			populateDBData();

			initworkdata();
		}
	}
	catch (DbException dbe)
	{
		throw EDBException(dbe.what());
	}
	catch (...)
	{
		throw EDBException("could not access database");
	}

	if (ret ==0)
		return true;

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
	
	mAttrs.clear(); // for opened parent
	try
	{
		Dbt Key((void*)entry.c_str(), entry.length()), Data;
		u_int32_t flags = 0;
		ret = mECB.dbp->put(NULL, &Key, &Data, flags);

		if (ret ==0)
//		{
//			realentry="";
			crntentry =entry;
//		}
	}
	catch (DbException dbe)
	{
		throw EDBException(dbe.what());
	}
	catch (...)
	{
		throw EDBException("could not access database");
	}

	return (ret ==0);
}

bool EDBb2::copyEntry(const char* eto, const char* efrom, bool overwrite)
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


bool EDBb2::openRoot()
{
	return openEntry(LOGIC_SEPS, true);
}

bool EDBb2::openParent()
{
	if (crntentry.length() <=2)
		return false;

	int pos=crntentry.find_last_of(LOGIC_SEPC);
	if (pos <1)
		return false;

	return openEntry(crntentry.substr(0, pos).c_str());
}

bool EDBb2::openNextSibling()
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

bool EDBb2::openFirstChild()
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

bool EDBb2::openFirstGE(const char* e)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL || *e==0x00)
		return false;

	int ret = -1;
	try
	{
		Dbt Key((void*)e, strlen(e)), Data;
		ret = mECB.cur->get(&Key, &Data, DB_SET_RANGE);

		if (ret ==0)
		{
			crntentry =""; // realentry="";
			crntentry.append((const char*)Key.get_data(), Key.get_size());

			mAttrs.clear();
			initworkdata(Data.get_size());

			memcpy(wk_data, Data.get_data(), wk_datasize);

			populateDBData();

			initworkdata();
		}
	}
	catch (DbException dbe)
	{
//		throw EDBException(dbe.what());
	}
	catch (...)
	{
//		throw EDBException("could not access database");
	}

	return (ret ==0);
}

bool EDBb2::openNext()
{
	ASSET_BOOL(isDBConnected());

	int ret = -1;
	mAttrs.clear();
	try
	{
		Dbt Key, Data;
		ret = mECB.cur->get(&Key, &Data, DB_NEXT);

		if (ret ==0)
		{
			crntentry =""; // realentry="";
			crntentry.append((const char*)Key.get_data(), Key.get_size());

			initworkdata(Data.get_size());
			memcpy(wk_data, Data.get_data(), Data.get_size());
			populateDBData();

			initworkdata();
		}
	}
	catch (DbException dbe)
	{
		throw EDBException(dbe.what());
	}
	catch (...)
	{
		throw EDBException("could not access database");
	}

	return (ret ==0);
}

bool EDBb2::setReference(const char* e)
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

bool EDBb2::commitChanges()
{
	ASSET_BOOL(isDBConnected());

	int ret =-1;
	try
	{
		attrs2DBData();

		Dbt Key((void*)crntentry.c_str(), crntentry.length()), Data(wk_data, wk_datasize);
		u_int32_t flags = 0; // overwrite
		ret = mECB.dbp->put(NULL, &Key, &Data, flags);

//		if (ret ==0)
//			mECB.dbp->sync(0);

		initworkdata();
	}
	catch (DbException dbe)
	{
		throw EDBException(dbe.what());
	}
	catch (...)
	{
		throw EDBException("could not access database");
	}

	return (ret ==0);
}

bool EDBb2::deleteEntry(const char* e)
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
		try
		{
			int ret = mECB.cur->del(0);
		}
		catch (DbException dbe)
		{
			throw EDBException(dbe.what());
		}
		catch (...)
		{
			throw EDBException("could not access database");
		}
	}

	if (crnt2backup.compare(crnt2delete) !=0)
		openEntry(crnt2backup.c_str());
//	else openParent();

	return (ret ==0);
}

const char* EDBb2::getCurrentEntry()
{
	return crntentry.c_str();
}

// const char* EDBb2::getRealEntry()
// {
// 	return realentry.c_str();
// }

const char* EDBb2::getAttrName(const int index)
{
	int j =index;
	for (attrs_t::iterator i = mAttrs.begin(); i!= mAttrs.end() && j>0; i++, j--)
		;
	return (i== mAttrs.end()) ? NULL : i->first.c_str();
}

const char* EDBb2::getAttribute(const char* attrname)
{
	if (attrname ==NULL)
		return NULL;
	std::string v=mAttrs[attrname];
	return mAttrs[attrname].c_str();
}

bool EDBb2::setAttribute(const char* attrname, const char*value)
{
	std::string n=(attrname ==NULL)? "": attrname;
	std::string v =(value ==NULL)? "":value;

	if (n.empty())
		return false;

	mAttrs[n]=v;
	return true;
}

bool EDBb2::isReference()
{
	return (mAttrs[ENTRY_TYPE].compare("ref") ==0);
}
	
bool EDBb2::associateReference()
{
	if (!isReference())
		return true;

	//TODO: implement later
	return false;
}

int EDBb2::writeUTF8(char*buf, const char*str)
{
	int16 *plen = (int16 *)buf;
	if (plen ==NULL)
		return 0;

	*plen= (str==NULL) ? 0 :strlen(str);

	buf +=sizeof(int16);

	for (int i =0; i< *plen; i++)
		*buf++ = *str++;

	return *plen + sizeof (int16);
}

int EDBb2::readUTF8(const char*buf, char*str)
{
	if (buf ==NULL || str ==NULL)
		return 0;

	int16 *plen = (int16 *)buf;

	buf +=sizeof(int16);
	for (int i =0; i< *plen; i++)
		*str++ = *buf++;

	*str++=0x00;
	*str++=0x00;

	return *plen + sizeof (int16);
}

void EDBb2::populateDBData()
{
	mAttrs.clear();

	if (wk_data==NULL || wk_datasize<=0)
		return;

	char *p =(char*)wk_data, *tail = p + wk_datasize;
	char vn[MAX_PATH/4], vv[8192];

	while (p!=NULL && p<tail)
	{
		p +=readUTF8(p, vn);
		
		if (p<tail)
			p+=readUTF8(p, vv);
		else vv[0] =0x00;

		if (strlen(vn) <=0)
			break;
		mAttrs[vn]=vv;
	}
}

void EDBb2::attrs2DBData()
{
	initworkdata();

	attrs_t::iterator i;
	for (i=mAttrs.begin(); i !=mAttrs.end(); i++)
		wk_datasize += i->first.length() + i->second.length() +sizeof(int16) *2;

	if (wk_datasize<=0)
		return;

	wk_datasize +=8;

	initworkdata(wk_datasize);

	if (wk_data==NULL)
	{
		wk_datasize =0;
		return;
	}

	char *p=(char*)wk_data;

	for (i=mAttrs.begin(); i!=mAttrs.end(); i++)
	{
		p += writeUTF8(p, i->first.c_str());
		p += writeUTF8(p, i->second.c_str());
	}

	*p++=0x00;
	*p++=0x00;
	*p++=0x00;
	*p++=0x00;
	*p++=0x00;
	*p++=0x00;
}

#ifdef _DEBUG

void EDBb2::navigate()
{
	std::string backup = crntentry;

	openRoot();
	navigateN(0);

	openEntry(backup.c_str());

}

void EDBb2::navigateN(int depth)
{
	printf("[e] %s\n", crntentry.c_str());
	for (attrs_t::iterator i = mAttrs.begin(); i != mAttrs.end(); i++)
		printf("\t%s = %s\n", i->first.c_str(), i->second.c_str());

	bool hasChild = openFirstChild();
	bool succ = hasChild;

	for (; succ; succ=openNextSibling())
		navigateN(depth +1);

	if (hasChild) openParent();
}

#endif //_DEBUG

ENTRYDB_NAMESPACE_END

