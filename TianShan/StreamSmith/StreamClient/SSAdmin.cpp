// SSAdmin.cpp: implementation of the SSAdmin class.
//
//////////////////////////////////////////////////////////////////////

#include "SSAdmin.h"
#include "TianShanDefines.h"
#include <TsTransport.h>
#include <ostream>
#include <time.h>
#include "getopt.h"
#include "SystemUtils.h"
#include <TianShanIceHelper.h>
#include <strHelper.h>
#include "ZQResource.h"


#define TRY_BEGIN \
    try {
#define TRY_END \
    } \
    catch(const TianShanIce::InvalidParameter& e) { \
        std::cerr << e.message << std::endl; \
        return; \
    } \
    catch(const TianShanIce::InvalidStateOfArt& e) { \
        std::cerr << e.message << std::endl; \
        return; \
    } \
    catch(const TianShanIce::ServerError& e) { \
        std::cerr << e.message << std::endl; \
        return; \
    } \
    catch(const TianShanIce::NotSupported& e) { \
        std::cerr << e.message << std::endl; \
        return; \
    } \
    catch(const Ice::Exception& e) { \
        std::cerr << e.ice_name() << std::endl; \
        return; \
    } \
    catch(const std::exception& e) { \
        std::cerr << e.what() << std::endl; \
        return; \
    } \
    catch(...) { \
        std::cerr << "unknown error" << std::endl; \
        return; \
    }

#define	SHOWDETAILSTART() uint64 dwTime=SYS::getTickCount();
#define	SHOWDETAILEND(x) \
    dwTime=SYS::getTickCount()-dwTime; \
    if(m_bShowDetail) { \
    std::cout << #x" ok with time count=" << dwTime << std::endl; \
    }


SSAdmin::SSAdmin():
m_ic(0),
m_AdminPrx(0), 
m_plPrx(0),
quit_(false),
m_bShowDetail(false) {
	_lastItemId = 0;
	int i=0;
	m_ic=Ice::initialize(i, NULL);
	m_bShowDetail = false;
}

SSAdmin::~SSAdmin() {
}


void SSAdmin::version () {
    std::cout << "Console Client for StreamSmith version: " 
        << ZQ_PRODUCT_VER_MAJOR << '.' 
        << ZQ_PRODUCT_VER_MINOR << '.'
        << ZQ_PRODUCT_VER_PATCH << "(build " 
        << ZQ_PRODUCT_VER_BUILD << ")" 
        << std::endl;   
}

void SSAdmin::help() {
// ruler         "================================================================================" 
    version();
	std::cout << "\n"
              << "help                                show this screen\n"
              << "connect <endpoint>                  connect to streamsmith(SS:tcp -h HOST -p PORT)\n"
	          << "close                               disconnect from the streamsmith\n"
	          << "create <IP> <PORT> [alias]          crate a playlist\n"
              << "push <name> <id>                    push item to current playlist\n"
              << "pushf <name> <restriction> <id>     push item to current playlist with restriction\n"
	          << "macaddr <addr>                      set destination macaddress\n"
	          << "list [item|streamer]                show the playlists, items or streamers\n"
	          << "destroy [all]                       destroy current/all playlist(s)\n"
	          << "erase <id>                          erase specified item from current playlist\n"
	          << "seek <offset> [begin|end]           seek current stream\n"
              << "seekitem <id> <offset> [begin|end [speed]] seek specified item\n"
	          << "speed <val>                         set speed of current stream\n"
              << "rate <val>                          set max rate of stream\n"
              << "play [[speed] <offset> <begin|end>] play current stream\n"
              << "pause                               pause current stream\n"
	          << "resume                              resume current stream\n"
	          << "current                             show current context\n"
	          << "info                                get stream information\n"
              << "verbose <true|false>                show execute time\n"
              << "alias <id> <name>                   create or change alias of a playlist\n"
              << "clear                               clear screen\n"
	          << "quit|exit                           quit the program"
              << std::endl;
}

std::string SSAdmin::prompt() const {
	std::string prompt = (netId_.empty() ? "StreamClient" : netId_) + "$" + alias_ + "> ";
    //std::cout << prompt;
	return prompt;
}

