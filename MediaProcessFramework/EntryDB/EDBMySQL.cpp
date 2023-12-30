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
// Desc  : entry database MySQL edition
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBMySQL.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBMySQL.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 2     4/12/05 6:03p Hui.shao
// ============================================================================================


#include "EDBMySQL.h"
#include "urlstr.h"
#include "EDBImpl.h"

#define TBL_ENTRY		"_entry"
#define TBL_ATTRIBUTE	"_eattr"
#define COL_EID			"eid"
#define COL_ENTRY		"entry"
#define COL_ATTRNAME	"attrn"
#define COL_ATTRVALUE	"attrv"
#define TBLCOL(_T, _C)	_T "." _C

ENTRYDB_NAMESPACE_BEGIN

#ifdef _DEBUG
// #  define TRACESQL(_Q) printf("%s\n", _Q.preview())
#  define TRACESQL(_Q) {std::string st=_Q.preview(); }
#else
#  define TRACESQL(_Q) 
#endif

EDBMySQL::EDBMySQL()
         :mConn(NULL)
{
}

EDBMySQL::~EDBMySQL()
{
	disconnectDB();
}

bool EDBMySQL::connectDB(const char* url)
{
	if (url == NULL || isDBConnected())
		return false;

	std::string protocol, hostname, dbname, user, password;

	URLStr urlstr(url);
	protocol = urlstr.getProtocol();
	hostname = urlstr.getHost();
	dbname = urlstr.getPath();
	user = urlstr.getVar("user");
	password = urlstr.getVar("password");

	if (protocol.compare(EDBMYSQLURLPROT)!=0 || hostname.empty()||dbname.empty())
		return false;

	bool ret =false;
	try
	{
		mConn= new Connection(dbname.c_str(), hostname.c_str(), user.c_str(), password.c_str());
		ret = (mConn!=NULL);
	}
	catch(...){ return false; }

	if (!ret)
		return false;

	try
	{
		Query query = mConn->query();
		Result res;

		query << "CREATE TABLE IF NOT EXISTS " TBL_ENTRY " ("
		  << "    " COL_EID " bigint(20) NOT NULL auto_increment,"
		  << "    " COL_ENTRY " varchar(255) NOT NULL,"
		  << "    PRIMARY KEY  (" COL_EID "),"
		  << "    UNIQUE KEY " COL_ENTRY " (" COL_ENTRY ")"
		  << ");";
		TRACESQL(query);
		res = query.store(); 

		query.reset();
		query << "CREATE TABLE IF NOT EXISTS " TBL_ATTRIBUTE " ("
		  << "    " COL_EID " bigint(20) NOT NULL default '0',"
		  << "    " COL_ATTRNAME " varchar(128) NOT NULL default '',"
		  << "    " COL_ATTRVALUE " varchar(255) default NULL,"
		  << "    UNIQUE KEY attrkey (" COL_EID "," COL_ATTRNAME "),"
		  << "    KEY " COL_EID " ("COL_EID "),"
		  << "    KEY " COL_ATTRNAME " (" COL_ATTRNAME ")"
		  << ");";
		TRACESQL(query);
		res.purge(); res = query.store(); 

		openRoot(); // create the root entry
	}
	catch(...)
	{
		delete mConn;
		mConn =NULL;
		ret =false;
	}

	mUrl = url;

	if (ret) _nInstance_in_EDM++;

	return ret;
}

bool EDBMySQL::isDBConnected()
{
	return (mConn !=NULL);
}

void EDBMySQL::disconnectDB()
{
	if (isDBConnected())
	{
		if (mConn!=NULL)
			delete mConn;
		_nInstance_in_EDM--;
	}
}

void EDBMySQL::free()
{
	delete this;
}

const char* EDBMySQL::getDBURL()
{
	return mUrl.c_str();
}

