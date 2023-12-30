#ifndef __SSADMIN_H__
#define __SSADMIN_H__

#include "CmdParser.h"

#include <TsStorage.h>
#include "StreamSmithAdmin.h"

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

using namespace TianShanIce::Streamer;
using namespace TianShanIce::Storage;

namespace {
    long DefaultBW = 5000*1000;
}

class SSAdmin {

public:

    typedef std::map<std::string, std::string> PLAYLISTS;

public:

	SSAdmin();
	SSAdmin(::Ice::CommunicatorPtr ic);
	virtual ~SSAdmin();

public:

    std::string prompt() const;

    void connect(const std::string&);
    void close();

    void help();
    void version();

    void exit();
    bool quit() const;

    void verbose(bool);

    void listPlaylist();
    void listStreamer();
    void listItem();

    void createPlaylist(const std::string&, uint16, const std::string& = "");
    void createPlaylist(int id, int bitrate);
    void createudpPlaylist(const std::string&, uint16, int bitrate=3750000);
    void pushItem(const std::string& name, uint16 id, long long flags = 0, int offsetIn=0, int offsetOut=0, time_t start=0);
    void pushItemWithFlag(const std::string& name, const std::string& flags, int playTimes, int offsetIn=0, int offsetOut=0, time_t start=0);
	void play(long bandwidth=DefaultBW);
    void playEx(float, int64, short);
    void seekItem(uint16 id, int64 offset, short from=1, float speed=0.0f);
    void seek(int64 offset, short from=1);
    void pause();
    void pauseEx();
    void resume();
    void setSpeed(float);
    void setRate(int);
    void erase(uint16);
    void destroy();
    void destroyAll();
    
    void current() const;
    void info();
    void setDestMac(const std::string&);

    void select(const std::string& id);
    void alias(const std::string& guid, const std::string& name);

protected:
	
	void dumpStreamInfo( const TianShanIce::Streamer::StreamInfo& info );

private:
       
    bool checkConnection_();
    bool checkPlaylist_();

    void printState_(StreamState) const;

private:
	Ice::CommunicatorPtr		m_ic;
	StreamSmithAdminPrx			m_AdminPrx;
	PlaylistExPrx				m_plPrx;

    bool quit_;
	bool						m_bShowDetail;

    TianShanIce::Properties prop_;
    PLAYLISTS playlists_;

	std::string					m_lastEffectGuid;
	
    std::string netId_;
    std::string alias_;

	int _lastItemId;
};

#endif 
