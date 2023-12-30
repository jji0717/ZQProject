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
// Author: Daniel Wang
// Desc  : entry database berkeley database 4 edition
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 16    05-07-29 18:25 Daniel.wang
// 
// 15    05-07-28 18:39 Daniel.wang
// 
// 14    05-05-31 3:08p Daniel.wang
// 
// 13    05-05-31 2:16p Daniel.wang
// 
// 12    05-05-10 16:00 Daniel.wang
// 
// 11    05-05-08 16:05 Daniel.wang
// 
// 10    05-04-29 11:38 Daniel.wang
// 
// 9     05-04-28 22:39 Daniel.wang
// 
// 8     4/14/05 10:13a Hui.shao
// 
// 7     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 6     4/12/05 6:44p Hui.shao
// ============================================================================================

#include "EDBb4.h"
#include "EDBb4Mgr.h"
#include "EDBImpl.h"
#include "urlstr.h"
// #include "Timestamp.h"

ENTRYDB_NAMESPACE_BEGIN


DbEnv EBDB4Env::s_dbenv(0);
size_t EBDB4Env::s_count = 0;

EBDB4Env::EBDB4Env()
{
	++s_count;
	if (1 == s_count)
	{
		s_dbenv.open(NULL, DB_CREATE|DB_INIT_LOCK|DB_INIT_MPOOL|DB_INIT_TXN|DB_RECOVER, 0);
	}
}

EBDB4Env::~EBDB4Env()
{
	if (1 == s_count)
	{
		s_dbenv.close(0);
	}
	--s_count;
}

DbEnv& EBDB4Env::getInst()
{
	return s_dbenv;
}

size_t EBDB4Env::count()
{
	return s_count;
}



TxnEdbb4::TxnEdbb4()
{
	if (0 != env.getInst().txn_begin(NULL, &txn, 0))
		throw "Can not create transaction for edbb4";
}

TxnEdbb4::~TxnEdbb4()
{
	//delete txn;
}

bool TxnEdbb4::commit()
{
	return 0 == txn->commit(0);
}

bool TxnEdbb4::abort()
{
	return 0 == txn->abort();
}

DbTxn*& TxnEdbb4::getInst()
{
	return txn;
}


EDBb4::EDBb4(const char* dbpath, const char*errf, ITransaction* txn)
		 :wk_data(NULL)
{
	mECB.dbp = NULL;
	//mECB.cur = NULL;
	mECB.errf = NULL;
	initworkdata();

	std::string url = EBDB4URLPROT "://localhost/";
	url += (dbpath !=NULL && *dbpath !=0x00) ? dbpath : "ebdb4";

	if (!connectDB(url.c_str(), txn))
		throw EDBException("could not connect to database");
}

EDBb4::EDBb4()
		  :wk_data(NULL)
{
	mECB.dbp = NULL;
	//mECB.cur = NULL;
	mECB.errf = NULL;
	initworkdata();
}

EDBb4::~EDBb4()
{
	disconnectDB();
}

bool EDBb4::connectDB(const char* url, ITransaction* txn)
{
	if (url == NULL || isDBConnected())
		return false;

	std::string protocol, hostname, path;

//	printf("!!![addr:%x]%s\n", (unsigned)url, url);

	URLStr urlstr(url);
	protocol = urlstr.getProtocol();
	hostname = urlstr.getHost();
	path = urlstr.getPath();

	if (protocol.compare(EBDB4URLPROT)!=0 || path.empty())
		return false;

	if (hostname.empty() || (hostname.compare("localhost")!=0 && hostname.compare(0, 4,"127.", 0, 4)!=0))
		return false;

	for (int i=0; i< path.length(); i++)
		if (path[i]=='/') path[i] = FNSEPC;
		
	int dpos =path.find_last_of('.');
	if (dpos<0 || path.find_last_of(FNSEPC) >= dpos)
		path += ".ed4";
		
	mECB.dbpath= path;
		
	bool ret =edb4mgr.connect(mECB);
		
	if (ret)
	{
		mUrl = url;
		openRoot(); // create the root entry
		_nInstance_in_EDM++;
	}
	
	return ret;
}

bool EDBb4::isDBConnected()
{
	return (mECB.dbp !=NULL);
}

void EDBb4::disconnectDB()
{
	_sdels(wk_data);

	if (isDBConnected())
	{
		edb4mgr.disconnect(mECB);
		_nInstance_in_EDM--;
	}
}

void EDBb4::free()
{
	delete this;
}

const char* EDBb4::getDBURL()
{
	return mUrl.c_str();
}

