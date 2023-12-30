// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: AppSite.h,v 1.1 2005/07/15 11:00:00 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : impl the application site configuation data & management 
//
// Revision History: 
// ---------------------------------------------------------------------------
// 
// Revision 1.1  2005/07/14 11:00:00 Ken Qian
//   definition and implemention
//
// ===========================================================================

#ifndef _STREAMDATA_H_
#define _STREAMDATA_H_

#include <list>
#include <vector>
using namespace std;

// common include
#include "XMLPreference.h"
#include "Locks.h"


#define XML_APPLICATION_SITES           "ApplicationSites"
#define XML_APPLICATION_PREFIX          "PrefixLength"
#define XML_APP_SITE                    "AppSite"

#define XML_APP_SITE_NAME               "Name"
#define XML_APP_SITE_ID                 "ID"
#define XML_APP_SITE_WSDL               "WSDL"

////////////////////////////////////////////////////////////////////////////
//    CAppSiteData is used to store a application site data. 
////////////////////////////////////////////////////////////////////////////

class CAppSiteData  
{
public:
	/// constructor
	/// @param strName - the Application site Name/Description.
	/// @param strID   - the Application site ID, it is unique.
	/// @param strWsdl - the Application stie WSDL.
	CAppSiteData(string strName, string strID, string strWSDL);

	/// default constructor 
	CAppSiteData() {};

	/// copy constructor
	CAppSiteData(const CAppSiteData& rhs);

	/// destructor
	virtual ~CAppSiteData();

	/// override of operation = 
	const CAppSiteData& operator=(const CAppSiteData& rhs);

public:
	/// get and set functions: implement the operation to member variables

	string GetName(void) { return m_strName; }
	void  SetName(string strName) { m_strName = strName; }

	string GetID(void) { return m_strID; }
	void  SetID(string strID) { m_strID = strID; }
	
	string GetWSDL(void) { return m_strWSDL; }
	void  SetWSDL(string strWSDL) { m_strWSDL = strWSDL; }
	
public:
	/// Read Application Site data from XML reference
	/// @param itemPrefG - the preference of the current Application Site.
	/// @return - the result of reading operation.
	BOOL ReadFromXML(ZQ::common::PrefGuard& itemPrefG);

private:
	// the following two variables are the KEY
	string   m_strName;	// Name
	string   m_strID;	// ID
	string   m_strWSDL;	// WSDL
};

////////////////////////////////////////////////////////////////////////////
//    CAppSiteDataManager is a container to manage the CAppSiteData objects. 
////////////////////////////////////////////////////////////////////////////

class CAppSiteDataManager 
{
public:
	/// constructor
	CAppSiteDataManager();
	/// destructor
	virtual ~CAppSiteDataManager();

public:
	/// read all the application sites information
	/// @param wchFileName - full path of the xml file
	/// @return - true: load succeed; false: load failed
	bool ReadConfiguration(const wchar_t* wchFileName);
	
	/// Get siteData according to application site id.
	/// @param siteData - the output of application site data object.
	/// @param strID - the application site ID.
	/// @return - the application site data.
	CAppSiteData* GetSiteDataByID(string strID);

	/// Get the Prefix Length
	DWORD GetPrefixLength() { return m_dwPrefixLength; };

private:
	/// Free resource xml doc
	void FreeXmlDoc(void);

private:
	DWORD m_dwPrefixLength;	// the prefix length of AppID in the ticketID

	/// the container of the CAppSiteData objects.
	list<CAppSiteData> m_lstAppSiteDatas;

	ZQ::common::XMLPrefDoc* m_xmlDoc;
	ZQ::common::ComInitializer* m_comInit;
};

#endif // _STREAMDATA_H_
