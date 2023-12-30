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
// $Log: /ZQProjs/TianShan/CRG/DsmccCRG/TSPumpService/TSPump.cpp $
// 
// 8     6/18/14 11:32a Zhiqiang.niu
// add ZeroMemory for next file
// 
// 7     4/03/14 4:53p Li.huang
// 
// 6     4/03/14 4:34p Li.huang
// 
// 5     3/31/14 11:22a Build
// 
// 4     3/27/14 10:45a Li.huang
// add linux build
// 
// 3     3/24/14 11:41a Li.huang
// 
// 2     3/24/14 11:37a Li.huang
// fix bug 18871
// 
// 7     3/12/14 11:09a Li.huang
// add scan subfolder depth
// 
// 6     5/09/12 4:31p Li.huang
// 
// 5     4/06/12 11:15a Li.huang
// 
// 4     4/03/12 4:31p Li.huang
// 
// 1     2/15/12 11:05a Hui.shao
// TsPump designed for ServiceGroup parameter advertizing
// ---------------------------------------------------------------------------
#include "TsPump.h"
#include <RuleEngine.h>
#include "Log.h"
using namespace std;
extern "C" {
#include <stdio.h>
}
#ifdef ZQ_OS_MSWIN
#include "zqappshell.h"
#endif
#define WAITDELAY 10
std::string TsPumper::_cmd_Dehex = "ls -l"; // dummy initialization

#ifdef ZQ_OS_MSWIN
#define POPEN _popen
#elif defined ZQ_OS_LINUX
#define POPEN popen
#endif
// -----------------------------
// class TsPumper
// -----------------------------
TsPumper::TsPumper(ZQ::common::InetHostAddress& bindAddr, 
						ZQ::common::tpport_t bport,char* folderName,bool bHex, uint16 interval,int scanFolderInterval,int subFolderDepth): UDPSocket(bindAddr, bport),
							_folderName(folderName),_bHex(bHex),/*_hWakeup(NULL),*/
								_bQuit(false), _interval(interval),_scanInterval(scanFolderInterval),_subFolderDepth(subFolderDepth)
{
//	_hWakeup = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (_interval < 10 || _interval > MAX_WAIT)
		_interval = 100;

}

TsPumper::~TsPumper()
{
	stop();

//	if (_hWakeup != NULL) 
//		::CloseHandle(_hWakeup);
//	_hWakeup = NULL;
}

void TsPumper::stop()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "TsPumper stop() enter"));

	_bQuit = true;
	_cond.signal();
//	::SetEvent(_hWakeup);
	//::Sleep(10); 

	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "TsPumper stop() leave"));
}

std::string ScanFolder::getfilename(std::string filePath)
{		
	std::string filename = strrchr(filePath.c_str(), FNSEPC);
	filename = filename.substr(1);

	return filename;
}

static bool compare(FileInfo file1, FileInfo file2)
{
	return (file1.FilePath.compare(file2.FilePath));
}

#ifdef ZQ_OS_MSWIN

int64 TsPumper::FileTimeToTime(FILETIME filetime)
{
	unsigned __int64 ltime;
	memcpy(&ltime,&filetime,sizeof(ltime));
	ltime /= 10000;  //convert nsec to msec

	return ltime;
}
#else
int64 TsPumper::FileTimeToTime(time_t time)
{
	int64 ltime;
	ltime = (int64)time;
	ltime *= 1000;  //convert sec to msec

	return ltime;
}
#endif

void ScanFolder::listFiles(const std::string folder, std::list<FileInfo>& filelist, int depth)
{
	bool bFlag = true;
	std::string filePathName = folder;
	std::string subFolder = "";
	std::string filename = "";

	std::string suffix = (_bHex ? ".hex":".ts");

#ifdef ZQ_OS_MSWIN
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));

	HANDLE hFind;

	filePathName.append("*");
	hFind = FindFirstFile(filePathName.c_str(), &fd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "unavailable handle"));
		return;
	}
	filename = fd.cFileName;