bool EDBMySQL::openEntry(const char* e, bool creatIfNotExist)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL || strlen(e) <=0)
		return false;

	std::string entry =e;
	
	if (crntentry==e)
		return true;

	mAttrs.clear();

	try
	{
		Query query = mConn->query();
		Result res;

		query << "SELECT " TBLCOL(TBL_ENTRY, COL_EID) "," TBLCOL(TBL_ENTRY, COL_ENTRY)
			  << " FROM " TBL_ENTRY
			  << " WHERE " TBLCOL(TBL_ENTRY, COL_ENTRY) "='" << entry <<"'";

		TRACESQL(query);
		res.purge(); res = query.store(); 

		if (res.begin() != res.end())
		{
			Row row = *res.begin(); 
			crntentry = row[1];
			crnteid = row[0];

			query << "SELECT " TBLCOL(TBL_ATTRIBUTE, COL_ATTRNAME) "," TBLCOL(TBL_ATTRIBUTE, COL_ATTRVALUE)
			  << " FROM " TBL_ENTRY ", " TBL_ATTRIBUTE
			  << " WHERE " TBLCOL(TBL_ENTRY, COL_EID) "=" TBLCOL(TBL_ATTRIBUTE, COL_EID)
			  << "   AND " TBLCOL(TBL_ENTRY, COL_ENTRY) "='" << entry <<"'";
			TRACESQL(query);
			res.purge(); res = query.store(); 

			// The Result class has a read-only Random Access Iterator 
			for (Result::iterator i = res.begin(); i != res.end(); i++)
			{ 
				Row row = *i; 
				mAttrs[row[0].c_str()]=row[1];
			}

			return true;
		} 
	}
	catch (...)
	{
		return false;
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

	mAttrs.clear(); // for opened parent
	try
	{
		Query query = mConn->query();
		Result res;

		query << "INSERT INTO " TBL_ENTRY
			  << "   SET " TBLCOL(TBL_ENTRY, COL_ENTRY) "='" << entry <<"'";
		TRACESQL(query);
		res.purge(); res = query.store(); 

		return openEntry(entry.c_str());
	}
	catch (...)
	{
		return false;
	}
}

bool EDBMySQL::copyEntry(const char* eto, const char* efrom, bool overwrite)
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

bool EDBMySQL::openRoot()
{
	return openEntry(LOGIC_SEPS, true);
}

bool EDBMySQL::openParent()
{
	if (crntentry.length() <=2)
		return false;

	int pos=crntentry.find_last_of(LOGIC_SEPC);
	if (pos <1)
		return false;

	return openEntry(crntentry.substr(0, pos).c_str());
}

bool EDBMySQL::openNextSibling()
{
	ASSET_BOOL(isDBConnected());

	std::string entry2backup = crntentry;

	int pos =crntentry.find_last_of(LOGIC_SEPC);
	if (pos<1)	return false; // root doesn't have Sibling

	std::string entry2try = crntentry + LOGIC_SEPS "~"; // append with a small ascii

	bool succ = openFirstGE(entry2try.c_str());
	succ = succ && (entry2backup != crntentry)
		  && (entry2backup.compare(0, pos+1, crntentry, 0, pos+1) ==0);

	if (!succ)
		openEntry(entry2backup.c_str());

	return succ;
}

bool EDBMySQL::openFirstChild()
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

