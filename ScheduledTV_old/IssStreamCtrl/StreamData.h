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
// Ident : $Id: StreamData.cpp,v 1.1 2004/10/07 11:00:00 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : impl the data and data-management of the session between AS and SS
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/IssStreamCtrl/StreamData.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 11    05-01-19 22:02 Ken.qian
// 
// 10    05-01-19 16:49 Ken.qian
// 
// 9     05-01-19 16:26 Ken.qian
// 
// 8     04-12-31 11:46 Bernie.zhao
// added file lock
// 
// 7     04-12-02 20:32 Ken.qian
// 
// 6     04-12-01 17:38 Ken.qian
// 
// 5     04-12-01 11:02 Ken.qian
// 
// 4     04-10-20 10:31 Ken.qian
// 
// 3     04-10-15 11:38 Ken.qian
// 
// 2     04-10-14 18:08 Ken.qian
// 
// 1     04-10-11 17:04 Ken.qian
// 
// Revision 1.1  2004/10/07 11:00:00 Ken Qian
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


#include "IssApi.h"
#include "stv_ssctrl.h"

#define NOTITYPE_NONE	0
#define NOTITYPE_PLAY	1
#define NOTITYPE_PS		2
#define NOTITYPE_STOP	3

#define XML_STRMDATA_COLLECTION			"StreamDataCollection"
#define XML_STRMDATA_ITEM				"StreamDataItem"

#define XML_STRMDATA_ITEM_PURCHASEID	"PurchaseID"
#define XML_STRMDATA_ITEM_SID			"SID"
#define XML_STRMDATA_ITEM_ASSET			"AssetID"
#define XML_STRMDATA_ITEM_PreAEUID		"PreAEUID"
#define XML_STRMDATA_ITEM_NewAEUID		"NewAEUID"

////////////////////////////////////////////////////////////////////////////
//    CStreamData is used to store a stream session's data. The data life 
//    started from the Stream Session was created(CallBack(NewStream)),
//    ended with the stream session was terminated(CallBack(StateChange)).
//    The purchase ID and SID are the primary keys CStreamData's object.
////////////////////////////////////////////////////////////////////////////

class CStreamData  
{
public:
	/// constructor
	/// @param dwPurchaseID - the unique flag to the stream session.
	/// @param sid  - the stream id, corresponding to the purchase id
	/// @param dwAssetID - the asset id of the stream session
	CStreamData(DWORD dwPurchaseID, STREAMID sid, DWORD dwAssetID, 
				DWORD dwPreAEUID=0, DWORD dwNewAEUID=0);

	/// default constructor 
	CStreamData() {};

	/// copy constructor
	CStreamData(const CStreamData& rhs);

	/// destructor
	virtual ~CStreamData();

	/// override of operation = 
	const CStreamData& operator=(const CStreamData& rhs);

public:
	/// get and set functions: implement the operation to member variables

	DWORD GetPurchaseID(void) { return m_dwPurchaseID; }
	void  SetPurchaseID(DWORD dwPurchaseID) { m_dwPurchaseID = dwPurchaseID; }

	DWORD GetAssetID(void) { return m_dwAssetID; }
	void  SetAssetID(DWORD dwAssetID) { m_dwAssetID = dwAssetID; }
	
	const PSTREAMID GetStreamID(void) { return &m_sid; }
	void  SetStreamID(const STREAMID& sid) { memcpy(&m_sid, &sid, sizeof(m_sid)); }

	DWORD GetPreAEUID(void) { return m_dwPreAEUID; }
	void  SetPreAEUID(DWORD dwPreAEUID) { m_dwPreAEUID = dwPreAEUID; }
	
	DWORD GetNewAEUID(void) { return m_dwNewAEUID; }
	void  SetNewAEUID(DWORD dwNewAEUID) { m_dwNewAEUID = dwNewAEUID; }

	DWORD GetXitionSeqNum() { return m_dwXitionSeqNum; };
	void  SetXitionSeqNum(DWORD dwSeqNum) { m_dwXitionSeqNum = dwSeqNum; };

	/// save Asset Element ID
	void AddAEID(DWORD dwAEID);
	/// Does the specified Asset Element existed in the array.
	BOOL  ExistedAEID(DWORD dwAEID);
	/// Clear AEID from m_arrAEID
	void ClearAEIDs(void);

public:
	/// Read StreamData from XML reference
	/// @param itemIpref - the preference of the current StreamData.
	/// @return - the result of reading operation.
	BOOL ReadFromXML(ZQ::common::IPreference* itemIpref);
	/// Write StreamData to XML reference
	/// @param itemIpref - the preference of the current StreamData.
	/// @return - the result of writing operation.
	BOOL WriteToXML(ZQ::common::IPreference* itemIpref);

private:
	// the following two variables are the KEY
	DWORD    m_dwPurchaseID;	// PurchaseID
	STREAMID m_sid;			    // StreamID	

	DWORD    m_dwAssetID;		// the Asset ID of current stream