void EDBb4::initworkdata(int size)
{
	_sdels(wk_data);

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

bool EDBb4::openEntry(const char* e, bool creatIfNotExist, ITransaction* txn)
{
	DbTxn* pTxn = NULL;
	if (NULL != txn)
		pTxn = reinterpret_cast<TxnEdbb4*>(txn)->getInst();

	ASSET_BOOL(isDBConnected());

	if (e==NULL || strlen(e) <=0)
		return false;

	std::string entry =e;

	if (crntentry==entry)
		return true;

	int ret =-1;
	try
	{
		Dbc* pCursor = NULL;
		mECB.dbp->cursor(pTxn, &pCursor, 0);

		Dbt Key((void*)entry.c_str(), entry.length()), Data;
		ret = pCursor->get(&Key, &Data, DB_SET);

		pCursor->close();

		if (ret ==0)
		{
			mAttrs.clear();

			crntentry ="" ; //realentry="";
			crntentry.append((const char*)Key.get_data(), Key.get_size());

			initworkdata(Data.get_size());
			memcpy(wk_data, Data.get_data(), Data.get_size());
			populateDBData();

			initworkdata();
		}

		if (ret ==0)
			return true;

		if (!creatIfNotExist)
			return false;

		if (entry.compare(LOGIC_SEPS)!=0)
		{
			// not a root, check if the parent exists
			int pos = entry.find_last_of(LOGIC_SEPC);
			if (std::string::npos == pos)
			{
				return false;
			}

			std::string p = entry.substr(0, pos);
			if (!openEntry(p.c_str()))
			{
				if (!openEntry(p.c_str(), true))
				{
					return false;
				}
			}
		}
		
		mAttrs.clear(); // for opened parent

		//Dbt Key((void*)entry.c_str(), entry.length()), Data;
		u_int32_t flags = 0;
		ret = mECB.dbp->put(pTxn, &Key, &Data, flags);

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

bool EDBb4::copyEntry(const char* eto, const char* efrom, bool overwrite, ITransaction* txn)
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


bool EDBb4::openRoot(ITransaction* txn)
{
	return openEntry(LOGIC_SEPS, true);
}

bool EDBb4::openParent(ITransaction* txn)
{
	if (crntentry.length() <=2)
		return false;

	int pos=crntentry.find_last_of(LOGIC_SEPC);
	if (pos <1)
		return false;

	return openEntry(crntentry.substr(0, pos).c_str());
}

bool EDBb4::openNextSibling(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	std::string entry4backup = crntentry;

	int pos =crntentry.find_last_of(LOGIC_SEPC);
	if (pos<1)	return false; // root doesn't have Sibling

	std::string entry4try = crntentry + LOGIC_SEPS "~"; // append with a small ascii

	bool succ = openFirstGE(entry4try.c_str());
	succ = succ && (entry4backup.compare(0, pos+1, crntentry, 0, pos+1) ==0);

	if (!succ)
		openEntry(entry4backup.c_str());

	return succ;
}

bool EDBb4::openFirstChild(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	std::string crnt4backup = crntentry;
	std::string entry4try = crntentry;
	int pos =entry4try.find_last_of(LOGIC_SEPC);
	if (pos == std::string::npos || pos<1)
		entry4try = LOGIC_SEPS;

	entry4try += LOGIC_SEPS;
	pos =entry4try.find_last_of(LOGIC_SEPC);
	
	bool ret = openFirstGE((entry4try+"!").c_str());
	ret = ret && (entry4try.compare(0, pos+1, crntentry,0, pos+1) ==0);

	if (!ret)
		openEntry(crnt4backup.c_str());

	return ret;
}

bool EDBb4::openFirstGE(const char* e, ITransaction* txn)
{
	DbTxn* pTxn = NULL;
	if (NULL != txn)
		pTxn = reinterpret_cast<TxnEdbb4*>(txn)->getInst();

	ASSET_BOOL(isDBConnected());

	if (e==NULL || *e==0x00)
		return false;

	int ret = -1;
//	mAttrs.clear();
	try
	{
		Dbc* pCursor = NULL;
		mECB.dbp->cursor(pTxn, &pCursor, 0);

		Dbt Key((void*)e, strlen(e)), Data;
		ret = pCursor->get(&Key, &Data, DB_SET_RANGE);

		pCursor->close();

		if (ret ==0)
		{
			mAttrs.clear();
			crntentry =""; // realentry="";
			crntentry.append((const char*)Key.get_data(), Key.get_size());

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

bool EDBb4::openNext(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	DbTxn* pTxn = NULL;
	if (NULL != txn)
		pTxn = reinterpret_cast<TxnEdbb4*>(txn)->getInst();

	int ret = -1;
	mAttrs.clear();
	try
	{
		Dbc* pCursor = NULL;
		mECB.dbp->cursor(pTxn, &pCursor, 0);

		Dbt Key, Data;
		ret = pCursor->get(&Key, &Data, DB_NEXT);

		pCursor->close();

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

bool EDBb4::setReference(const char* e, ITransaction* txn)
{
	if (e==NULL || strlen(e) <=0)
		return false;

	std::string entry4backup = crntentry;

	bool ret = openEntry(e);

	openEntry(entry4backup.c_str());

	if (ret)
	{
		mAttrs.clear();
		setAttribute(ENTRY_TYPE, "ref");
		setAttribute("ref", e);
	}

	return ret;
}

bool EDBb4::commitChanges(ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	DbTxn* pTxn = NULL;
	if (NULL != txn)
		pTxn = reinterpret_cast<TxnEdbb4*>(txn)->getInst();

	int ret =-1;
	try
	{
		attrs4DBData();

		Dbt Key((void*)crntentry.c_str(), crntentry.length()), Data(wk_data, wk_datasize);
		u_int32_t flags = 0; // overwrite
		ret = mECB.dbp->put(pTxn, &Key, &Data, flags);

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

bool EDBb4::deleteEntry(const char* e, ITransaction* txn)
{
	ASSET_BOOL(isDBConnected());

	DbTxn* pTxn = NULL;
	if (NULL != txn)
		pTxn = reinterpret_cast<TxnEdbb4*>(txn)->getInst();

	if (e==NULL)
		e=crntentry.c_str();

	if (strlen(e)<2)
		return false; // cann't del the root;

	std::string crnt4backup = crntentry;
	std::string crnt4delete = e;

	if (crnt4backup.compare(crnt4delete) !=0 && !openEntry(crnt4delete.c_str()))
	{
		openEntry(crnt4backup.c_str());
		return false;
	}

	for(bool succ = openFirstChild();
		succ; succ = succ && openNextSibling())
	{
		if (!deleteEntry(NULL))
			return false;
	}

	int ret =0;
	if (openEntry(crnt4delete.c_str()))
	{
		try
		{
			Dbc* pCursor = NULL;
			mECB.dbp->cursor(pTxn, &pCursor, 0);

			int	ret = pCursor->del(0);
			pCursor->close();
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

	if (crnt4backup.compare(crnt4delete) !=0)
		openEntry(crnt4backup.c_str());
//	else openParent();

	return (ret ==0);
}

const char* EDBb4::getCurrentEntry()
{
	return crntentry.c_str();
}

// const char* EDBb4::getRealEntry()
// {
// 	return realentry.c_str();
// }

const char* EDBb4::getAttrName(const int index, ITransaction* txn)
{
	int j =index;
	for (attrs_t::iterator i = mAttrs.begin(); i!= mAttrs.end() && j>0; i++, j--)
		;
	return (i== mAttrs.end()) ? NULL : i->first.c_str();
}

const char* EDBb4::getAttribute(const char* attrname, ITransaction* txn)
{
	if (attrname ==NULL)
		return NULL;
	std::string v=mAttrs[attrname];
	return mAttrs[attrname].c_str();
}

bool EDBb4::setAttribute(const char* attrname, const char*value, ITransaction* txn)
{
	std::string n=(attrname ==NULL)? "": attrname;
	std::string v =(value ==NULL)? "":value;

	if (n.empty())
		return false;

	mAttrs[n]=v;
	return true;
}

bool EDBb4::isReference()
{
	return (mAttrs[ENTRY_TYPE].compare("ref") ==0);
}
	
bool EDBb4::associateReference()
{
	if (!isReference())
		return true;

	//TODO: implement later
	return false;
}


void EDBb4::createTxn(ITransaction*& txn)
{
	txn = new TxnEdbb4;
}

void EDBb4::deleteTxn(ITransaction*& txn)
{
	if (NULL != txn)
	{
		delete txn;
		txn = NULL;
	}
}

int EDBb4::writeUTF8(char*buf, const char*str)
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

int EDBb4::readUTF8(const char*buf, char*str)
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

void EDBb4::populateDBData()
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

void EDBb4::attrs4DBData()
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

void EDBb4::navigate(ITransaction* txn)
{
	std::string backup = crntentry;

	openRoot();
	navigateN(0);

	openEntry(backup.c_str());

}

void EDBb4::navigateN(int depth, ITransaction* txn)
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