#else
	DIR *dir;
	struct dirent *fd;

	if((dir = opendir(filePathName.c_str())) == NULL)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "unavailable handle"));
		return;
	}
	bFlag = ((fd=readdir(dir)) != NULL);
	filename = fd->d_name;
#endif

	while(bFlag){
		//Is directory
		if(ISDIR(fd)){
			if((depth > 0) && strcmp(filename.c_str(), ".") && strcmp(filename.c_str(), ".."))
			{  
				subFolder = folder; 
				subFolder.append(filename);
				subFolder.append(FNSEPS);
				listFiles(subFolder, filelist, --depth);
			}
		}else if(strcmp(filename.c_str(), ".")	\
			&& strcmp(filename.c_str(), "..") \
			&& (strstr(filename.c_str(), "SG") && (filename == strstr(filename.c_str(), "SG"))) \
			&& (strstr(filename.c_str(), suffix.c_str()) && suffix == strstr(filename.c_str(), suffix.c_str()))){
				FileInfo fileinfo;
				filePathName = folder;
				filePathName.append(filename);
				fileinfo.FilePath = filePathName;
#ifdef	ZQ_OS_MSWIN
				//fileinfo.stampLastSent
				fileinfo.stampFileAsOf = _pumper.FileTimeToTime(fd.ftLastWriteTime);
				filelist.push_back(fileinfo);
				//glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "[listFiles] read file [%s] success !"),filePathName.c_str());
		}
		ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));
		if(!FindNextFile(hFind, &fd)){
			bFlag = false;
		}else{
			filename = fd.cFileName;
		}	

#else
				struct stat st;
				if(stat(filePathName.c_str(), &st) != 0){
					continue;
				}

				fileinfo.stampFileAsOf = _pumper.FileTimeToTime(st.st_mtime);
				filelist.push_back(fileinfo);
				//glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "[listFiles] read file [%s] success !"),filePathName.c_str());
		}

		if ((fd=readdir(dir)) == NULL){
			bFlag = false;
		}else{
			filename = fd->d_name;
		}

#endif

	}

#ifdef ZQ_OS_MSWIN
	FindClose(hFind);
#else
	closedir(dir);
#endif
}

//send packets to destination and remove which is timeout
int TsPumper::run(void)
{
	ScanFolder s(*this,_folderName,_bHex, _subFolderDepth);
	s.start();
	int nextSleep =0;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "TsPumper run() enter"));

	while (!_bQuit)
	{
		//indicate the app alive
#ifdef ZQ_OS_MSWIN
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		SetEvent(handle_array[APPALIVE]);
#else
		struct timeval tvstart;
		struct timeval tvend;
#endif	
		int64 timestamp = 0;
		if (nextSleep >0)
		{
			DBGTRACE("sleep %d msec...\n", nextSleep);


			//replace L_INFO to L_DEBUG level log,you can check them if necessary
			//timestamp = GetTickCount();
#ifdef ZQ_OS_MSWIN
			LARGE_INTEGER timeStart;
			QueryPerformanceCounter(&timeStart);
			//WaitForSingleObject(_hWakeup, nextSleep);
			SYS::sleep(nextSleep);
			LARGE_INTEGER timeEnd;
			QueryPerformanceCounter(&timeEnd);
			//glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "sleep %d msec...finished took %dms"),nextSleep, (int)(GetTickCount() - timestamp));
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "sleep %d msec...finished took %dms"),
				nextSleep, (int)((timeEnd.QuadPart-timeStart.QuadPart)*1000/freq.QuadPart));
#else
			gettimeofday(&tvstart, NULL);
			SYS::sleep(nextSleep);
			gettimeofday(&tvend, NULL);	

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "sleep %d msec...finished took %dms"),
				nextSleep, (int)(tvend.tv_sec-tvstart.tv_sec)*1000+(tvstart.tv_usec-tvend.tv_usec)/1000);
