#ifndef _cdnss_c2streamer_c2sessionstatdump_h__
#define _cdnss_c2streamer_c2sessionstatdump_h__
#include <vector>
#include <fstream>
#include <map>
#include <Locks.h>
#include <string>
#include <NativeThread.h>
#include "C2Session.h"
namespace C2Streamer
{
class C2StreamerEnv;
class C2Service;

class SessLogMgr : public ZQ::common::NativeThread
{
	public:
		struct SessLogName
		{
			std::string   logname;
			int           no;
			bool  operator() (const SessLogName& n1, const SessLogName& n2)
			{
				return n1.no < n2.no;
			}
		};
	private:
		C2StreamerEnv&      mEnv;
		C2Service&          mSvc;
		int                 mMaxfilenum;
		int                 mLoopInterval;
		uint64              mSessexpire;
		std::string         mFileDir;
		std::string         mFileName;
		std::string         mSrcFileName;  //symbol link source file
		typedef std::map<std::string,C2Session::StatInfo>  SessInfo;
		SessInfo            mSessInfo;
		std::ofstream       mfileStream;
		ZQ::common::Mutex   mLocker;
		bool                mbQuit;

	private:
		int        open();
		void       CreateFileVec(std::vector<SessLogName>&);
		void       RemoveAndRenameFile();
		void       createDir();
		void       writeMessage();
		int        run();
		void       RemoveExpireSess();
	public:
		SessLogMgr(C2StreamerEnv& env,C2Service& svc);
		~SessLogMgr();
		bool        start(std::string sessfilename = "sess", int maxsesslognum = 5, int loopInterval = 3, int sessexpire = 3000, std::string filedirectory = "./sess");
		void		stop();
		void        UpdateMsg(const std::string& sessId, C2Session::StatInfo& info);
		void        EraseSess(const std::string& sessId);
};




}


































#endif
