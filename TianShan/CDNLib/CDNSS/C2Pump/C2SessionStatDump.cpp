#include <C2SessionStatDump.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>
#include <sstream>
#include <algorithm>
#include <TimeUtil.h>
#include "C2StreamerEnv.h"

namespace C2Streamer
{

SessLogMgr::SessLogMgr(C2StreamerEnv& env,C2Service& svc)
	:mEnv(env),
	mSvc(svc),
	mfileStream(NULL)
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessLogMgr,"SessLogMgr begin()"));
}

SessLogMgr::~SessLogMgr()
{
	mSessInfo.clear();
}

bool SessLogMgr::start(std::string sessfilename, int maxsesslognum, int loopInterval, int sessexpire, std::string filedirectory)
{
	mFileName = mEnv.getConfig().sesslogfilename;
	mMaxfilenum = mEnv.getConfig().maxsesslognum;
	mLoopInterval = mEnv.getConfig().sesslogmgrloopInterval;
	mSessexpire = mEnv.getConfig().sesslogmgrsessExpire;
	mFileDir = mEnv.getConfig().sesslogfiledirectory;
	mSrcFileName = mFileName;
	mbQuit = false;
	return ZQ::common::NativeThread::start();
}

int SessLogMgr::run()
{
	while (!mbQuit)
	{
		writeMessage();
		RemoveExpireSess();
		::sleep(mLoopInterval);
	//	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessLogMgr,"run() next loop"));
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessLogMgr,"thread quit"));
	return 1;
}

void SessLogMgr::stop()
{
	mbQuit = true;
}

void SessLogMgr::writeMessage()
{
	open();
	mfileStream<<"SessId,AssetName,Destination,State,ByteOffset,TimeOffset,Duration,Bitrate,speed,OutputDev/OutputIp,OnDemandSession"<<std::endl;
	std::map<std::string,C2Session::StatInfo>::iterator it;
	{
		ZQ::common::MutexGuard gd(mLocker);
		for (it = mSessInfo.begin(); it != mSessInfo.end();++it)
		{
			mfileStream<<it->first<<","<<it->second.assetName<<","<<it->second.destination<<","<<it->second.stat<<","<<it->second.byteOffset<<","<<it->second.timeOffset<<","<<it->second.duration<<","<<it->second.bitrate<<","<<it->second.scale<<","<<it->second.streamingPort<<"/"<<it->second.streamingIP<<","<<it->second.OnDemandSession<<std::endl;
		}
	}
	if (mfileStream.is_open()){
		mfileStream.close();
	}
	RemoveAndRenameFile();
}

void SessLogMgr::CreateFileVec(std::vector<SessLogName>& outvec)
{
	int cflags = REG_EXTENDED;
	int status;
	const size_t nmatch = 1;
	SessLogName sesslogname;
	regmatch_t pmatch[1];
	regex_t reg;
	std::string pattern = "^"+mFileName+"+[0-9]*$";
	regcomp(&reg,pattern.c_str(),cflags);

	struct dirent* dirp;
	DIR* dp;
	if ( (dp = opendir(mFileDir.c_str())) == NULL){
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessLogMgr,"opendir error"));
		return;
	}
	while ( (dirp = readdir(dp)) != NULL)
	{
		if (dirp->d_type == 8)
		{
			status = regexec(&reg,dirp->d_name,nmatch,pmatch,0);
			if (status == 0 && mFileName.compare(dirp->d_name)!=0)
			{
				std::string suffix = dirp->d_name;
				suffix = suffix.substr(strlen("sess"));
				sesslogname.logname = dirp->d_name;
				sesslogname.no = atoi(suffix.c_str());
				outvec.push_back(sesslogname);
			//	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(CreateFileVec,"match sess log file[%s]"),sesslogname.logname.c_str());
			}
		}
	}
	regfree(&reg);
	closedir(dp);
	std::sort(outvec.begin(),outvec.end(),SessLogName());
}