#endif
		}

		if (_bQuit)
			break;

		nextSleep = _interval -1;
		_stampNow = ZQ::common::now(); 

		// step 1, scan the dests see if there is any needs to post msg
		{

			std::vector<int> SGToDel;
			int64 stampExp = _stampNow - (int) (_interval *0.85);
			int64 stampDel = _stampNow - (int) (MAX_WAIT*3 +2000);
			if(_dests.empty())
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "dests map is empty"));
				continue;
			}

			//ZQ::common::MutexGuard g(_lock);
			for (DestMap::iterator it = _dests.begin();it != _dests.end(); it++)
			{
				if (it->second.fileInfo.stampFileSeen < stampDel)
				{
					SGToDel.push_back(it->first);
					continue;
				}

				int64 wait = it->second.fileInfo.stampLastSent - stampExp;
				if (wait > WAITDELAY)
				{
					if (nextSleep > wait)
						nextSleep = (int32) wait;
					continue;
				}

				it->second.fileInfo.stampLastSent = _stampNow;

				for (int i=0; i < it->second.packets.size(); i++)
					sendto(it->second.packets[i].bytes, sizeof(it->second.packets[i].bytes), it->second.fileInfo.destAddr, it->second.fileInfo.destPort);

				DBGTRACE("sent to SG[%d]\n", it->first);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "sent to SG[%d]"),it->first);

			}

			for (int i =0; i < SGToDel.size(); i++)
			{
				DBGTRACE("removing SG[%d]\n", SGToDel[i]);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "removing SG[%d]"),SGToDel[i]);

				ZQ::common::MutexGuard g(_lock);
				_dests.erase(SGToDel[i]);
			}

			if (_bQuit)
				break;

			nextSleep = nextSleep - (ZQ::common::now() - _stampNow) - WAITDELAY;
		}

	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "TsPumper run() leave"));

	return 0;
}


//scan folder and stored the folder information latest
int	ScanFolder::scanFolder(TsPumper::FileInfoMap& fmap)
{
	fmap.clear();
	_pumper._stampNow = ZQ::common::now();
	std::list<FileInfo> fileList;

	listFiles(_folderName, fileList, _pumper._subFolderDepth);

	fileList.sort(compare);

	int lastSGId =-1; 

	while(!fileList.empty())
	{
		//	SetEvent(handle_array[APPALIVE]);
		FileInfo fileInfo = fileList.front();
		fileList.pop_front();

		std::string fileName = getfilename(fileInfo.FilePath);

		if (strlen(fileName.c_str()) <=2)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "the length of filename less than 2"));
			continue;
		}

		int SGId = atoi(fileName.c_str() +2);
		if (SGId <=0)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "the SGID is illegal"));
			continue;
		}

		if (lastSGId == SGId)
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(TsPumper, "multiple TS files exist in the folder for a same serviceGroup[%d]"), SGId);

		lastSGId = SGId;

		//ZQ::common::MutexGuard g(_lock);
		if (_pumper._bQuit)
			break;
		fileInfo.stampLastSent =0;
		fileInfo.stampMessageAsOf = fileInfo.stampFileAsOf;
		fileInfo.stampFileSeen = _pumper._stampNow;		

		fmap.insert(TsPumper::FileInfoMap::value_type(SGId,fileInfo));
	}

	return fmap.size();
}

