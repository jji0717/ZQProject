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
// Desc  : EntryDB implementation by using expatxx
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/ExpatDB.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/ExpatDB.cpp $
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
// 2     4/12/05 5:39p Hui.shao
// ============================================================================================

#include "ExpatDB.h"
#include "Expatxx.h"
#include "EDBNil.h"
#include "TimeStamp.h"

ENTRYDB_NAMESPACE_BEGIN

typedef std::stack<std::string > stack_t;

// class ExpatDBNesting
class ExpatDBNesting
	: public ExpatBaseNesting
{
	friend class ExpatDB;
	
private:
	ExpatDB *pExpatDB;
	
	ExpatDBNesting(ExpatDB *pem) { pExpatDB=pem; }
	
	// overrideable callbacks
	virtual void startElement(const XML_Char* name, const XML_Char** atts);
	virtual void endElement(const XML_Char*);
	virtual void charData(const XML_Char*, int len);
	
private:
	
	// Element stack definition
	stack_t m_stack;
	static uint32 elmid;
	std::string content;
};

void ExpatDBNesting::startElement(const XML_Char* name, const XML_Char** atts)
{
	// The path string
	std::string entry = (!m_stack.empty()) ? m_stack.top() : pExpatDB->getCurrentEntry();

	pExpatDB->setContent(content.c_str());
	content = "";

	// Add the new element name to the path
	if (!m_stack.empty())
	{
		entry += LOGIC_SEPS;
		entry += newID();
	}
	
	if (pExpatDB->openEntry(entry.c_str(), true))
		pExpatDB->setAttribute(ELEM_TYPE_ATTR, name);
	else return;

	// Populate the element's attribute list
	for (int n = 0; atts[n]; n += 2)
	{
		if (atts[n]!=NULL)
			pExpatDB->setAttribute(atts[n], atts[n+1]);
	}
	pExpatDB->setAttribute(ELEM_TYPE_ATTR, name);
	
	m_stack.push(entry);

	pExpatDB->bLogicalClosed=false;
}


void ExpatDBNesting::endElement(const XML_Char* name)
{
	m_stack.pop();
	pExpatDB->setContent(content.c_str());

	if (!m_stack.empty())
	{
		pExpatDB->openEntry(m_stack.top().c_str());
		const char* c = pExpatDB->getContent();
		content = (c!=NULL) ?c: "";
	}
	else
		pExpatDB->bLogicalClosed=true;
}

void ExpatDBNesting::charData(const XML_Char* strData, int len)
{
	int pos = 0;
	
	// Append the data to the local string
	std::string local;
	local.append(strData, len);
	
	// Remove extraneous characters
	while (1)
	{
		// Determine the position of the first extraneous char
		pos = local.find_first_of("\r\n\t", 0);
		// Break if no extraneous char exists
		if (pos == -1)  break;
		
		// Remove the extraneous char
		local.erase(pos, 1);
	}
	
	// Return if the string length is 0
	if (local.length() == 0)
		return;

	// trim
	pos = local.find_first_not_of(" ");
	int posend =local.find_last_not_of(" ");

	if (pos>=posend || pos<0)
		return;

	local = local.substr(pos, posend-pos+1);
	content = content.empty() ? local : (content + " " +local);
}


// -----------------------------
// class ExpatDB
// -----------------------------
ExpatDB::ExpatDB(const char *szFilename, bool preInMemory)
         :EDB(), pEMN(NULL), bLogicalClosed(true)
{
	if (preInMemory && !isConnected())
		connect(EDBNILURLPROT "://localhost/");

	if (szFilename!=NULL && *szFilename!=0x00)
		parseXML(szFilename);
}


ExpatDB::ExpatDB(const char *szBuffer, const int nBufferLen, const int nFinal)
         :pEMN(NULL), bLogicalClosed(true)
{
	parseXML(szBuffer, nBufferLen, nFinal);

	if (nFinal)
		delete pEMN;
}

ExpatDB::~ExpatDB()
{
	if (pEMN != NULL)
		delete pEMN;
}

bool ExpatDB::parseXML(const char *szBuffer, const int nBufferLen, const int nFinal)
{
	if (szBuffer==NULL)
		return false;

	if (nBufferLen<=0)
		return true;
	
	if (!isConnected())
		if (!connect(EDBNILURLPROT "://localhost/"))
			return false;

	if (pEMN==NULL)
		pEMN = new ExpatDBNesting(this);

	bLogicalClosed =false;

	// Parse the specified buffer
	// If there is an error, create an error string and throw
	if (!pEMN->XML_Parse(szBuffer, nBufferLen, nFinal))
	{
		char lineno[16];
		itoa(pEMN->XML_GetCurrentLineNumber(), lineno, 10);
		error = std::string("(") + lineno + ") " + pEMN->XML_GetErrorString();
		return false;
	}

	return true;
}

