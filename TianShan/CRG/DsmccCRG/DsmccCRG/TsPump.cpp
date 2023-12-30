// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: Socket.cpp,v 1.7 2004/07/29 06:25:44 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : a simple TS pumper by monitoring a folder, each file specifies dest
//         of UDP destination
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CRG/DsmccCRG/DsmccCRG/TsPump.cpp $
// 
// 1     8/09/13 2:47p Zonghuan.xiao
// seprated from crm_dsmcc.h for error code
// 
// 1     2/15/12 11:05a Hui.shao
// TsPump designed for ServiceGroup parameter advertizing
// ---------------------------------------------------------------------------

#include "TsPump.h"
extern "C" {
#include <stdio.h>
}

std::string TsPumper::_cmd_Dehex = "ls -l"; // dummy initialization
// -----------------------------
// class TsPumper
// -----------------------------
TsPumper::TsPumper(ZQ::common::InetHostAddress& bindAddr, ZQ::common::tpport_t bport, const char* fileFolder, bool bHex, uint16 interval)
: UDPSocket(bindAddr, bport), _hWakeup(NULL), _bQuit(false), _interval(interval), _bHex(bHex)
{
	_hWakeup = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (_interval < 10 || _interval > MAX_WAIT)
		_interval = 100;

	if (NULL != fileFolder)
		_folderName = fileFolder;
	else _folderName = "." FNSEPS;

	if (FNSEPC != _folderName[_folderName.length() -1])
		_folderName += FNSEPS;
}

TsPumper::~TsPumper()
{
	stop();

	if (_hWakeup != NULL) 
		::CloseHandle(_hWakeup);
	_hWakeup = NULL;
}

void TsPumper::stop()
{
	_bQuit = true;
	::SetEvent(_hWakeup);
	::Sleep(1); 
}

bool TsPumper::readFileToDest(Dest& dest, const std::string& filename)
{
	DBGTRACE("reading file[%s]\n", filename.c_str());
	dest.filename = filename;
	size_t pos = filename.find('_');
	if (std::string::npos == pos)
		return false;
	std::string destIp = filename.substr(++pos);
	pos = destIp.find('_');
	dest.destPort = 0;
	if (std::string::npos == pos)
		return false;

	dest.destPort = atoi(destIp.substr(++pos).c_str());
	destIp = destIp.substr(0,pos-1);
	if (dest.destPort <=0)
		return false;

	dest.destAddr.setAddress(destIp.c_str());
	dest.packets.clear();


	FILE* fin = _bHex ? _popen((_cmd_Dehex + " " + _folderName + filename).c_str(), "r")
		: fopen((_folderName + filename).c_str(), "rb");

	if (NULL == fin)
		return false;

	Packet packet;
	int nRead;
	do {
		memset(packet.bytes, 0xff, sizeof(packet.bytes));
		nRead = fread(packet.bytes, 1, sizeof(packet.bytes), fin);
		if (nRead<=0)
			break;
		dest.packets.push_back(packet);
	} while (nRead >= sizeof(packet.bytes));

	fclose(fin);

	return (dest.packets.size() >0);
}

int64 TsPumper::FileTimeToTime(FILETIME filetime)
{
	unsigned __int64 ltime;
	memcpy(&ltime,&filetime,sizeof(ltime));
	ltime /= 10000;  //convert nsec to msec

	return ltime;
}

int TsPumper::run(void)
{
	DestMap _dests;
	std::queue<WIN32_FIND_DATA> fileList;

	int32 nextSleep = -1;
	int64 stampLastScanFolder =0;

	while (!_bQuit)
	{
		if (nextSleep >0)
		{
			DBGTRACE("sleep %d msec...\n", nextSleep);
			WaitForSingleObject(_hWakeup, nextSleep);
		}

		if (_bQuit)
			break;

		nextSleep = _interval-1;
		int64 stampNow = ZQ::common::now();

		// step 1, scan the dests see if there is any needs to post msg
		{
			//	ZQ::common::MutexGuard g(_lock);
			std::vector<int> SGToDel;
			int64 stampExp = stampNow - (int) (_interval *0.85);
			int64 stampDel = stampNow - (int) (MAX_WAIT*3 +2000);
			for (DestMap::iterator it = _dests.begin(); it !=_dests.end(); it++)
			{
				if (it->second.stampFileSeen < stampDel)
				{
					SGToDel.push_back(it->first);
					continue;
				}

				int64 wait = it->second.stampLastSent - stampExp;
				if (wait >0)
				{
					if (nextSleep > wait)
						nextSleep = (int32) wait;

					continue;
				}

				it->second.stampLastSent = stampNow;

				for (int i=0; i < it->second.packets.size(); i++)
					sendto(it->second.packets[i].bytes, sizeof(it->second.packets[i].bytes), it->second.destAddr, it->second.destPort);
			
				DBGTRACE("sent to SG[%d]\n", it->first);
			}

			for (int i =0; i < SGToDel.size(); i++)
			{
				DBGTRACE("removing SG[%d]\n", SGToDel[i]);
				_dests.erase(SGToDel[i]);
			}
		}

		if (_bQuit)
			break;

		// step 2. scan the folder every 1min see if there is any file changed
		if (fileList.empty())
		{
			if (stampLastScanFolder < stampNow - MAX_WAIT)
			{
				DBGTRACE("listing files\n");
				WIN32_FIND_DATA findData;
				HANDLE hFind = FindFirstFile((_folderName + "SG*." + (_bHex ? "hex":"ts")).c_str(), &findData);

				if (hFind != INVALID_HANDLE_VALUE)
				{
					fileList.push(findData);

					while (FindNextFile(hFind, &findData))
						fileList.push(findData);

					FindClose(hFind);
				}

				stampLastScanFolder = stampNow;
				nextSleep =0;
			}

			continue;
		}

		// step 3. there are some files not processed from lastest list
		nextSleep =0;

		WIN32_FIND_DATA fileData = fileList.front();
		fileList.pop();

		char* fileName = fileData.cFileName;
		int64 stampFileAsOf = FileTimeToTime(fileData.ftLastWriteTime);

		if (strlen(fileName) <=2)
			continue;

		int SGId = atoi(fileName +2);
		if (SGId <=0)
			continue;

		//	ZQ::common::MutexGuard g(_lock);
		if (_bQuit)
			break;

		DestMap::iterator itDest = _dests.find(SGId);
		if (_dests.end() == itDest)
		{
			// new destination
			Dest dest;
			if (!readFileToDest(dest, fileName))
				continue;

			dest.stampLastSent =0;
			dest.stampMessageAsOf = stampFileAsOf;
			dest.stampFileSeen = stampNow;

			_dests.insert(DestMap::value_type(SGId, dest));
			continue;
		}

		// found dest pre-exist
		itDest->second.stampFileSeen = stampNow;

		if (0 == itDest->second.filename.compare(fileName) && itDest->second.stampMessageAsOf >= stampFileAsOf)
			continue; // file has no changes

		// file changed
		if (!readFileToDest(itDest->second, fileName))
		{
			_dests.erase(itDest);
			continue;
		}

		itDest->second.stampMessageAsOf = stampFileAsOf;
	}

	return 0;
}