bool ScanFolder::readFileToDest(TsPumper::Dest& dest, const std::string& filePath)
{
	DBGTRACE("reading file[%s]\n", filename.c_str());

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "reading file[%s]"),filePath.c_str());

	dest.fileInfo.FilePath = filePath;
	std::string filename = getfilename(filePath);

	size_t pos = filename.find('_');
	if (std::string::npos == pos)
		return false;
	std::string destIp = filename.substr(++pos);

	pos = destIp.find('_');
	dest.fileInfo.destPort = 0;
	if (std::string::npos == pos)
		return false;

	dest.fileInfo.destPort = atoi(destIp.substr(++pos).c_str());
	destIp = destIp.substr(0,pos-1);
	if ((dest.fileInfo.destPort) <=0)
		return false;

	dest.fileInfo.destAddr.setAddress(destIp.c_str());
	dest.packets.clear();
	FILE* fin = _bHex ? POPEN((_pumper._cmd_Dehex + " " + filePath).c_str(), "r")
		: fopen(filePath.c_str(), "rb");

	if (NULL == fin)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TsPumper, "open file error"));
		return false;
	}

	TsPumper::Packet packet;
	int nRead;
	do {
		memset(packet.bytes, 0xff, sizeof(packet.bytes));
		nRead = fread(packet.bytes, 1, sizeof(packet.bytes), fin);
		if (nRead<=0)
			break;
		//glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "read file [%s], read string length is: %d"), filePath.c_str(), nRead);
		dest.packets.push_back(packet);
	} while (nRead >= sizeof(packet.bytes));

	fclose(fin);
	return (dest.packets.size() >0);
}

//compare between old file list and the latest,see if there are any changes
int ScanFolder::run(void)
{
	int64 stampLastScanFolder =0;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "ScanFolder run() enter"));
	while(!_pumper._bQuit)
	{
		TsPumper::FileInfoMap	oldFileInfoMap;
		{
			ZQ::common::MutexGuard g(_pumper._lock);
			for(TsPumper::DestMap::iterator ite = _pumper._dests.begin();ite != _pumper._dests.end();ite++)
			{
				STL_MAPSET(TsPumper::FileInfoMap, oldFileInfoMap, ite->first, ite->second.fileInfo);
			}
		}

		TsPumper::FileInfoMap currentFileMap;
		scanFolder(currentFileMap);
		int64 _stampNow = ZQ::common::now();

		for(TsPumper::FileInfoMap::iterator it = currentFileMap.begin();it != currentFileMap.end();it++)
		{
			std::string fileName = getfilename(it->second.FilePath);
			int64 SGId = atoi(fileName.c_str() + 2);
			TsPumper::FileInfoMap::iterator itOldFile = oldFileInfoMap.find(SGId);

			if(oldFileInfoMap.end() == itOldFile)
			{
				//new destination
				TsPumper::Dest dest;
				if (!readFileToDest(dest, it->second.FilePath))
					continue;

				dest.fileInfo.stampLastSent =0;
				dest.fileInfo.stampMessageAsOf = it->second.stampFileAsOf;
				dest.fileInfo.stampFileSeen = _stampNow;

				ZQ::common::MutexGuard g(_pumper._lock);
				STL_MAPSET(TsPumper::DestMap, _pumper._dests, SGId, dest);
				continue;
			}


			//found dest pre-exit

			if (0 != itOldFile->second.FilePath.compare(it->second.FilePath) || itOldFile->second.stampFileAsOf < it->second.stampFileAsOf)
			{
				// found file has been ever changed
				TsPumper::Dest dest;
				bool bRead = readFileToDest(dest,it->second.FilePath);

				ZQ::common::MutexGuard g(_pumper._lock);
				if (!bRead)
				{
					_pumper._dests.erase(SGId);
					glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "removing SG[%d]"),SGId);
					continue;
				}

				TsPumper::DestMap::iterator itDest = _pumper._dests.find(SGId);
				if (_pumper._dests.end() !=  _pumper._dests.find(SGId))
					itDest->second = dest;
			}

			// file found no change, just update stampFileSeen
			ZQ::common::MutexGuard g(_pumper._lock);
			TsPumper::DestMap::iterator itDest = _pumper._dests.find(SGId);
			if (_pumper._dests.end() !=  _pumper._dests.find(SGId))
				itDest->second.fileInfo.stampFileSeen = _stampNow;
		}

		SYS::sleep(_pumper._scanInterval);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumper, "ScanFolder run() leave"));
	return 0;	
}