void SSAdmin::connect(const std::string& endpoint) {
    std::ostringstream oss;
    oss << endpoint;

    TRY_BEGIN
	m_AdminPrx = StreamSmithAdminPrx::checkedCast(m_ic->stringToProxy(oss.str()));
    netId_ = m_AdminPrx->getNetId();
	TRY_END
	
	if(!m_AdminPrx)	{
        std::cerr << "failed connecting with " << oss.str() << std::endl;
        return;
	}

    std::cout << "connected with (" << oss.str() << ")" << std::endl;
}

void SSAdmin::close() {
    if(!m_AdminPrx) {
        std::cerr << "not connected to any StreamSmith server" << std::endl;
        return;
    }
	m_AdminPrx = 0;
    m_plPrx = 0;
    alias_.clear();

    m_lastEffectGuid.clear();
    netId_.clear();
}


void SSAdmin::listStreamer() {
    if(!checkConnection_()) {
        return;
    }

	TianShanIce::Streamer::StreamerDescriptors strmers;
    TRY_BEGIN
    strmers = m_AdminPrx->listStreamers();
    TRY_END

	for(size_t i = 0; i< strmers.size(); ++i) {
        std::cout << strmers[i].deviceId << std::endl;
	}
}

void SSAdmin::dumpStreamInfo(const TianShanIce::Streamer::StreamInfo& info) {
    std::cout << "ID: " << info.ident.name << std::endl;
    std::cout << "State: "; printState_(info.state);
	
    const TianShanIce::Properties& prop = info.props;
	TianShanIce::Properties::const_iterator it = prop.begin();
	for( ; it != prop.end() ; ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
	}
}

void SSAdmin::pauseEx() {
    if(!checkPlaylist_()) {
        return;
    }

	TianShanIce::StrValues expectValues;
	expectValues.push_back("CURRENTPOS");
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");

	TianShanIce::Streamer::StreamInfo info;

    SHOWDETAILSTART()
	
    TRY_BEGIN
	info = m_plPrx->pauseEx(expectValues);
	dumpStreamInfo(info);
    TRY_END

    SHOWDETAILEND(PauseEx)
}

void SSAdmin::playEx(float speed, int64 offset, short from) {
    if(!checkPlaylist_()) {
        return;
    }

	TianShanIce::StrValues expectValues;
	expectValues.push_back("CURRENTPOS");
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");
	
    SHOWDETAILSTART()

	TianShanIce::Streamer::StreamInfo info;
    TRY_BEGIN
	info = m_plPrx->playEx(speed, offset, from, expectValues);
	dumpStreamInfo(info);
    TRY_END

    SHOWDETAILEND(play);
}

void SSAdmin::play(long bandwidth) {
    if(!checkPlaylist_()) {
        return;
    }

	SHOWDETAILSTART()

    TRY_BEGIN
	PlaylistExPrx prx = PlaylistExPrx::uncheckedCast(m_AdminPrx->openPlaylist(m_lastEffectGuid, std::vector<int>(), false));
	if(!prx) {
        std::cerr << "failed to find playlist " << m_lastEffectGuid << std::endl;
	    return;
	}
	prx->setMuxRate(0, bandwidth, 0);
    prx->commit();
	if(!prx->play()) {
        std::cerr << "failed to play current playlist" << std::endl;
		return;
	}
	TRY_END

    std::cout << "playing (" << m_lastEffectGuid << ")" << std::endl;
    SHOWDETAILEND(play);
}

