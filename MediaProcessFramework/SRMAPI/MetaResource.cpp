
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
// Dev  : Microsoft Developer Studio
// Name  : MetaResource.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : resource in database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaResource.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 45    05-07-29 15:29 Daniel.wang
// 
// 44    05-07-29 15:15 Daniel.wang
// 
// 43    05-07-27 17:33 Daniel.wang
// 
// 42    05-07-26 14:50 Daniel.wang
// 
// 41    05-06-28 5:08p Daniel.wang
// 
// 40    05-06-28 12:04p Daniel.wang
// 
// 39    05-06-28 11:39a Daniel.wang
// 
// 38    05-06-27 10:10a Daniel.wang
// 
// 37    05-06-24 9:12p Daniel.wang
// 
// 36    05-06-24 5:11p Daniel.wang
// 
// 35    05-06-21 10:00p Daniel.wang
// 
// 34    05-06-21 12:19p Daniel.wang
// 
// 33    05-06-15 3:16p Daniel.wang
// 
// 32    05-06-14 6:59p Daniel.wang
// 
// 31    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#include "MetaResource.h"
#include "../MPFException.h"

SRM_BEGIN

ResourceBooker::ResourceBooker(ResourceManager& rm, const char* type)
:m_rm(rm), Booker(type)
{
}

bool ResourceBooker::OnStartAction(size_t time, string resId, const ActionList& subResList)
{
	return m_rm.OnStartAction(time, resId, subResList);
}

void ResourceBooker::OnEndAction(size_t time, string resId, const ActionList& subResList)
{
	m_rm.OnEndAction(time, resId, subResList);
}

BYTE ResourceManager::score(const char* resourceentry, const char* sessionentry)
{
	//do nothing
	return SCORE_0;
}

ResourceManager::ResourceManager(const char* type, size_t leaseterm, bool book, const char* entry)
: RecordManager(utils::NodePath::getSubPath(entry, type).c_str(), RM_PROP_IMMEDIATELY_FLUSH, leaseterm)
{
	m_strTypeEntry	= utils::NodePath::getSubPath(entry, type);
	m_strRootEntry	= entry;

	//m_pBooker		= new ResourceBooker(*this, type);
	m_bBook			= book;
}

ResourceManager::ResourceManager(const char* type)
: RecordManager(utils::NodePath::getSubPath(DB_RESOURCE_ROOT, type).c_str(), RM_PROP_IMMEDIATELY_FLUSH, DEF_RESOURCE_CLEAR_TIME),
m_bBook(false)
{
	m_strRootEntry = DB_RESOURCE_ROOT;
	m_strTypeEntry = utils::NodePath::getSubPath(DB_RESOURCE_ROOT, type);

	//m_pBooker = new ResourceBooker(*this, type);
}

ResourceManager::~ResourceManager()
{
	//delete m_pBooker;
}

bool ResourceManager::OnLeaseTerm(const char* entry, size_t leaseterm)
{
	MetaRecord mr(entry, PM_PROP_READ_ONLY);
	size_t timeout = mr.get(RESOURCE_TIME_OUT_KEY);
	time_t curtime;
	time(&curtime);

	return curtime < timeout;
}

template <typename _ITEM>
struct scoreinfo
{
	_ITEM	item;
	int		score;
};

template <typename _ITEM>
const _ITEM* generateRandomItem(const scoreinfo<_ITEM>* scoreitems, size_t count)
{
	static time_t curtime = 0;
	if (0 == curtime)
		srand((int)time(&curtime));

	int nDecideScore = rand()%scoreitems[count-1].score;

	for (size_t i = 0; i < count; ++i)
	{
		if (scoreitems[i].score > nDecideScore)
			return &scoreitems[i].item;
	}

	return NULL;
}