bool ExpatDB::parseXML(const char *szFilename)
{
	char szBuffer[8192];
	bool done = false;
	
	// Open the specified file
	std::ifstream from(szFilename);
	if (from.rdstate())
		return false;
	
	// Read to EOF
	while (!done)
	{
		// Read data from the input file; store the bytes read
		from.read(szBuffer, sizeof(szBuffer));
		
		done = from.eof();
//		if (from.rdstate()) 	// If another error is encountered
//			throw "Could not read data from input file"; 
		
		// Parse the data read
		if (!parseXML(szBuffer, from.gcount(), done))
			return false;
	}

	return true;
}

const char* ExpatDB::getContent()
{
	return EDB::getAttribute(CONTENT_ATTR);
}

bool ExpatDB::setContent(const char *szContent)
{
	if (szContent == NULL || *szContent ==0x00)
		return false;

	return EDB::setAttribute(CONTENT_ATTR, szContent);
}

const char* ExpatDB::getElementName()
{
	return EDB::getAttribute(ELEM_TYPE_ATTR);
}

bool ExpatDB::openFirstChild(char *name)
{
	std::string crnte = getCurrentEntry();
	bool succ =EDB::openFirstChild();

	if (!succ)
	{
		openEntry(crnte.c_str());
		return false;
	}

	const char* cntype =getElementName();

	bool found =false;
	if (name==NULL || *name==0x00)
	{
		if (cntype !=NULL && *cntype !=0x00)
			found = true;
		else found = openNextSibling(false);
	}
	else
	{
		found = (cntype!=NULL) && (strcmp(cntype, name)==0);
		while (!found)
		{
			if (!EDB::openNextSibling())
				break;
			cntype =getElementName();
			found = (cntype!=NULL) && (strcmp(cntype, name)==0);
		}
	}

	succ = succ && found;
	if (!succ)
		openEntry(crnte.c_str());

	return succ;
}

bool ExpatDB::openNextSibling(bool sameElementType)
{
	std::string crnte = getCurrentEntry();
	const char* cntype =getElementName();
	std::string crntTp = (cntype==NULL) ? "" : cntype;

	bool succ =EDB::openNextSibling();

	if (!succ)
	{
		openEntry(crnte.c_str());
		return false;
	}

	cntype =getElementName();
	bool found =false;

	if (!sameElementType || crntTp.empty())
	{
		found = (cntype !=NULL && *cntype !=0x00);

		while (!found)
		{
			if (!EDB::openNextSibling())
				break;

			cntype =getElementName();
			found = (cntype !=NULL && *cntype !=0x00);
		}
	}
	else
	{
		found = (cntype!=NULL) && (crntTp.compare(cntype)==0);
		while (!found)
		{
			if (!EDB::openNextSibling())
				break;

			cntype =getElementName();
			found = (cntype!=NULL) && (crntTp.compare(cntype)==0);
		}
	}

	succ = succ && found;

	if (!succ)
		openEntry(crnte.c_str());

	return succ;
}

// Display the specified element's information and recurse downward
bool ExpatDB::xexp(int depth, std::ostream &out, const char* elyield, const char* atyield)
{
	std::string ey;
	int i;
	for (i=0; i< depth && elyield!=NULL; i++)
		ey +=elyield;

	bool expanded=false, succ=true;

	std::string crnte = getCurrentEntry();
	std::string elemtype;
	{
		const char* et =getElementName();
		elemtype = (et==NULL) ? "": et;
	}

	if (elemtype.empty()) // skip non-xml nodes
		return true;

	const char* c = getContent();
	std::string content= (c==NULL? "" : c);;

	expanded = !content.empty();

	out << ey << "<" << elemtype;
	i =0;
	while (succ)
	{
		const char* attrname = getAttrName(i++);
		if (attrname==NULL)
			break;
		if (strncmp(attrname, HIDDEN_ATTR_PREF, sizeof(HIDDEN_ATTR_PREF) -1) ==0)
			continue;
		out << " " << attrname << "=\"" << getAttribute(attrname) << "\"";
	}
	
	bool hasChildElem = openFirstChild(NULL);

	expanded |= hasChildElem;

	out << (expanded ? ">" : " />") << endl;
	if (!content.empty())
		out << ey << "  " << content << endl;

	for (bool more = hasChildElem; succ && more; more=openNextSibling(false))
		succ = xexp(depth +1, out);

	if (expanded)
		out << ey << "</" << elemtype << ">" << endl;

	openEntry(crnte.c_str());

	return succ;
}

bool ExpatDB::newChild(char *name)
{
	if (!EDB::isConnected() || name==NULL || *name==0x00)
		return false;

	std::string crnte = getCurrentEntry();
	std::string entry = crnte + "/" + newID();

	bool succ =false;
	
	if (EDB::openEntry(entry.c_str(), true))
		succ = EDB::setAttribute(ELEM_TYPE_ATTR, name);

	if (!succ)
		openEntry(crnte.c_str());
	
	return succ;
}

const char* ExpatDB::lastError()
{
	return error.c_str();
}

ENTRYDB_NAMESPACE_END
