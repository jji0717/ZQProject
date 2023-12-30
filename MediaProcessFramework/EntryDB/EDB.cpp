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
// Desc  : generic entry database usage
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDB.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDB.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 14    05-07-28 18:39 Daniel.wang
// 
// 13    05-06-17 1:57p Daniel.wang
// 
// 12    05-06-16 2:12p Daniel.wang
// 
// 11    05-05-10 16:00 Daniel.wang
// 
// 10    05-05-08 16:05 Daniel.wang
// 
// 9     05-04-29 11:38 Daniel.wang
// 
// 8     4/14/05 10:13a Hui.shao
// 
// 6     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 5     4/12/05 5:46p Hui.shao
// ============================================================================================

#include "EDB.h"
#include "EDBNil.h"
#include "Timestamp.h"

#include <fstream>

ENTRYDB_NAMESPACE_BEGIN

EDBFactory defEDBFactory;
EDBFactory* EDB::pFactory = &defEDBFactory;

Transaction::Transaction(ITransaction* pTxn)
:m_pTransaction(pTxn)
{
}

bool Transaction::commit()
{
	if (NULL == m_pTransaction)
		return NULL;

	return m_pTransaction->commit();
}

bool Transaction::abort()
{
	if (NULL == m_pTransaction)
		return NULL;

	return m_pTransaction->abort();
}

EDB::EDB(const char* url)
	:pIEDBImpl(NULL), bDirty(false)
{
	if (url !=NULL)
		connect(url);
}

EDB::~EDB()
{
	disconnect();
}

bool EDB::connect(const char* url)
{
	if (url==NULL || *url==0x00 || pFactory==NULL)
		return false;

	disconnect();
	
	pIEDBImpl = pFactory->Connect(url);	

	bool ret= isConnected();

	if (ret)
	{
		openRoot();
		Timestamp now;
		char buf[32];
		setAttribute(HIDDEN_ATTR_PREF "lastopen", now.hex_str(buf));
	}

	return ret;
}

bool EDB::isConnected()
{
	return (pIEDBImpl!=NULL);
}

void EDB::disconnect()
{
	if (!isConnected())
		return;
	else openRoot();

	pIEDBImpl->free();
	pIEDBImpl = NULL;
}

const char* EDB::URL()
{
	return (isConnected() ? pIEDBImpl->getDBURL() : NULL);
}
	
bool EDB::openEntry(const char* e, bool creatIfNotExist, Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
		bDirty= !pIEDBImpl->commitChanges(pTxn);

	bool succ = !bDirty && pIEDBImpl->openEntry(e, creatIfNotExist, pTxn);

	if (succ)
		bDirty= false;

	return succ;
}

/*
bool EDB::commitChanges()
{
	if (!bDirty)
		return true;

	bool succ = (isConnected() ? pIEDBImpl->commitChanges() : false);
	if (succ)
		bDirty = false;
	return succ;
}
*/

bool EDB::rollbackAttrs(Transaction* txn)
{
	if (!bDirty)
		return true;

	if (!isConnected())
		return false;

	/*
	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;
		*/

	std::string crnte= getCurrentEntry();

	bool succ = openEntry(crnte.c_str(), txn);

	return succ;
}

bool EDB::deleteEntry(const char* e, Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
		bDirty= !pIEDBImpl->commitChanges(pTxn);

	return pIEDBImpl->deleteEntry(e, pTxn);
}

bool EDB::copyEntry(const char* eto, const char* efrom, bool overwrite, Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
		bDirty= !pIEDBImpl->commitChanges(pTxn);

	return pIEDBImpl->copyEntry(eto, efrom, overwrite, pTxn);
}

bool EDB::openRoot(Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
	{
		bDirty= !pIEDBImpl->commitChanges(pTxn);
	}

	bool succ = !bDirty && pIEDBImpl->openRoot(pTxn);

	if (succ)
		bDirty = false;
	return succ;
}

bool EDB::openParent(Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
		bDirty= !pIEDBImpl->commitChanges(pTxn);

	bool succ = !bDirty && pIEDBImpl->openParent(pTxn);

	if (succ)
		bDirty = false;
	return succ;
}

bool EDB::openNextSibling(Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
		bDirty= !pIEDBImpl->commitChanges(pTxn);

	bool succ = !bDirty && pIEDBImpl->openNextSibling(pTxn);

	if (succ)
		bDirty = false;
	return succ;
}

bool EDB::openFirstChild(Transaction* txn)
{
	if (!isConnected())
		return false;

	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	if (bDirty)
		bDirty= !pIEDBImpl->commitChanges(pTxn);

	bool succ = !bDirty && pIEDBImpl->openFirstChild(pTxn);
	if (succ)
		bDirty = false;
	return succ;
}

const char* EDB::getCurrentEntry(bool basename)
{
	const char* e = (isConnected() ? pIEDBImpl->getCurrentEntry() : NULL);
	if (e!=NULL && basename)
		e = strrchr(e, LOGIC_SEPC) +1;
	return e;
}