void SSAdmin::pushItemWithFlag(const std::string& name, const std::string& flagStr, int playtimes, int offsetIn, int offsetOut, time_t start) {
	std::vector<std::string> flags;
	ZQ::common::stringHelper::SplitString(flagStr,flags,"|","| \t\v\r\n\"'");
	std::vector<std::string>::const_iterator it = flags.begin(); 
	long long iFlags = 0;
	for( ; it != flags.end(); it ++ ) {
		if( stricmp(it->c_str(),"NoPause") == 0 ) {
			iFlags |= TianShanIce::Streamer::PLISFlagNoPause;
		} else if(stricmp(it->c_str(),"NoFF") == 0 ) {
			iFlags |= TianShanIce::Streamer::PLISFlagNoFF;
		} else if(stricmp(it->c_str(),"NoREW") == 0 ) {
			iFlags |= TianShanIce::Streamer::PLISFlagNoRew;
		} else if(stricmp(it->c_str(),"NoSEEK") == 0 ) {
			iFlags |= TianShanIce::Streamer::PLISFlagNoSeek;
		} else if(stricmp(it->c_str(),"SkipAtFF") == 0 ) {
			iFlags |= TianShanIce::Streamer::PLISFlagSkipAtFF;
		} else if(stricmp(it->c_str(),"SkipAtFEW") == 0 ) {
			iFlags |= TianShanIce::Streamer::PLISFlagSkipAtRew;
		}
	}
	iFlags |= ((long long)playtimes)<<4;
	pushItem(name, _lastItemId++,iFlags,offsetIn, offsetOut, start);
}
void SSAdmin::pushItem(const std::string& name, uint16 id, long long flags, int offsetIn, int offsetOut, time_t start) {
    if(!checkPlaylist_()) {
        return;
    }
	
	SHOWDETAILSTART();
    
    time_t criticalStart = 0;
    if(start) {
        criticalStart = time(0) + start;
    }

    std::string basename = name;

    PlaylistItemSetupInfo info;
    if(name.size() > 2 && name.find("\\") == 0) {
        TianShanIce::ValueMap& valMap =info.privateData;
        TianShanIce::Variant var;
        var.strs.clear();
        var.strs.push_back(name);
        var.type = TianShanIce::vtStrings;
        valMap["storageLibraryUrl"] = var;

        basename = name.substr(name.find_last_of('\\'));
    }

	info.contentName = basename;
	info.criticalStart = criticalStart;
	info.forceNormal = false;
	info.inTimeOffset = offsetIn;
	info.outTimeOffset = offsetOut;
	info.spliceIn = false;
	info.spliceOut = false;
	info.flags = flags;		
    TRY_BEGIN
	m_plPrx->pushBack(id, info);
    TRY_END

    std::cout << "item (" << name << ") added to (" << m_lastEffectGuid << ")" << std::endl;

    SHOWDETAILEND(pushItem)
}

void SSAdmin::createPlaylist(int id, int bitrate) {
    if(!checkConnection_()) {
        return;
    }

    SHOWDETAILSTART();
	
	TianShanIce::SRM::ResourceMap res;
	TianShanIce::SRM::Resource resGourpID;
	TianShanIce::SRM::Resource resBandWidth;
		
	resGourpID.resourceData["servicegroup"].ints.push_back(id);
	resBandWidth.resourceData["bandwidth"].ints.push_back(bitrate);
	res[TianShanIce::SRM::rtServiceGroup] = resGourpID;
	res[TianShanIce::SRM::rtTsDownstreamBandwidth] = resBandWidth;

    TRY_BEGIN
	::TianShanIce::Properties params;
	m_plPrx=PlaylistExPrx::uncheckedCast(m_AdminPrx->createStreamByResource(res, params));
	
    if(!m_plPrx) {
        std::cerr << "failed to create playlist" << std::endl;
        return;
	}
	m_lastEffectGuid=m_plPrx->getId();
	TRY_END
    
    SHOWDETAILEND(CreatePlaylist);
}

void SSAdmin::createPlaylist(const std::string& addr, uint16 port, const std::string& alias) {
    if(!checkConnection_()) {
        return;
    }

	SHOWDETAILSTART();		

    TRY_BEGIN
	m_plPrx = PlaylistExPrx::uncheckedCast(m_AdminPrx->createStream(0));
	
    if(!m_plPrx) {
        std::cerr << "failed to create playlist" << std::endl;
        return;
	}
	m_plPrx->setDestination(addr, port);
	m_plPrx->setDestMac("11:22:23:a4:45:d6");		
	m_lastEffectGuid = m_plPrx->getId();
    TRY_END
    
    std::string tmp_ = alias;
    if(alias.empty()) {
        std::ostringstream oss;
        oss << (playlists_.size() + 1);
        tmp_ = oss.str();
    }

    alias_ = tmp_;
    playlists_.insert(std::make_pair(m_lastEffectGuid, alias_));
    
    std::cout << "playlist created (" << m_lastEffectGuid << ")" << std::endl;

    SHOWDETAILEND(createPlaylist);
}