void SessLogMgr::RemoveAndRenameFile()
{
	std::vector<SessLogName>fileVec;
	CreateFileVec(fileVec);
	std::string idx("0");
	if (!fileVec.empty())
	{
		int index = fileVec.back().no;
		std::stringstream out;
		out<<index+1;
		idx = out.str();
	}
	char newName[128];
	std::string strPrex(mFileDir+"/");
	memset(newName,0,sizeof(newName));
	sprintf(newName,"%s%s%s",strPrex.c_str(),mFileName.c_str(),idx.c_str());
	//remove the oldest sesslog file
	std::vector<SessLogName>::iterator itFile = fileVec.begin();
	for (int fileNum = fileVec.size();itFile != fileVec.end()&& fileNum >= mMaxfilenum; ++itFile, --fileNum)
	{
		std::string fileNameTmp(itFile->logname);
		fileNameTmp = strPrex + fileNameTmp;
		::remove(fileNameTmp.c_str());
	}
	//rename the newest sesslog file
	std::string filename(strPrex+mFileName);
	::rename(filename.c_str(),newName);
	std::string linkname = strPrex +mFileName + "ions.csv";//"file"; sessions.csv
	std::string oldpath = mFileName+idx;
	::remove(linkname.c_str());
	::symlink(oldpath.c_str(),linkname.c_str());
}

void SessLogMgr::createDir()
{
	if (0 != access(mFileDir.c_str(),F_OK)){
		mkdir(mFileDir.c_str(),0777);
	}
}

int SessLogMgr::open()
{
	if (mFileName.empty()){
		mFileName = "sess";
	}
	if (mfileStream != NULL){
		mfileStream.close();
	}
	createDir();
	std::string filename = mFileDir + "/" + mFileName;
//	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessLogMgr,"open() filename[%s]"),filename.c_str());
	mfileStream.open(filename.c_str());
	if (!mfileStream.is_open()){
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessLogMgr,"open() error filename[%s]"),filename.c_str());
	}
	return 0;
}

void SessLogMgr::UpdateMsg(const std::string& sessId,C2Session::StatInfo& info)
{
//	C2Session::StatInfo sess;
	ZQ::common::MutexGuard gd(mLocker);
	int64 now = ZQ::common::TimeUtil::now();
	SessInfo::iterator it = mSessInfo.begin();
	it = mSessInfo.find(sessId);
	if (it != mSessInfo.end())//exist and update
	{
		it->second.duration = info.duration;
		it->second.assetName = info.assetName;
		it->second.byteOffset = info.byteOffset;     //12312;
		it->second.timeOffset = info.timeOffset;   //42353;
		it->second.updatetime = now;
		it->second.destination = info.destination;
		it->second.stat = info.stat;
		it->second.bitrate = info.bitrate;
		it->second.scale = info.scale;
	}
	else //new and insert
	{
		info.updatetime = now;
		mSessInfo.insert(std::make_pair<std::string,C2Session::StatInfo>(sessId,info));
	}
}
//eraseSess when sess destroy
void SessLogMgr::EraseSess(const std::string& sessId)
{
	ZQ::common::MutexGuard gd(mLocker);
	mSessInfo.erase(sessId);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(EraseSess,"erase sess[%s]"),sessId.c_str());
}

void SessLogMgr::RemoveExpireSess()
{
	ZQ::common::MutexGuard gd(mLocker);	
	int64 now = ZQ::common::TimeUtil::now();
	SessInfo::iterator it = mSessInfo.begin();
	for (;it != mSessInfo.end();)
	{
		if( (now - it->second.updatetime) > mSessexpire)
		{
			mSessInfo.erase(it++);
			//MLOG(ZQ::common::Log::L_INFO,CLOGFMT(RemoveExpireSess,"remove expire sess[%s] updatetime[%ld], now[%ld]"),(it->first).c_str(),(it->second).updatetime,now);
		}
		else
		{
			it++;
		}
	}
}




}