	// the following two variables are changing with the stream going on.
	DWORD    m_dwPreAEUID;		// the previous playing Asset Element of the stream
	DWORD    m_dwNewAEUID;		// the new playing Asset Element of the stream

	vector<DWORD> m_arrAEID;

	BOOL m_bHasSch;				// Does we have search for unregular asset element.
	DWORD	m_dwXitionSeqNum;
};

////////////////////////////////////////////////////////////////////////////
//    CStreamDataManager is a container to manage the CStreamData objects. 
////////////////////////////////////////////////////////////////////////////

class CStreamDataManager 
{
public:
	/// constructor
	CStreamDataManager();
	/// destructor
	virtual ~CStreamDataManager();

public:
	/// Add a new stream session data to the container. the PurchaseID of
	///    CStreamData object is the primary key. If the PurchaseID represented
	///	   CStreamData has been existed in the container, replaced it.
	/// @param streamData - the new stream session data object.
	/// @return - the result of add operation.
	BOOL Add(CStreamData& streamData);

	/// Removed the CStreamData object from the container according the stream id.
	/// @sid - stream id of CStreamData object that you want to removed from container.
	BOOL Remove(const STREAMID& sid);

	/// Update the PreAeUID and NewAeUID of the specified CStreamData object accoding to sid
	/// and set the SSAENotification(report the Asset Element's status).
	/// @param sid - the stream id
	/// @param dwPreAEUID - the previous asset element unique ID
	/// @param dwNewAEUID - the new asset element unique ID
	/// @param stopNoti - the stopped asset element SSAENotification data.
	/// @param playNoti - the playing asset element SSAENotification data.
	/// @return - which one or both of SSAENotification(stopNoti, playNoti) were filled out.
	int  GenSSAENotification(const STREAMID& sid, DWORD dwPreAEUID, DWORD dwNewAEUID, DWORD dwXitionSeqMum, 
						   SSAENotification& stopNoti, SSAENotification& playNoti);

	/// Set the SSStreamNotification(report the Stream's status).
	/// @param sid - the stream id
	/// @param errCode - the Error Code of the Stream
	/// @param errOp - the Operation To the Error Stream
	/// @param errNoti - the SSStreamNotification of the ERROR Stream data.
	BOOL GenSSStreamNotification(const STREAMID& sid, DWORD errCode, DWORD errOp, SSStreamNotification& errNoti);

	DWORD GetPurchaseID(const STREAMID& sid);

	/// for save asset element id 
	BOOL AddAEID(const STREAMID& sid, DWORD dwAEID);
	/// Clear the asset element 
	void ClearAEIDs(const STREAMID& sid);

public:
	/// load CStreamData to memory from XML file
	/// @param wchFileName - full path of the xml file
	/// @return - TRUE: load succeed; FALSE: load failed
	BOOL LoadStreamDoc(const wchar_t* wchFileName);

	/// delete CStreamData XML file
	/// @return - TRUE: delete succeed; FALSE: delete failed
	BOOL DeleteStreamDoc(void);

private:
	/// Read CStreamData to memory from XML doc
	/// @return - operation of Restore.
	BOOL Resotre(void);

	/// Save the xml doc from memory to the xml file
	/// @return - operation of Restore.
	BOOL UpdateToFile(void);

	/// Free resource xml doc
	void FreeXmlDoc(void);

	/// Get the IPreference from the xml doc according to the StreamData
	/// @param streamData - the query condition
	/// @return - the IPreference of the query
	ZQ::common::IPreference* FindItemIprefByPchID(CStreamData& streamData);

	/// Add a CStreamData to the xml doc 
	/// @param streamData - the CStreamData object
	/// @return - the result of adding operation
	BOOL AddStreamDataToXML(CStreamData& streamData);
	/// Remove a CStreamData from the xml doc 
	/// @param streamData - the CStreamData object
	/// @return - the result of removing operation
	BOOL RemoveStreamDataFromXML(CStreamData& streamData);


private:
	/// Query the CStreamData objcet by stream ID
	/// @param sid - the stream ID
	/// @return - the iterator of the specified CStreamData objcet.
	list<CStreamData>::iterator QueryBySID(const STREAMID& sid);

	/// Query the CStreamData objcet by Pruchase ID
	/// @param sid - the Purchase ID
	/// @return - the iterator of the specified CStreamData objcet.
	list<CStreamData>::iterator QueryByPurchaseID(DWORD dwPurchaseID);

	/// the list to store the CStreamData objects.
	list<CStreamData> m_lstStreamDatas;

	/// XML doc for save and reload Stream Data: PurchaseID and SID
	ZQ::common::XMLPrefDoc* m_streamDoc;
	ZQ::common::ComInitializer* m_comInit;

	/// xml file path
	wchar_t m_wszStreamDocFileName[MAX_PATH];

	/// lock for xml file
	ZQ::common::Mutex m_xmlLock;
};

#endif // _STREAMDATA_H_