void SSAdmin::listPlaylist() {
    if(!checkConnection_()) {
        return;
    }

    SHOWDETAILSTART()
    
    PlaylistIDs ids;
    TRY_BEGIN
	ids = m_AdminPrx->listPlaylists();
	TRY_END

    PlaylistIDs::iterator it = ids.begin();
	for(short i = 1; it != ids.end(); ++it, ++i) {
	    PlaylistExPrx listPrx = PlaylistExPrx::uncheckedCast(m_AdminPrx->openPlaylist(*it, std::vector<int>(), false));
		if(!listPrx) {
			continue;
		}

        PLAYLISTS::const_iterator iter = playlists_.find(*it);
        if(iter != playlists_.end()) {
            std::cout << iter->second << '\t';
        }
        else {
            std::ostringstream oss;
            oss << i;
            playlists_[*it] = oss.str(); 

            std::cout << i << '\t';
        }
        std::cout << (*it) << '\t';
        
		StreamState state;

		TRY_BEGIN
	    state = listPrx->getCurrentState();
		TRY_END
        
        printState_(state);
		
		if(ids.empty()) {
            std::cerr << "no playlist available" << std::endl;
		}
    }
    SHOWDETAILEND(listPlaylist)
}

void SSAdmin::verbose(bool detail) {
    m_bShowDetail = detail;
}

void SSAdmin::setDestMac(const std::string& mac) {
    if(!checkPlaylist_()) {
        return;
    }

	SHOWDETAILSTART()

    TRY_BEGIN
    m_plPrx->setDestMac(mac);
	TRY_END
    
    SHOWDETAILEND(setDestMac);
}

void SSAdmin::erase(uint16 id) {
    if(!checkPlaylist_()) {
        return; 
    }

	SHOWDETAILSTART()
    
    TRY_BEGIN
	m_plPrx->erase(id);
    TRY_END

	SHOWDETAILEND(erase)
    
    std::cout << "item (" << id << ") erased from playlist (" << m_lastEffectGuid << ")" << std::endl;
}

void SSAdmin::destroy() {
    if(!checkPlaylist_()) {
        return;
    }

    SHOWDETAILSTART()
	
    TRY_BEGIN  
	m_plPrx->destroy();
	TRY_END

    playlists_.erase(m_lastEffectGuid);
    alias_.clear();

    SHOWDETAILEND(destroy)
}

void SSAdmin::destroyAll() {
    if(!checkConnection_()) {
        return;
    }

    TRY_BEGIN
    TianShanIce::Streamer::PlaylistIDs ids=m_AdminPrx->listPlaylists();
    for(size_t i = 0; i < ids.size(); ++i) {
        PlaylistExPrx prx = PlaylistExPrx::uncheckedCast(
                    m_AdminPrx->openPlaylist(ids[i], std::vector<int>(), false));
        prx->destroy();
    }
    TRY_END

    playlists_.clear();
    m_lastEffectGuid.clear();
    alias_.clear();
}

void SSAdmin::select(const std::string& alias) {
    if(!checkConnection_()) {
        return;
    }

    PlaylistExPrx prx = 0;
    bool found = false;

    /* try alias */
    PLAYLISTS::const_iterator iter = playlists_.begin();
    for(; iter != playlists_.end(); ++iter) {
        if(alias == iter->second) {
            TRY_BEGIN
            prx = PlaylistExPrx::uncheckedCast(m_AdminPrx->openPlaylist(iter->first, std::vector<int>(), false));
            TRY_END
            found = true;
            break;
        }
    }

    /* try guid */
    if(!prx && (iter = playlists_.find(alias)) == playlists_.end()) {
        std::cerr << "playlist " << alias << " not available" << std::endl;
        return;
    }

    m_plPrx = prx;
    alias_ = found ? alias : alias_ = iter->second;
    m_lastEffectGuid = iter->first;
     
    std::cout << "playlist (" << m_lastEffectGuid << ") selected" << std::endl;
}

void SSAdmin::alias(const std::string& guid, const std::string& name) {
    if(!checkConnection_()) {
        return;
    }
 
    playlists_[guid] = name;
}