bool EDBMySQL::openFirstGE(const char* e)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL || *e==0x00)
		return false;

	int ret = -1;
	mAttrs.clear();
	try
	{
		Query query = mConn->query();

		query << "SELECT " TBLCOL(TBL_ENTRY, COL_EID) "," TBLCOL(TBL_ENTRY, COL_ENTRY)
			  << " FROM " TBL_ENTRY
			  << " WHERE " TBLCOL(TBL_ENTRY, COL_ENTRY) ">='" << e <<"'";

		TRACESQL(query);
		mRes.purge(); mRes = query.store();
		mCursor = mRes.begin();

		if (mCursor != mRes.end())
		{
			Row row = *mCursor; 
			crntentry = row[1];
			crnteid = row[0];
			query << "SELECT " TBLCOL(TBL_ATTRIBUTE, COL_ATTRNAME) "," TBLCOL(TBL_ATTRIBUTE, COL_ATTRVALUE)
			  << " FROM " TBL_ATTRIBUTE
			  << " WHERE " TBLCOL(TBL_ATTRIBUTE, COL_EID) "=" << crnteid;
			TRACESQL(query);

			Result res;
			res.purge(); res = query.store(); 

			// The Result class has a read-only Random Access Iterator 
			for (Result::iterator i = res.begin(); i != res.end(); i++)
			{ 
				Row row = *i; 
				mAttrs[row[0].c_str()]=row[1];
			}

			return true;
		} 
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool EDBMySQL::openNext()
{
	ASSET_BOOL(isDBConnected());

	if (mRes.begin()==mRes.end() || (++mCursor) == mRes.end())
	{
		mRes.purge();
//		mCursor =(Result::iterator)NULL;
		return false;
	}

	mAttrs.clear();
	try
	{
		Query query = mConn->query();
		Row row = *mCursor; 
		crntentry = row[1];
		crnteid = row[0];
		query << "SELECT " TBLCOL(TBL_ATTRIBUTE, COL_ATTRNAME) "," TBLCOL(TBL_ATTRIBUTE, COL_ATTRVALUE)
			<< " FROM " TBL_ATTRIBUTE
			<< " WHERE " TBLCOL(TBL_ENTRY, COL_EID) "=" << crnteid;
		TRACESQL(query);
		
		Result res;
		res.purge(); res = query.store(); 
		
		// The Result class has a read-only Random Access Iterator 
		for (Result::iterator i = res.begin(); i != res.end(); i++)
		{ 
			Row row = *i; 
			mAttrs[row[0].c_str()]=row[1];
		}
		
		return true;
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool EDBMySQL::setReference(const char* e)
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

bool EDBMySQL::commitChanges()
{
	ASSET_BOOL(isDBConnected());

	try
	{
		Query query = mConn->query();

		query << "DELETE FROM " TBL_ATTRIBUTE 
			<< " WHERE " TBLCOL(TBL_ATTRIBUTE, COL_EID) "=" << crnteid;
		TRACESQL(query);
		
		Result res;
		res.purge(); res = query.store(); 
	}
	catch (...) {}
		
	try
	{
		Query query = mConn->query();
		attrs_t::iterator i;
		for (i=mAttrs.begin(); i !=mAttrs.end(); i++)
		{
			query.reset();
			query << "INSERT INTO " TBL_ATTRIBUTE 
				  << "  SET " COL_EID "=" << crnteid << ", "
				  <<        COL_ATTRNAME "='" <<i->first << "', "
				  <<        COL_ATTRVALUE "='" <<i->second << "' ";
			TRACESQL(query);
			Result res;
			res.purge(); res = query.store(); 
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool EDBMySQL::deleteEntry(const char* e)
{
	ASSET_BOOL(isDBConnected());

	if (e==NULL)
		e=crntentry.c_str();

	if (strlen(e)<2)
		return false; // cann't del the root;

	std::string crnt2backup = crntentry;
	std::string crnt2delete = e;
	std::string eid2del="-1";

	if (crnt2backup.compare(crnt2delete) !=0 && !openEntry(crnt2delete.c_str()))
	{
		openEntry(crnt2backup.c_str());
		return false;
	}

	eid2del = crnteid;

	for(bool succ = openFirstChild();
		succ; succ = succ && openNextSibling())
	{
		if (!deleteEntry(NULL))
			return false;
	}

	try
	{
		Query query = mConn->query();
		
		query << "DELETE FROM " TBL_ATTRIBUTE 
			<< " WHERE " TBLCOL(TBL_ATTRIBUTE, COL_EID) "=" << eid2del;
		
		TRACESQL(query);
		Result res;
		res.purge(); res = query.store(); 
	}
	catch (...) {}
		
	try
	{
		Query query = mConn->query();
		query << "DELETE FROM " TBL_ENTRY
			<< " WHERE " TBLCOL(TBL_ENTRY, COL_EID) "=" << eid2del;
		TRACESQL(query);
		Result res;
		res.purge(); res = query.store(); 
	}
	catch (...)
	{
		return false;
	}

	if (crnt2backup.compare(crnt2delete) !=0)
		openEntry(crnt2backup.c_str());

	return true;
}

const char* EDBMySQL::getCurrentEntry()
{
	return crntentry.c_str();
}

// const char* EDBMySQL::getRealEntry()
// {
// 	return realentry.c_str();
// }

const char* EDBMySQL::getAttrName(const int index)
{
	int j =index;
	for (attrs_t::iterator i = mAttrs.begin(); i!= mAttrs.end() && j>0; i++, j--)
		;
	return (i== mAttrs.end()) ? NULL : i->first.c_str();
}

const char* EDBMySQL::getAttribute(const char* attrname)
{
	if (attrname ==NULL)
		return NULL;
	std::string v=mAttrs[attrname];
	return mAttrs[attrname].c_str();
}

bool EDBMySQL::setAttribute(const char* attrname, const char*value)
{
	std::string n=(attrname ==NULL)? "": attrname, v =(value ==NULL)? "":value;

	if (n.empty())
		return false;

	mAttrs[n]=v;

	return true;
}

bool EDBMySQL::isReference()
{
	return (mAttrs[ENTRY_TYPE].compare("ref") ==0);
}
	
bool EDBMySQL::associateReference()
{
	if (!isReference())
		return true;

	//TODO: implement later
	return false;
}

ENTRYDB_NAMESPACE_END