const char* EDB::getAttrName(const int index, Transaction* txn)
{
	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	return (isConnected() ? pIEDBImpl->getAttrName(index, pTxn) : NULL);
}

const char* EDB::getAttribute(const char* attrname, Transaction* txn)
{
	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	return (isConnected() ? pIEDBImpl->getAttribute(attrname, pTxn) : "");
}

bool EDB::setAttribute(const char* attrname, const char*value, Transaction* txn)
{
	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;

	bool succ = (isConnected() ? pIEDBImpl->setAttribute(attrname, value, pTxn) : false);
	if (succ)
		bDirty = true;
	
	return succ;
}

bool EDB::import(EDB& edb, const char *at, int depth, Transaction* txn)
{
	if (!isConnected() || !edb.isConnected())
		return false;

	/*
	ITransaction* pTxn = NULL;
	if (NULL != txn)
		pTxn = txn->m_pTransaction;
		*/

	std::string crnte= getCurrentEntry();
	std::string crntie= edb.getCurrentEntry();
	std::string e2imp = (at!=NULL) ? at : crnte;
	
	if (e2imp.empty())
		e2imp = crnte;

	if (e2imp[0] != LOGIC_SEPC)
		e2imp = crnte + LOGIC_SEPS + e2imp;
	
	bool succ = false;

	if (edb.openRoot(txn) && (e2imp ==crnte || openEntry(e2imp.c_str(), true, txn)))
	{
		//printf("record -> db");
		succ =copybranch(edb, *this, depth, txn);
	}

	edb.openEntry(crntie.c_str(), txn);
	openEntry(crnte.c_str(), txn);

	return succ;
}

bool EDB::export(EDB& edb, const char *at, int depth, Transaction* txn)
{
	if (!isConnected() || !edb.isConnected())
		return false;

	std::string crnte= getCurrentEntry();
	std::string e2exp = (at!=NULL) ? at : crnte;
	
	if (e2exp.empty())
		e2exp = crnte;

	if (e2exp[0] != LOGIC_SEPC)
		e2exp = crnte + LOGIC_SEPS + e2exp;
	
	bool succ = false;

	if (edb.openRoot(txn) && (e2exp ==crnte || openEntry(e2exp.c_str(), true, txn)))
	{
		//printf("db -> record");
		succ =copybranch(*this, edb, depth, txn);
	}

	edb.openRoot(txn);
	openEntry(crnte.c_str(), txn);

	return succ;
}

bool EDB::copybranch(EDB& from, EDB& to, int depth, Transaction* txn)
{
	int i =0;
	const char* attrname;

	std::string frome= from.getCurrentEntry();
	std::string toe = to.getCurrentEntry();

	for(attrname = from.getAttrName(i); attrname!=NULL; attrname = from.getAttrName(++i))
	{
		//printf("	+key: %s, value: %s, path: %s\n", attrname, from.getAttribute(attrname), toe.c_str());
		to.setAttribute(attrname, from.getAttribute(attrname), txn);
	}

	if (depth != 0)
	{
		for (bool succ = from.openFirstChild(txn); succ; succ=from.openNextSibling(txn))
		{
			std::string be = from.getCurrentEntry(txn);
			be = be.substr(be.find_last_of(LOGIC_SEPC));
			std::string e2mk = toe+ be;
			if(!to.openEntry(e2mk.c_str(), true, txn))
				return false;
			if (!copybranch(from, to, --depth, txn))
				return false;
		}
	}

	from.openEntry(frome.c_str(), txn);
	to.openEntry(toe.c_str(), txn);

	return true;
}

bool EDB::xexport(std::ostream &out, const char *at, Transaction* txn)
{
	if (!isConnected())
		return false;


	std::string crnte= getCurrentEntry(txn);
	std::string e2nav = (at!=NULL) ? at : crnte;
	
	if (e2nav.empty())
		e2nav = crnte;

	if (e2nav[0] != LOGIC_SEPC)
		e2nav = crnte + LOGIC_SEPS + e2nav;

	if (!openEntry(e2nav.c_str()))
		return false;

	bool succ =xexp(0, out, "  ", NULL, txn);

	openEntry(crnte.c_str(), txn);

	return succ;
}

bool EDB::xexport(const char* filename, const char *at, Transaction* txn)
{
	std::ofstream f;
	if (filename !=NULL && *filename!=0x00)
	{
		f.open(filename);
		if (!f.is_open())
			return false;
	}

	bool succ = f.is_open() ? xexport(f, at, txn) : xexport(std::cout, at, txn);

	if (f.is_open())
		f.close();

	return succ;
}

bool EDB::createTxn(Transaction*& txn)
{
	ITransaction* pTxnInst = NULL;
	pIEDBImpl->createTxn(pTxnInst);

	if (NULL == pTxnInst)
		return false;

	txn = new Transaction(pTxnInst);
	return txn != NULL;
}