char* ResourceManager::allocate(const char* sessionentry, char* url, size_t size,
		ActionList& subResList, size_t beginTime, size_t endTime)
{
	if (NULL == url)
	{
		assert(false);
		return NULL;
	}

	ResourceBooker* pBooker = new ResourceBooker(*this, utils::NodePath::getPureName(m_strTypeEntry).c_str());

	Booker::ResList reslist = pBooker->getResList();
	if (m_bBook)
	{
		if (!pBooker->verify(beginTime, endTime, subResList, reslist))
			return NULL;
	}

	scoreinfo<std::string>* pScoreItems = NULL;
	try
	{
		size_t count = reslist.size();

		if (count > 0)
			pScoreItems = new scoreinfo<std::string>[count];

		int nAddUp = 0;
		int i = 0;
		for(Booker::ResItor itor = reslist.begin(); itor != reslist.end(); ++itor, ++i)
		{
			nAddUp += score(utils::NodePath::getSubPath(getEntry(), itor->c_str()).c_str(), sessionentry);
			
			pScoreItems[i].item = itor->c_str();
			pScoreItems[i].score = nAddUp;
		}
		
		if (0 == nAddUp)
		{
			if (count > 0)
			{
				delete[] pScoreItems;
				pScoreItems = NULL;
			}

			delete pBooker;

			return NULL;
		}

		const std::string* genItem = generateRandomItem(pScoreItems, count);

		if (NULL == genItem)
		{
			if (count > 0)
			{
				delete[] pScoreItems;
				pScoreItems = NULL;
			}
			MPFLog(MPFLogHandler::L_WARNING, "[ResourceManager::allocate]\tcan not allocate resource");

			delete pBooker;

			return NULL;
		}
		

		if (m_bBook)
		{
			if (!pBooker->book(beginTime, endTime, *genItem, subResList))
			{
				delete pBooker;
				return NULL;
			}
		}

		std::string strGetItem = utils::NodePath::getSubPath(getEntry(), *genItem);

		if (count > 0)
		{
			delete[] pScoreItems;
			pScoreItems = NULL;
		}

		delete pBooker;

		return compose(strGetItem.c_str(), url, size);

	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_ERROR, "[ResourceManager::allocate]\tget an unknown error, it may be a memory error");
		return NULL;
	}
}

void ResourceManager::free(const char* entry)
{
}

char* ResourceManager::compose(const char* entry, char* url, size_t size)
{
	if (NULL == entry)
	{
		assert(false);
		return NULL;
	}

	if (NULL == url)
	{
		assert(url);
		return NULL;
	}

	MetaRecord node(entry, PM_PROP_READ_ONLY);

	char strNodeIp[MAX_IP_STR_LEN] = {0};
	char strNodePort[MAX_INT32_STR_LEN] = {0};
	node.get(WORKNODE_IP_KEY, strNodeIp, MAX_IP_STR_LEN);
	node.get(WORKNODE_PORT_KEY, strNodePort, MAX_INT32_STR_LEN);
	_snprintf(url, size-1, "%s://%s:%s/%s?%s=%s", URL_PROTOCOL_MPF, strNodeIp, strNodePort,
		URL_PATH_WORKNODE, URL_VARNAME_WORKNODE_ID, utils::NodePath::getPureName(entry).c_str());

	return url;
}

const char* ResourceManager::getRootEntry() const
{
	return m_strRootEntry.c_str();
}

const char* ResourceManager::getTypeEntry() const
{
	return m_strTypeEntry.c_str();
}

MetaResource::MetaResource(const ResourceManager& rm, const char* resid, size_t timeout/*second*/, unsigned int property)
:MetaRecord(MetaRecord(utils::NodePath::getSubPath(rm.getTypeEntry(), resid).c_str(),
			property|PM_PROP_LAST_ACCESS_TIME))
{
	time_t curtime;
	time(&curtime);
	set(RESOURCE_TIME_OUT_KEY, curtime+timeout/1000);
}

MetaResource::MetaResource(const char* rootentry, const char* resid, size_t timeout/*second*/, unsigned int property)
:MetaRecord(MetaRecord(utils::NodePath::getSubPath(rootentry, resid).c_str(),
			property|PM_PROP_LAST_ACCESS_TIME))
{
	time_t curtime;
	time(&curtime);
	set(RESOURCE_TIME_OUT_KEY, curtime+timeout/1000);
}


SRM_END