void SSAdmin::seekItem(uint16 id, int64 offset, short from, float speed) {
    if(!checkPlaylist_()) {
        return;
    }

 	TianShanIce::Streamer::StreamInfo info;
 	TianShanIce::StrValues expectValues;
     
 	expectValues.push_back("ITEM_CURRENTPOS");
 	expectValues.push_back("ITEM_TOTALPOS");
 	expectValues.push_back("SPEED");
 	expectValues.push_back("STATE");
 	expectValues.push_back("USERCTRLNUM");
 	
    SHOWDETAILSTART()
     
    TRY_BEGIN	
 	info = m_plPrx->playItem(id, offset, from, speed, expectValues);
    TRY_END	
 
    SHOWDETAILEND(seek)
 
    dumpStreamInfo(info);
}

void SSAdmin::seek(int64 offset, short from) {
    if(!checkPlaylist_()) {
        return;
    }

    SHOWDETAILSTART()

    Ice::Long res = 0;
    TRY_BEGIN
    res = m_plPrx->seekStream(offset, from);
    TRY_END

    std::cout << "seek to: " << res << std::endl;

    SHOWDETAILEND(seek)
}

void SSAdmin::setSpeed(float speed) {
    if(!checkPlaylist_()) {
        return;
    }

    SHOWDETAILSTART();

	TRY_BEGIN
	m_plPrx->setSpeed(speed);
    TRY_END

    std::cout << "speed set to (" << speed << ")" << std::endl;
    SHOWDETAILEND(setSpeed);
}
void SSAdmin::pause() {
    if(!checkPlaylist_()) {
        return;
    }

    SHOWDETAILSTART();

	TRY_BEGIN
	m_plPrx->pause();
    TRY_END

    std::cout << "stream (" << m_lastEffectGuid << ") paused" << std::endl;

    SHOWDETAILEND(pause);
}

void SSAdmin::resume() {
    if(!checkPlaylist_()) {
        return;
    }

    SHOWDETAILSTART();

	TRY_BEGIN
	m_plPrx->resume();
    TRY_END

    std::cout << "stream (" << m_lastEffectGuid << ") resumed" << std::endl; 

    SHOWDETAILEND(resume);
}

void SSAdmin::current() const {
    if(m_AdminPrx) {
        TRY_BEGIN
        std::cout << "StreamSmith NetID: " << netId_ << '\n' 
                  << "Streamsmith proxy: " << m_ic->proxyToString(m_AdminPrx) << std::endl;
          
        std::string info = m_AdminPrx->ShowMemory();
        if(!info.empty()) {
            std::cout << info << std::endl;
        }
        TRY_END
    }
    if(m_plPrx) {
        TRY_BEGIN    
        std::cout << "control number (" << m_plPrx->current() << ") "
                  << "in playlist (" << m_lastEffectGuid << ")" 
                  << std::endl;
        TRY_END
    }
}

void SSAdmin::info() {
    if(!checkPlaylist_()) {
        return;
    }

    TRY_BEGIN
    std::cout << "id: " << m_lastEffectGuid << '\n'
              << "size: " << m_plPrx->size() << '\n'
              << "state: "; printState_(m_plPrx->getCurrentState());
    TRY_END

    int mask = -1;
loop:
    if(mask == -1) mask = TianShanIce::Streamer::infoPLAYPOSITION;
    else if(mask == infoDVBCRESOURCE) mask = TianShanIce::Streamer::infoPLAYPOSITION;
    else if(mask == infoPLAYPOSITION) mask = TianShanIce::Streamer::infoSTREAMNPTPOS;
    else if(mask == infoSTREAMNPTPOS) mask = TianShanIce::Streamer::infoSTREAMSOURCE;
    else if(mask == infoSTREAMSOURCE) return;
	
    TianShanIce::ValueMap var;	
    TRY_BEGIN
	if(!m_plPrx->getInfo(mask, var)) {
//      std::cerr << "failed to get playlist info" << std::endl;
        goto loop;
  	}
    TRY_END

	TianShanIce::ValueMap::iterator it = var.begin();
	for (; it != var.end() ; ++it) {				
	    std::string strKey = it->first;
		TianShanIce::Variant varValue = it->second;
		
        std::ostringstream oss;

        switch(varValue.type) {
		case TianShanIce::vtInts:
            oss << "[" << strKey << "]: " << varValue.ints[0];
            if(varValue.bRange) {
                oss << "-" << varValue.ints[1];
            }
		    break;
		case TianShanIce::vtLongs:
            oss << "[" << strKey << "]: " << varValue.lints[0];
            if (varValue.bRange) {
                oss << "-" << varValue.lints[1];
            }				
			break;
		case TianShanIce::vtStrings:
            oss << "[" << strKey << "]: " << varValue.strs[0];
			if (varValue.bRange) {
                oss << "-" << varValue.strs[1];
            }				
			break;
		case TianShanIce::vtFloats:
            oss << "[" << strKey << "]: " << varValue.floats[0];
			if (varValue.bRange) {
                oss << "-" << varValue.floats[1];
			}
			break;
		default:
			break;
        }
        std::cout << oss.str() << std::endl;
	}
    goto loop;
}