void EDB::deleteTxn(Transaction*& txn)
{
	if (NULL != txn)
	{
		pIEDBImpl->deleteTxn(txn->m_pTransaction);

		delete txn;
		txn = NULL;
	}
}

bool EDB::xexp(int depth, std::ostream &out, const char* elyield, const char* atyield, Transaction* txn)
{
	std::string ey;
	int i;
	for (i=0; i< depth && elyield!=NULL; i++)
		ey +=elyield;
	std::string e =getCurrentEntry();
	int pos = e.find_last_of(LOGIC_SEPC);
	e = e.substr(pos+1);

	out << ey << "<edb:entry ebid=\"" << e << "\" ";

	i =0;
	
	while (true)
	{
		const char* attrname = getAttrName(i++, txn);
		if (attrname==NULL)
			break;
		out << attrname << "=\"" << getAttribute(attrname, txn) << "\"";
	}

	bool hasChild = openFirstChild(txn);

	out << (hasChild ? ">" : "/>") << endl;

	for (bool more = hasChild; more; more=openNextSibling(txn))
	{
		if (!xexp(depth +1, out))
			return false;
	}

	if (hasChild)
	{
		out << ey << "</edb:entry>" << endl;
		openParent(txn);
	}

	return true;
}

/*
bool EDB::createTransaction(EDBTxn& txn)
{
	if (!isConnected())
		return false;
	
	txn.m_pTxn = pIEDBImpl->createTransaction();
	return NULL != txn.m_pTxn;
}

bool EDB::setTransaction(EDBTxn& txn)
{
	if (!isConnected())
		return false;

	pIEDBImpl->setTransaction((txn.m_pTxn));
	return true;
}

bool EDB::getCurrentTransaction(EDBTxn& txn)
{
	if (!isConnected())
		return false;

	txn.m_pTxn = pIEDBImpl->getTransaction();
	return NULL != txn.m_pTxn;
}

void EDB::delTransaction(EDBTxn& txn)
{
	pIEDBImpl->delTransaction(txn.m_pTxn);
}
*/

int64 EDBAddin::instanceCount(void)
{
	return insCount();
}

// class EDBFactory
EDBFactory::EDBFactory()
           :FactoryModule<EDBAddin>()
{
	init();
}

EDBFactory::EDBFactory(AddinManager& admg)
           :FactoryModule<EDBAddin>(admg)
{
	init();
}

EDBFactory::~EDBFactory()
{
	//to delete addin instance
	for (int i = 0; i < FactoryModule<EDBAddin>::vmods.size(); ++i)
	{
		delete FactoryModule<EDBAddin>::vmods[i];
		FactoryModule<EDBAddin>::vmods[i] = NULL;
	}

	FactoryModule<EDBAddin>::vmods.clear();
}

void EDBFactory::init()
{
	EDBAddin *pEDBAddin = new EDBAddin();

	if (pEDBAddin ==NULL)
		return;

	pEDBAddin->isInternal = true;
		
	pEDBAddin->Connect = EDBNilConnect;
	pEDBAddin->URLProtocol = EDBNilURLProtocol;
	pEDBAddin->URLHelp = EDBNilURLHelp;
	pEDBAddin->EDBType = EDBNilType;
	pEDBAddin->EDBDesc = EDBNilDesc;
	pEDBAddin->insCount= EDBNilCount;

	FactoryModule<EDBAddin>::vmods_lock.enter();
	FactoryModule<EDBAddin>::vmods.push_back(pEDBAddin);
	FactoryModule<EDBAddin>::vmods_lock.leave();
}

IEDB* EDBFactory::Connect(const char *url)
{
	IEDB * retdb=NULL;
	
	FactoryModule<EDBAddin>::vmods_lock.enter();
	for (FactoryModule<EDBAddin>::vmods_t::iterator i = FactoryModule<EDBAddin>::vmods.begin();
	     i <FactoryModule<EDBAddin>::vmods.end(); i++)
	{
		if ((*i)==NULL || !(*i)->isValid())
			continue;

		retdb = (*i)->Connect(url);

		if (retdb != NULL)
			break;
	}
	FactoryModule<EDBAddin>::vmods_lock.leave();
	
	return retdb;
}

const char* EDBFactory::ModuleFile(int i)
{
	return (i>=0 && i< size()) ? (*this)[i]->getImageName():NULL;
}

const char* EDBFactory::ModuleURLHelp(int i)
{
	return (i>=0 && i< size()) ? (*this)[i]->URLHelp(0):NULL;
}

const char* EDBFactory::ModuleDBDesc(int i)
{
	return (i>=0 && i< size()) ? (*this)[i]->EDBDesc(0):NULL;
}

const bool EDBFactory::isModuleInternal(int i)
{
	return (i>=0 && i< size()) ? (*this)[i]->isInternal:false;
}

const int64 EDBFactory::ModuleInstance(int i)
{
	return (i>=0 && i< size()) ? (*this)[i]->instanceCount():false;
}

ENTRYDB_NAMESPACE_END