void SSAdmin::setRate(int rate) {
    if(!checkPlaylist_()) {
        return;
    }

	SHOWDETAILSTART()

    TRY_BEGIN
	m_plPrx->setMuxRate(0,rate,0);
    TRY_END

    std::cout << "bitrate set to (" << rate << ")" << std::endl;
    
    SHOWDETAILEND(setRate);		
}

void SSAdmin::listItem() {
    if(!checkPlaylist_()) {
        return;
    }

	SHOWDETAILSTART();
		
    TRY_BEGIN
	TianShanIce::IValues ivalue= m_plPrx->getSequence();
    std::cout << "there are " << ivalue.size() << " items in playlist " << m_lastEffectGuid << std::endl;
	for(int i=0;i<(int)ivalue.size();i++) {
        std::cout << "Item " << i << " = " << ivalue[i] << std::endl;
	}
    TRY_END
		
	SHOWDETAILEND(listItem);		
}

void SSAdmin::exit() {
    m_AdminPrx = 0;

    TRY_BEGIN
    m_ic->destroy();
    TRY_END

    quit_ = true;
}

bool SSAdmin::quit() const {
    return quit_;
}

bool SSAdmin::checkConnection_() {
    if(!m_AdminPrx) {
        std::cerr << "not connected to any StreamSmith server" << std::endl;
        return false;
    }
    return true;
}

bool SSAdmin::checkPlaylist_() {
    if(m_lastEffectGuid.empty() || !m_plPrx) { 
        std::cerr << "no playlist selected yet" << std::endl;
        return false;
    }
    if(!m_AdminPrx->openPlaylist(m_lastEffectGuid, std::vector<int>(), false)) {
        std::cerr << "current playlist is not valid any more" << std::endl;
        m_lastEffectGuid.clear();
        m_plPrx = 0;
    }

    return true;
}

void SSAdmin::printState_(StreamState state) const {
    switch(state) {
    case stsSetup:
        std::cout << "Setup" << std::endl;
        break;
    case stsStreaming:
        std::cout << "Streaming" << std::endl;
        break;
    case stsPause:
        std::cout << "Pause" << std::endl;
        break;
    case stsStop:
        std::cout << "Stop" << std::endl;
        break;
    default:
        std::cout << "Unknown status" << std::endl;
        break;
    }
}

extern int yyparse();
extern SSAdmin admin;

extern FILE *yyin;
extern bool isEOF;

void usage() {
    std::cout << "Usage: StreamClient [option] [arg]\n\n"
        << "Options:\n"
        << "  -h              show this message\n"
        << "  -e <endpoint>   ICE endpoint to be connected with\n"
        << "  -f <file>       read instruction from file, for batch jobs\n"
        << "  -v              output product version\n"
        << std::endl;
}

int main(int argc, char* argv[]) {
    std::string file;

    if(argc > 1) {
        int ch = 0;
        while((ch = getopt(argc, argv, "he:f:v")) != EOF) {
            if(ch == 'h') {
                usage();
                return (0);
            }			
            else if(ch == 'e') {
                admin.connect(optarg);
            }
            else if(ch == 'f') {
                file = optarg;
            }
            else if(ch == 'v') {
                admin.version();
                return (0);
            }
            else {
                std::cerr << "invalid option" <<  std::endl;
                return (0);
            }
        }
    }

    if(!file.empty()) {
        FILE* fp = std::fopen(file.c_str(), "r");
        yyin = fp;
       
        while(!isEOF) {
            yyparse();
        }
        if(!admin.quit()) {
            admin.exit();
        }
        fclose(fp);
    }	
    else {
        while(!admin.quit()) {
            //admin.prompt();
            yyparse();
        }
    }

    return (0);
}
