#include <sstream>
#include <cstdio>
#include <iomanip>
#include "strHelper.h"
#include "TimeUtil.h"
#include "ZQResource.h"
#include "ContentImpl.h"
#include "ContentClient.h"
#include "SystemUtils.h"

#ifdef ZQ_OS_LINUX
#include <unistd.h>
#include "version.h"
#endif
#include "getopt.h"

#define TRY_BEGIN \
	try {
#define TRY_END \
	} \
	catch(const TianShanIce::Storage::NoResourceException& e) { \
		std::cerr << e.message << std::endl; \
		return; \
	} \
	catch(const TianShanIce::InvalidParameter& e) { \
		std::cerr << e.message << std::endl; \
		return; \
	} \
	catch(const TianShanIce::InvalidStateOfArt& e) { \
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
    catch(const char* str) { \
        std::cerr << str << std::endl; \
    } \
    catch(...) { \
        std::cerr << "unknown error" << std::endl; \
    }

void version() {
	std::cout << "Console client for ContentStore version: " 
#ifdef ZQ_OS_LINUX
	<< VERSION	
#else
	<< ZQ_PRODUCT_VER_MAJOR << "." 
	<< ZQ_PRODUCT_VER_MINOR << "." 
	<< ZQ_PRODUCT_VER_PATCH << "(build " 
	<< ZQ_PRODUCT_VER_BUILD << ")\n"
#endif
	<< std::endl;
}

namespace {
	const char* BITRATE = "bitrate";
	const char* SRCTYPE = "sourceType";
	const char* STARTTIME = "startTime";
	const char* ENDTIME = "endTime";
    const char* TIMEOUT = "timeout";
	const char* LISTMD = "lookForMetaData";
	const char* LOWBANDWIDTH = "lowBandwidth";
	const char* HIGHBANDWIDTH = "highBandwidth";
	const char* SESSIONINTERFACE = "sessionInterface";
	const char* DESTIP = "destIP";
	const char* SINCE = "since";
	const char* TIMEWINOFPOPULAR = "timeWinOfPopular";
	const char* COUNTOFPOPULAR = "countOfPopular";
	const char* HOTTIMEWIN = "hotTimeWin";

    unsigned defaultTimeout = 5000;
	
	unsigned provCounter = 0;

    const char* serverStates[] = {
        "Not Provisioned",
        "Provisioned",
        "In Service",
        "Out Of Service" 
    };
}

using namespace TianShanIce::Storage;


CSClient::CSClient():quit_(false),interactive_(false) {

 	ic_ = Ice::initialize();

	init_();
}

CSClient::~CSClient() {

}

/*
*    [ContentClient|netId]:volume/folder:content>  
*/
std::string CSClient::prompt() const {
    std::ostringstream oss;
    oss << (ctx_.netId_.empty() ? "ContentClient" : ctx_.netId_);
    if(!ctx_.folderName_.empty()) {
        oss << ":" << ctx_.folderName_;  
    }
    else if(!ctx_.volumeName_.empty()) {
        oss << ":" << ctx_.volumeName_;
    } 

    if(!ctx_.contentName_.empty()) {
        oss << ":" << ctx_.contentName_;
    }
    oss << "> ";

#ifdef ZQ_OS_MSWIN
    std::cout << oss.str();
#endif
    
    return oss.str();
}


void CSClient::usage(const std::string& key) const {
// ruler         "-------------------------------------------------------------------------------"
	version();

    std::cout << "\n\n"
		      << "connect <endpoint>           ICE endpoint,\n"
			  << "                             eg:\"CacheStore:tcp -h 10.0.0.1 -p 10700\"\n"
              << "                             refer to variable <timeout>, require a \n"
              << "                             reconnection to take effect\n" 
              << "close                        disconnect with ContentStore\n"
			  << "exit|quit                    exit shell\n"
			  << "clear                        clear screen\n"
			  << "sleep <milliseconds>         suspend content client for a period of time\n"
			  << "help                         display this screen\n"
			  << "current                      display the current console context\n"
              << "up                           go up one level to parent folder/volume\n"
			  << "set [<var>=<value>]          set a variable in the context, show if no args\n"
			  << "open <contentName> [true]    open a content, use true to create if not exist\n"
			  << "open volume <volumeName>     open a mounted volume\n"
			  << "open folder <name> [true]    open a subfolder, true to create\n"
			  << "list [startName][count]      list contents start from the name, refer to the\n"
              << "                             variable <lookForMetaData>\n"
			  << "list volume [<name>][true]   list available volume, true to include folders\n"
              << "list folder                  list folders under a volume\n"
			  << "hashFolderName <contentName> show the hashed folder name by content name\n"
			  << "info                         show info of current content/folder/volume/store\n"
			  << "expose  [ftp|cifs]           expose current content, with the optional proto\n"
			  << "cancel                       cancel current session\n"
			  << "destroy [true]               destroy current content/volume, true for a force\n"
			  << "                             deletion on content even when in use\n"
			  << "provision <url> [bitrate]    provision the current content by pulling media\n"
			  << "                             from the given URL. This command will refer to\n"
			  << "                             the variable <startTime> <endTime> <bitrate>  \n"
			  << "                             and <sourceType>, to specify relative time\n"
              << "                             o, use a '+' sign that follows an offset, eg: \n"
              << "                             set startTime =now (to start from current time)\n"
              << "                             set startTime =+180(to start 3 minutes later)\n" 
              << "                             set endTime =+1800 (duration set to 30 minutes\n"
              << "sync                         update storage info on cluster with undelaying\n" 
              << "                             filesystem on node\n"
			  << "update <start|stop> time     update schedule time for opened session, \n"
			  << "                             start or stop time can be specified seperately\n"
			  << "update <time1> <time2>       update both schedule time at the same time\n"
		  	  << "mset [<metaData>=<value>]    set a user meta data of the current content \n"
			  << "                             or volume. displays all metadata if no args\n"
			  << "!<systemCommand>             execute the system command,eg:\"!dir /c /d\"\n";

	if (ctx_.cacheStore_)
	{
		std::cout
			  << "export <content> <subFile>   eg:export cdntest1234567890127zq.com index 6000 1\n"
			  << "<Timeout> <cacheStoreDepth>  This command will refer to the variables\n"	
			  << "                             <lowBandwidth>(KB/s) <highBandwidth>(KB/s) <sessionInterface>\n"
			  << "                             and <destIP>\n"
			  << "                             set sessionInterface=\"c2http://10.15.10.50:1234\"\n"
			  << "cache <content> <endpoint>   eg:cache cdntest1234567890127zq.com\n"
			  << "                             \"tcp -h 10.15.10.50 -p 10700\" \n"
			  << "hashFolder <content>         show the path of the content in CacheStore\n"
			  << "                             eg:hashFolder rtitest1234567890010zq.com\n"
			  << "accessCount <content>        This command will refer to the variables <since>\n"
			  << "listMissed <maxNum>          show the missed content\n"
			  << "listHot <maxNum>             show the hot contents in local\n"
			  << "setCacheWindow               set properties of access threshold\n"
			  << "                             This command will refer to the variables:\n"
			  << "                             <timeWinOfPopular> <countOfPopular> and \n"
			  << "                             <hotTimeWin>\n"
			  << "cacheWindow                  show the properties of access threshold\n"
			  << "localFn <content> <subfile>  show the local name of <content>,<subfile> is\n"
			  << "                             similar to the extension\n"
		      << "storeDistance <content> <NetID> show the distance between <content> \n"
			  << "                                and <cacheStore>\n"
			  << "storeDistance <content>      show distance list between <content> and the\n"
			  << "                             different cache stores\n";

	}
	std::cout<< std::endl;
}

void CSClient::connect(const std::string& endpoint) {
    init_();

	std::ostringstream oss;
	//oss << SERVICE_NAME_ContentStore << ":" << endpoint;
 	oss<<endpoint;
    std::string::size_type pos = endpoint.rfind("-t");
    if(pos  == std::string::npos) {
        oss << " -t " << defaultTimeout;
    }
    else {
        std::istringstream iss(endpoint.substr(pos+3));

        iss >> defaultTimeout;
    }

	std::string::size_type contentStorePos = oss.str().rfind("ContentStore");
	if(std::string::npos != contentStorePos)
		ctx_.bContentStore = true;
	else
		ctx_.bContentStore = false;

	try {
		if(ctx_.bContentStore)
			ctx_.store_ = ContentStorePrx::checkedCast(ic_->stringToProxy(oss.str()));
		else
			ctx_.cacheStore_ = CacheStorePrx::checkedCast(ic_->stringToProxy(oss.str())); 

		if (ctx_.bContentStore && !ctx_.store_) {
            std::cerr << "failed connecting with (" << oss.str() << ")" << std::endl;
	 		return;
		}
		if(!ctx_.bContentStore && !ctx_.cacheStore_){
			std::cerr << "failed connecting with (" << oss.str() << ")" << std::endl;
			return;
		}
		if(ctx_.cacheStore_)
			ctx_.store_ = ctx_.cacheStore_->theContentStore();

		ctx_.netId_ = ctx_.store_->getNetId();
	}
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
		return;
	}

	std::cout << "connected with (" << oss.str() << ")" << std::endl;
}

void CSClient::close() {
    if(checkConnection_()) {
	    init_();
    }
}

void CSClient::info() const {
    if(!checkConnection_()) {
        return;
    } 

	if(ctx_.content_) {
		std::cout << "proxy: " << ic_->proxyToString(ctx_.content_) << std::endl;
//        getMetaData();
        return;
	}
    
    if(ctx_.folder_) {
        /* nothing */
        return;
    }

	if(ctx_.volume_) {
	    VolumeExPrx volumeEx = VolumeExPrx::uncheckedCast(ctx_.volume_);
		VolumeInfo info = volumeEx->getInfo();

		Ice::Long totalMB = 0, freeMB = 0;

		TRY_BEGIN
		volumeEx->getCapacity(freeMB, totalMB);
		
		Ice::Identity id = volumeEx->getIdent();

		std::cout << "========== " << info.name << " ==========" << "\n"
			      << "virtual: " << std::boolalpha << info.isVirtual << "\n"
		     	  << "quota: " << info.quotaSpaceMB << " MB" << "\n"
				  << "free: " << freeMB << " MB\n"
				  << "total: " << totalMB << " MB\n"
				  << "mount: " << volumeEx->getMountPath() << "\n"
				  << "identity: " << id.category << id.name << std::endl;
		TRY_END

		TianShanIce::Properties::const_iterator meta = info.metaData.begin();
		for(; meta != info.metaData.end(); ++meta) {
			std::cout << meta->first << ": " << meta->second << std::endl;
		}
		return;
	}

	if(ctx_.store_) {

		TRY_BEGIN
		std::cout << "netId: " << ctx_.netId_ << "\n"
			      << "type: "  << ctx_.store_->type() << "\n"
				  << "valid: " << std::boolalpha << ctx_.store_->isValid() << "\n"
	//			  << "url: "   << ctx_.store_->getAdminUri() << "\n"
                  << "state: " << serverStates[ctx_.store_->getState()] << std::endl;	
        TRY_END        
        
        try {
            std::cout << "cacheLevel: " << static_cast<int>(ctx_.store_->getCacheLevel()) << std::endl;
        }
        catch(TianShanIce::NotSupported) {
            /* no cache mode */
        }
        catch(Ice::Exception&) {}
        catch(...) {}
	}
}

void CSClient::parent() {

    if(ctx_.content_) {
        ctx_.content_ = 0;
        ctx_.contentName_.clear(); 
        return;
    }

//folder:
    if(ctx_.folder_) {
        FolderPrx folder = ctx_.folder_->parent();

        if(!folder) { 
            /* no folder anymore */
            ctx_.folder_ = 0;
            ctx_.folderName_.clear();
            
            goto volume;
        }
        else {
            ctx_.folder_ = FolderPrx::uncheckedCast(folder->ice_timeout(defaultTimeout));
            ctx_.folderName_ = ctx_.folder_->getName();
            
            return;
        }
    }

volume:
    if(ctx_.volume_) {
        FolderPrx volume = ctx_.volume_->parent();
                
        /* no volume anymore */
        if(!volume) {
            ctx_.volume_ = 0;
            ctx_.volumeName_.clear();
        }
        else {
            ctx_.volume_ = VolumePrx::uncheckedCast(volume->ice_timeout(defaultTimeout));
            ctx_.volumeName_ = ctx_.volume_->getName();
        }
    }
}

void CSClient::current() const {
	std::ostringstream oss;

	if(ctx_.store_) {
		oss <<  "ContentStore NetID: " << ctx_.netId_ << "\n";
		oss <<  "ContentStore proxy: " << ic_->proxyToString(ctx_.store_) << "\n";
	}
	if(ctx_.folder_) {
		oss << "Volume: " << ctx_.folderName_ << "\n";
	}
	if(ctx_.content_) {
		oss << "Content: " << ctx_.contentName_ << "\n";
	}

	TianShanIce::Properties::const_iterator iter = prop_.begin();
	for(; iter != prop_.end(); ++iter) {
		oss << "var[" << iter->first << "]: " << iter->second << "\n";
	}
	
	if(!oss.str().empty()) {
		std::cout << oss.str();
	}
}

void CSClient::openVolume(const std::string& name) {
    if(!checkConnection_()) {
        return;
    }

	VolumePrx volume = 0;
	TRY_BEGIN
	volume = ctx_.store_->openVolume(name);
	
	if(!volume) {
		std::cerr << "failed to open volume (" << name << ")" << std::endl;
		return;
	}

    ctx_.volume_ = VolumePrx::uncheckedCast(volume->ice_timeout(defaultTimeout));
	ctx_.volumeName_ = volume->getVolumeName();
	TRY_END
	
	ctx_.content_ = 0;
	ctx_.contentName_.clear();

    ctx_.folder_ = 0;
    ctx_.folderName_.clear();

	std::cout << "volume (" << ctx_.volumeName_ << ") opened" << std::endl;
}

void CSClient::openFolder(const std::string& name, bool create) {
	FolderPrx folder = 0;

	TRY_BEGIN

    if(ctx_.folder_) {
        folder = ctx_.folder_->openSubFolder(name, create, 0);
    }
	else if(ctx_.volume_) {
		folder = ctx_.volume_->openSubFolder(name, create, 0);
	}
	else if(ctx_.store_) {
		folder = ctx_.store_->openSubFolder(name, create, 0);
	}
    else {
        std::cerr <<  "not connected or no active volume/folder opened" << std::endl;
        return;
    }
	
	if(!folder) {
		std::cerr << "failed to open folder " << "(" << name << ")" << std::endl;
		return;
	}

    ctx_.folder_ = FolderPrx::uncheckedCast(folder->ice_timeout(defaultTimeout));
	ctx_.folderName_ = ctx_.folder_->getName();

	TRY_END

	ctx_.content_ = 0;
	ctx_.contentName_.clear();

	std::cout << "folder" << " (" << name << ") opened" << std::endl;
}

void CSClient::listFolder() {
    FolderInfos infos;
    
    TRY_BEGIN
    
    if(ctx_.folder_) {
        infos = ctx_.folder_->listSubFolders();
    }
    else if(ctx_.volume_) {
        infos = ctx_.volume_->listSubFolders();
    }
    else if(ctx_.store_) {
        infos = ctx_.store_->listSubFolders();
    }
    else {
        std::cerr <<  "not connected or no active volume/folder opened" << std::endl;
    }

    TRY_END

    if(infos.empty()) {
		std::cerr << "no matching folders found" << std::endl;
		return;
    }

	FolderInfos::const_iterator iter = infos.begin();
	for(; iter != infos.end(); ++iter) {
		std::cout << iter->name << std::endl;
	}
}

void CSClient::listVolume(const std::string& name, bool includeFolder) const {
    if(!checkConnection_()) {
        return;
    }

	VolumeInfos infos;

	TRY_BEGIN
	infos = ctx_.store_->listVolumes(name, includeFolder);
	TRY_END

	if(infos.empty()) {
		std::cerr << "no matching volume found" << std::endl;
		return;
	}

	VolumeInfos::const_iterator iter = infos.begin();
	for(; iter != infos.end(); ++iter) {
		std::cout << iter->name << std::endl;
	}
}

void CSClient::openContent(const std::string& name, bool create) {
    if(!checkConnection_()) {
        return;
    }

	bool byFullName = false;

	if(!ctx_.bContentStore){
		std::string folderName;
		TRY_BEGIN
		folderName = ctx_.cacheStore_->getFolderNameOfContent(name);
		TRY_END
		std::string::size_type pos;
		folderName = folderName.substr(1);
		pos = folderName.find('/');
		if(std::string::npos == pos){
			std::cerr<<"open folder error"<<std::endl;
			return;
		}
		std::string folderName1 = folderName.substr(0,pos);
		folderName = folderName.substr(++pos);
		pos = folderName.find('/');
		if(std::string::npos == pos){
			std::cerr<<"open folder error"<<std::endl;
			return;
		}
		std::string folderName2 = folderName.substr(0,pos);
		
		TRY_BEGIN
		openFolder(folderName1, create);
		openFolder(folderName2, create);
		TRY_END
	}
	else{
		if(!ctx_.folder_ && (name.find('/') != std::string::npos)) {
			byFullName = true;
		}
	}
	ContentPrx content = 0;
	TRY_BEGIN
	if(byFullName) {
		content = ctx_.store_->openContentByFullname(name);	
	}
	else if(ctx_.folder_) {
		content = ctx_.folder_->openContent(name, ctMPEG2TS, create);
	}
    else if(ctx_.volume_) {
        content = ctx_.volume_->openContent(name, ctMPEG2TS, create);
    }
	else if(!byFullName && ctx_.store_){
		content = ctx_.store_->openContent(name, ctMPEG2TS, create);
	}
	TRY_END

	if(!content) {
		std::cerr << "failed to open content (" << name << ")" << std::endl;
		return;
	}
	
	if(byFullName) {
		TRY_BEGIN
		ctx_.contentName_ = content->getName();
		TRY_END
	}
	else {
		ctx_.contentName_ = name;
	}

	if(!ctx_.folder_) {
		TRY_BEGIN
        ctx_.folder_ = FolderPrx::uncheckedCast(content->theVolume()->ice_timeout(defaultTimeout));
		ctx_.folderName_ = ctx_.folder_->getName();
		TRY_END
	}
	
    ctx_.content_ = ContentPrx::uncheckedCast(content->ice_timeout(defaultTimeout));
	
	std::cout << "content (" << name << ") opened" << std::endl;
}

void CSClient::listContent(const std::string& pattern, int count) const {
	if(!ctx_.bContentStore && ctx_.volumeName_.empty()){
		listVolume();
		return;
	}

    ContentInfos infos;


    TRY_BEGIN

    if(ctx_.folder_) {
        infos = ctx_.folder_->listContents(listMetadata_, pattern, count);
    }
    else if(ctx_.volume_) {
        infos = ctx_.volume_->listContents(listMetadata_, pattern, count);
    }
    else if(ctx_.store_) {
        infos = ctx_.store_->listContents(listMetadata_, pattern, count);
    }
    else {
        std::cerr <<  "not connected or no active volume/folder opened" << std::endl;
        return;
    }

    TRY_END

    if(infos.empty()) {
		std::cerr << "no matching contents found" << std::endl;
        return;
    }

	int contentCount = 0;
 	ContentInfos::const_iterator iter = infos.begin();
 	for(; iter != infos.end(); ++iter) {
 		std::cout << (iter->name);
 
 		if(!iter->metaData.empty()) {
 		    std::cout << ": ";
        }
         
        TianShanIce::Properties::const_iterator metadata = iter->metaData.begin();
        for(; metadata != iter->metaData.end(); ++metadata) {
            std::cout << "[" << metadata->first << "]=[" << metadata->second << "] ";			
        }
         
        std::cout << std::endl;
		contentCount++;
    }
	std::cout<<"total:"<<contentCount<<std::endl;
}

bool CSClient::isInteractive() const {
	return interactive_;
}

void CSClient::setInteractive(bool val) {
	interactive_ = val;
}

void CSClient::setProperty(const std::string& key, const std::string& val) {
	if(key == LISTMD) {
		listMetadata_ = ZQ::common::stringHelper::split(val, ',');
        /* not show as a context variable */
        return;
	}

    if(key == TIMEOUT) {
        std::istringstream iss(val);
        iss >> defaultTimeout;

        if(ctx_.store_) {
            ctx_.store_ = ContentStorePrx::uncheckedCast(ctx_.store_->ice_timeout(defaultTimeout));
        } 
    }
	if(key == "highBandwidth" || key == "lowBandwidth")
	{
		char kVal[100];
		memset(kVal,0,100);
		sprintf(kVal,"%lld",_atoi64(val.c_str())*1000);
		prop_[key] = kVal;
		return;
	}
	prop_[key] = val;
}

void CSClient::getMetaData() const {
	if(!ctx_.content_) {
		std::cerr << "no active content opened" << std::endl;
		return;
	}

	TianShanIce::Properties metadata;
	TRY_BEGIN
	metadata = ctx_.content_->getMetaData();
	TRY_END;

	TianShanIce::Properties::const_iterator iter = metadata.begin();
	for(; iter != metadata.end(); ++iter) {
		std::cout << "[" << iter->first << "]: " << iter->second << std::endl;
	}
}

void CSClient::setMetaData(const std::string& key, const std::string& val) {
	if(!ctx_.content_) {
		std::cerr << "no active content opened" << std::endl;
		return;
	}
	
	TRY_BEGIN
	ctx_.content_->setUserMetaData(key, val);
	TRY_END
}

void CSClient::setSysMD(const std::string& key, const std::string& val) {
    if(!ctx_.content_) {
        std::cerr << "no active content opened" << std::endl;
        return;
    }

    TRY_BEGIN
    TianShanIce::Properties md;
    MAPSET(TianShanIce::Properties, md, key, val);
    TianShanIce::Storage::UnivContentPrx::uncheckedCast(ctx_.content_)->setMetaData(md);
    TRY_END
}

void CSClient::syncWithFS() const {
	if(!ctx_.volume_) {
		std::cerr << "no active volume opened" << std::endl;
		return;
	}
	
	TRY_BEGIN
	VolumeExPrx volumeEx = VolumeExPrx::uncheckedCast(ctx_.volume_);
#if ICE_INT_VERSION / 100 >= 306
	WithFSCBPtr CbPtr = new WithFSCB();
	Ice::CallbackPtr genericCB = Ice::newCallback(CbPtr, &WithFSCB::syncWithFileSystem);
	volumeEx->begin_syncWithFileSystem(genericCB);
#else
	SyncWithFSCB* cb = new SyncWithFSCB();
	volumeEx->syncWithFileSystem_async(cb);
	cb->setVolumeName(ctx_.volumeName_);
#endif
	TRY_END
}

bool CSClient::quit() const {
	return quit_;
}

void CSClient::init_() {
	ctx_.store_ = 0;
	ctx_.cacheStore_ = 0;
	ctx_.content_ = 0;
    ctx_.volume_ = 0;
	ctx_.folder_ = 0;
	_bCost = false;
	ctx_.netId_.clear();
	ctx_.contentName_.clear();
	ctx_.volumeName_.clear();
	ctx_.folderName_.clear();

	listMetadata_.clear();
	prop_.clear();
}

bool CSClient::checkConnection_() const {
	if(ctx_.bContentStore){
		if(!ctx_.store_) {
			std::cerr << "not connected with any ContentStore server" << std::endl;
				return false;
		}
	}
	else{
		if(!ctx_.cacheStore_){
			std::cerr << "not connected with any CacheStore server" << std::endl;
				return false;
		}
	}
    return true;
}

void CSClient::provision(const std::string& url, int bitrateReq) {
	if(!ctx_.content_) {
		std::cerr << "no active content opened" << std::endl;
		return;
	}
	
	TianShanIce::Properties::const_iterator stime = prop_.find(STARTTIME);
	TianShanIce::Properties::const_iterator etime = prop_.find(ENDTIME);
	if(stime == prop_.end() || etime == prop_.end()) {
		std::cerr << "no startTime or endTime specified" << std::endl;
		return;
	}
  
    std::string startTime, endTime;
    
    {
        /* startTime: empty = now, +xxx start offset in seconds */
        time_t now = time(0);
        time_t newStart = 0;

        char buff[64];

        if(stime->second == "now") {
            if(ZQ::common::TimeUtil::Time2Iso(now, buff, 64)) {
                startTime = buff;
            }
        }
        else if(stime->second.at(0) == '+') {
            std::istringstream iss(stime->second.substr(1));
            iss >> newStart;
            newStart = now + newStart;
            if(ZQ::common::TimeUtil::Time2Iso(newStart, buff, 64)) {
                startTime = buff;
            }
        }
        else {
        }

        if(etime->second.at(0) == '+') {
            time_t newEnd = 0;
            std::istringstream iss(etime->second.substr(1));
            iss >> newEnd;
            
            newEnd = (!newStart) ? (now+newEnd) : (newStart+newEnd);
            if(ZQ::common::TimeUtil::Time2Iso(newEnd, buff, 64)) {
                endTime = buff;
            }
        }
    }

    if(startTime.empty()) {
        startTime = stime->second;
    }
    if(endTime.empty()) {
        endTime = etime->second;
    }

 	TianShanIce::Properties::const_iterator stype = prop_.find(SRCTYPE);
	std::string sourceType = (stype == prop_.end() ? "MPEG2TS" : stype->second);

	TianShanIce::Properties::const_iterator br = prop_.find(BITRATE);
	int bitrate = (bitrateReq ? bitrateReq : (br == prop_.end() ? bitrateReq : atoi(br->second.c_str())));

    std::string passiveURL;
  	TRY_BEGIN
    if(url.empty()) {
        passiveURL = ctx_.content_->provisionPassive(sourceType, true, startTime, endTime, bitrate);
    }
    else {
        ctx_.content_->provision(url, sourceType, true, startTime, endTime, bitrate);
    }
	TRY_END
	
	std::cout << "Content [" << ctx_.contentName_ << "] is scheduled to provision:\n"
		      << "\tSourceType: " << sourceType << "\n"
              << (url.empty() ? "\tPassiveURL: " : "\tSourceURL:  ") 
              << (url.empty() ? passiveURL : url) << "\n"
			  << "\tStartTime:  " << startTime << "\n"
			  << "\tEndtime:    " << endTime << std::endl;

	++provCounter;

	/* remove session releated info */
	prop_.clear();
}

void CSClient::destroy(bool force) {
	bool volume = false, folder = false;

	if(!ctx_.content_ && 
       !ctx_.folder_  && 
       !ctx_.volume_) {
        std::cerr << "no active volume, folder or content opened" << std::endl;
        return;
    }
    
	std::string n;

	if(ctx_.content_) {
		n = ctx_.contentName_;

		TRY_BEGIN	
		if(force) {
			ctx_.content_->destroy2(force);
		}
		else {
			ctx_.content_->destroy();
		}
		TRY_END

		ctx_.content_ = 0;
		ctx_.contentName_.clear();
	}
    else if(ctx_.folder_) {
        folder = true;

        FolderPrx upper = 0;
        n = ctx_.folderName_;

        TRY_BEGIN
        upper = ctx_.folder_->parent();
        ctx_.folder_->destroy();
        TRY_END
    
        ctx_.folder_ = upper;
        if(upper) {
			ctx_.folderName_ = upper->getName();
		}
		else {
			ctx_.folderName_.clear();
		}
    }
	else {
        volume = true;

		VolumePrx upper = 0;
		n = ctx_.volumeName_;

		TRY_BEGIN
		upper = VolumePrx::uncheckedCast(ctx_.volume_->parent());
		ctx_.volume_->destroy();
		TRY_END

		ctx_.volume_ = upper;
		if(upper) {
			ctx_.volumeName_ = upper->getName();
		}
		else {
			ctx_.volumeName_.clear();
		}
	}

	std::cout << (folder?"folder":(volume?"volume":"content")) << "(" << n << ") destroyed" << std::endl;
}

void CSClient::cancel() const {
	if(!ctx_.content_) {
		std::cerr << "no active content opened" << std::endl;
		return;
	}

	TRY_BEGIN
	ctx_.content_->cancelProvision();
	TRY_END

	std::cout << "session (" << ctx_.contentName_ << ") cancelled" << std::endl;
}

void CSClient::expose(const std::string& proto) const {
	if(!ctx_.content_) {
		std::cerr << "no active content opened" << std::endl;
		return;
	}
	
	TianShanIce::Properties::const_iterator br = prop_.find(BITRATE);
	int bitrate = (br == prop_.end() ? 0 : atoi(br->second.c_str()));

 	Ice::Int ttl = 0;
 	TianShanIce::Properties prop;
 	std::string url;
 
 	TRY_BEGIN
 	url = ctx_.content_->getExportURL(proto, bitrate, ttl, prop);
 	TRY_END
 	
 	std::cout<< "url: " << url << std::endl;
 
 	TianShanIce::Properties::const_iterator iter = prop.begin();
 	for(; iter != prop.end(); ++iter) {
 		std::cout << iter->first << ": " << iter->second << std::endl;
 	}
}

void CSClient::adjustSchedule(const std::string& start, const std::string& stop) const {
	if(!ctx_.content_) {
		std::cerr << "no active content opened" << std::endl;
		return;
	}
	
	TRY_BEGIN
	ctx_.content_->adjustProvisionSchedule(start, stop);
	TRY_END

	if(start.empty() && stop.empty()) {
		std::cout << "no change to the schedule time for session (" << ctx_.contentName_ << ")" << std::endl;
	}
	else {
		std::cout << "new schedule for (" << ctx_.contentName_ 
			      << "): start [" << start << "] stop [" << stop << "]" << std::endl;
	}
}

void CSClient::timer(bool start) {
	static unsigned long timerStart; 
	int currCounter = 0;
	
	char buff[100];
	memset(buff, '\0', 100);

	if(start) {
		timerStart = SYS::getTickCount();
		currCounter = provCounter;
	
		ZQ::common::TimeUtil::Time2Str(time(0), buff, 100);
		std::cout << "timer start [" << buff << "]" << std::endl;
	}
	else {
		ZQ::common::TimeUtil::Time2Str(time(0), buff, 100);
		std::cout << "timer stop [" << buff << "] "
		          << "provision: " << (provCounter-currCounter) << " "
                  << "elapsed: " << (SYS::getTickCount()-timerStart) << " (ms)" << std::endl;
		provCounter = 0;
	}
}


void CSClient::exit() {
	try{
		ic_->destroy();
	} 
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch(...) {
		std::cerr << "unknown error" << std::endl;  /* not necessary but a place holder */
	}

	quit_ = true;
}

extern FILE *yyin;
extern int yyparse();

extern bool isEOF;

extern CSClient client;

void usage() {
	std::cout << "Usage: ContentClient [option] [arg]\n\n"
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
				client.connect(optarg);
			}
			else if(ch == 'f') {
				file = optarg;
			}
            else if(ch == 'v') {
				version();	
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
		
		client.setInteractive(false);
		while(!isEOF) {
			yyparse();
		}
		if(!client.quit()) {
			client.exit();
		}
		fclose(fp);
	}	
	else {
		client.setInteractive(true);

		while(!client.quit()) {
			client.prompt();
			yyparse();
		}
	}

	return (0);
}
#if  ICE_INT_VERSION / 100 >= 306
#else
void SyncWithFSCB::ice_response() {
	std::cout << "sync with volume [" << volumeName_ << "] completed." << std::endl;
    std::cout << std::endl;
}

void SyncWithFSCB::ice_exception(const Ice::Exception& e) {
 	std::cerr << e.ice_name() << std::endl;
}
#endif
// vim: ts=4 sw=4 bg=dark nu

void CSClient::exportContent(const ::std::string& contentName,const::std::string& subFile,int idleStreamTimeout,int cacheStoreDepth)
{
	int64 stampBegin = 0;
	int64 stampCost = 0;

	stampBegin = ZQ::common::now();

	::TianShanIce::SRM::ResourceMap resources; TianShanIce::Properties params;

	TianShanIce::Properties::const_iterator lowBandWidth = prop_.find(LOWBANDWIDTH);
	TianShanIce::Properties::const_iterator highBandWidth = prop_.find(HIGHBANDWIDTH);
	TianShanIce::Properties::const_iterator sessionInterface = prop_.find(SESSIONINTERFACE);
    TianShanIce::Properties::const_iterator destIP = prop_.find(DESTIP);


	if(lowBandWidth == prop_.end() || highBandWidth == prop_.end() || destIP == prop_.end()){
		std::cerr << "no variables specified" << std::endl;
		return;
	}

	int64 lBandWidth = _atoi64(lowBandWidth->second.c_str());
	int64 hBandWidth = _atoi64(highBandWidth->second.c_str());
	std::string dIP = destIP->second;

	//for the resource of rtTsDownstreamBandwidth
	{	
		::TianShanIce::SRM::Resource resDownStream;
		resDownStream.status   = TianShanIce::SRM::rsRequested;
		resDownStream.attr   = TianShanIce::SRM::raNonMandatoryNonNegotiable;
		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtLongs;
		v.bRange = true;
		v.lints.push_back(lBandWidth);
		v.lints.push_back(hBandWidth);
		MAPSET(::TianShanIce::ValueMap, resDownStream.resourceData, "bandwidth", v);
		MAPSET(::TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtTsDownstreamBandwidth, resDownStream);
	}

	// for the resource of rtTsUpstreamBandwidth
	{	
		::TianShanIce::SRM::Resource resUpStream;
		resUpStream.status   = TianShanIce::SRM::rsRequested;
		resUpStream.attr   = TianShanIce::SRM::raNonMandatoryNonNegotiable;
		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtLongs;
		v.bRange = true;
		v.lints.push_back(lBandWidth);
		v.lints.push_back(hBandWidth);
		MAPSET(::TianShanIce::ValueMap, resUpStream.resourceData, "bandwidth", v);
		if(sessionInterface != prop_.end())
		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtStrings;
			v.bRange = false;
			v.strs.push_back(sessionInterface->second);
			MAPSET(::TianShanIce::ValueMap, resUpStream.resourceData, "sessionInterface", v);
		}

		MAPSET(::TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtTsUpstreamBandwidth, resUpStream);
	}
	
	// for the resource of rtEthernetInterface
	{	
		::TianShanIce::SRM::Resource res;
		res.status   = TianShanIce::SRM::rsRequested;
		res.attr   = TianShanIce::SRM::raNonMandatoryNonNegotiable;
		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtStrings;
		v.bRange = false;
		v.strs.push_back(dIP);
		MAPSET(::TianShanIce::ValueMap, res.resourceData, "destIP", v);
		MAPSET(::TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtEthernetInterface, res);
	}
	
	MAPSET(TianShanIce::Properties, params, "Range",   "0-");
	MAPSET(TianShanIce::Properties, params, "CDNType", "");
	MAPSET(TianShanIce::Properties, params, "TansferDeplay", "0");

	try{
		::TianShanIce::Streamer::StreamPrx stream = ctx_.cacheStore_->exportContentAsStream(contentName,subFile,idleStreamTimeout,cacheStoreDepth,resources,params);

		if(NULL == stream)
		{
			std::cout<<"create stream failed"<<std::endl;
			return;
		}

		::TianShanIce::SRM::ResourceMap resOfStrm;
		::TianShanIce::SRM::ResourceMap::iterator itRes;
		resOfStrm = stream->getResources();
		itRes = resOfStrm.find(::TianShanIce::SRM::rtTsDownstreamBandwidth);
		if(resOfStrm.end() != itRes)
		{
			::TianShanIce::ValueMap& resData = itRes->second.resourceData;
			::TianShanIce::ValueMap::iterator itV = resData.find("sessionURL");

			if(client._bCost == true)
			{
				stampCost = ZQ::common::now() - stampBegin;
				std::cout<<stampCost<<" ";
			}

			if(resData.end() != itV)
			{
				std::cout<<itV->second.strs[0]<<std::endl;
			}
		}

	}catch(const Ice::Exception& e){
		std::cerr<<e.ice_name() <<std::endl;
	}
	catch(...)
	{
        std::cerr<<"exportContentAsStream caught unknown error" <<std::endl;
	}
	
	return;

}

void CSClient::exportContentWithCost(const ::std::string& contentName,const::std::string& subFile,int idleStreamTimeout,int cacheStoreDepth,bool bCost)
{
	if(bCost == true)
	{
		client._bCost = true;
		client.exportContent(contentName,subFile,idleStreamTimeout,cacheStoreDepth);
		client._bCost = false;

	}

	else
	{
		client.exportContent(contentName,subFile,idleStreamTimeout,cacheStoreDepth);
	}

	return;
}

void CSClient::cache(const ::std::string& contentName,const ::std::string& srcEndpoint)
{
	::TianShanIce::Storage::CacheStorePrx srcCachePrx;
	std::string cacheStoreEndPoint = "";
	std::string cacheStoreName = "CacheStore:";
	if(!srcEndpoint.empty())
		cacheStoreEndPoint = cacheStoreName + srcEndpoint;
	TianShanIce::Properties params; 
	try
	{
		srcCachePrx = ::TianShanIce::Storage::CacheStorePrx::checkedCast(ic_->stringToProxy(cacheStoreEndPoint));
		ctx_.cacheStore_->cacheContent(contentName,srcCachePrx,params);
	}
	catch (const Ice::Exception& e)
	{
		std::cerr<<e.ice_name()<<std::endl;
	}

}

void CSClient::findContent(const ::std::string& contentName)
{
	if(!ctx_.cacheStore_)
	{
		std::cerr<<"connection is not cacheStore"<<std::endl;
		return;
	}

	std::string folder = "";

	try{
		folder = ctx_.cacheStore_->getFolderNameOfContent(contentName);
	}
	catch(const Ice::Exception& e)
	{
		std::cerr<<e.ice_name()<<std::endl;
	}
	std::cout<<folder<<std::endl;

	return;
}

void CSClient::hashFolderName(const ::std::string& contentName)
{
	if(!ctx_.store_)
	{
		std::cerr << "not connected with any ContentStore server" << std::endl;
		return;
	}

	try
	{
		ContentStoreExPrx store = ContentStoreExPrx::uncheckedCast(ctx_.store_);
		std::string folder = store->hashFolderNameByContentName(contentName);

		std::cout<<folder<<std::endl;
	}
	catch(const Ice::Exception& e)
	{
		std::cerr<<e.ice_name()<<std::endl;
	}

	return;
}


void CSClient::mySleep(int msec)
{
#ifdef ZQ_OS_MSWIN
	::Sleep(msec);
#else
	sleep(msec/1000);
#endif

	return;
}

static void timeFormat(std::string& utcTime)
{
	utcTime.erase(0,5);
	size_t pos;

	utcTime = utcTime.substr(0,14);
	
}

void CSClient::getMissedList(int maxNum)
{
	TianShanIce::Storage::ContentAccessList accList;

	try{
		accList = ctx_.cacheStore_->listMissedContents(maxNum);
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
		return;
	}

	std::vector<ContentAccess>::iterator iter = accList.begin();

	if(iter == accList.end())
	{
		std::cout<<"no missed content exists"<<std::endl;

		return;
	}

	std::cout<<"Count"<<" "<<"Since\t"<<"     "<<"Till\t"<<"     "<<"ContentName"<<std::endl;

	for(iter;iter != accList.end();iter++)
	{
		char buffSince[60],buffLatest[60];

		ZQ::common::TimeUtil::TimeToUTC(iter->stampSince,buffSince,sizeof(buffSince),true);
		ZQ::common::TimeUtil::TimeToUTC(iter->stampLatest,buffLatest,sizeof(buffLatest)),true;
		std::string since(buffSince);
		std::string latest(buffLatest);
		timeFormat(since);
		timeFormat(latest);

		std::cout<<std::setw(5)<<std::right<<iter->accessCount<<" "
			<<std::left<<since<<" "
			<<std::left<<latest<<"  "
			<<std::left<<iter->contentName
			<<std::endl;
	}
	
	return;
}

void CSClient::getHotList(int maxNum)
{
	TianShanIce::Storage::ContentAccessList accList;

	try{
		accList = ctx_.cacheStore_->listHotLocals(maxNum);
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
		return;
	}

	std::vector<ContentAccess>::iterator iter = accList.begin();

	if(iter == accList.end())
	{
		std::cout<<"no hot contents exist in locals"<<std::endl;

		return;
	}

	std::cout<<"Count"<<" "<<"Since\t"<<"     "<<"Till\t"<<"     "<<"ContentName"<<std::endl;

	for(iter;iter != accList.end();iter++)
	{
		char buffSince[60],buffLatest[60];

		ZQ::common::TimeUtil::TimeToUTC(iter->stampSince,buffSince,sizeof(buffSince),true);
		ZQ::common::TimeUtil::TimeToUTC(iter->stampLatest,buffLatest,sizeof(buffLatest),true);
		std::string since(buffSince);
		std::string latest(buffLatest);
		timeFormat(since);
		timeFormat(latest);

		std::cout<<std::setw(5)<<std::right<<iter->accessCount<<" "
			<<std::left<<since<<" "
			<<std::left<<latest<<"  "
			<<std::left<<iter->contentName
			<<std::endl;
	}

	return;
}

void CSClient::setAccessThreshold()
{
	TianShanIce::Properties::iterator timeWinOfPopular = prop_.find(TIMEWINOFPOPULAR);
	TianShanIce::Properties::iterator countOfPopular = prop_.find(COUNTOFPOPULAR);
	TianShanIce::Properties::iterator hotTimeWin = prop_.find(HOTTIMEWIN);

	if(timeWinOfPopular == prop_.end() || countOfPopular == prop_.end() || hotTimeWin == prop_.end()){
		std::cout<<"variables timeWinOfPopular,countOfPopular or hotTimeWin not found"<<std::endl;
		return;
	}

	try{
		ctx_.cacheStore_->setAccessThreshold(atoi(timeWinOfPopular->second.c_str()),
				atoi(countOfPopular->second.c_str()),atoi(hotTimeWin->second.c_str()));
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
	}
	std::cout<<"set access threshold success"<<std::endl;

	return;
}

void CSClient::getAccessThreshold()
{
	int timeWinOfPopular=0,countOfPopular=0,hotTimeWin=0;

	try{
		ctx_.cacheStore_->getAccessThreshold(timeWinOfPopular,countOfPopular,hotTimeWin);
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
	}

	std::cout<<"threshold info:\n"
			 <<"time window of popular(sec):"<<timeWinOfPopular<<"\n"
	         <<"count of popular:"<<countOfPopular<<"\n"
			 <<"hot time window(msec):"<<hotTimeWin
			 <<std::endl;

	return;
}

void CSClient::addAccessCount(std::string contentName,int countToAdd)
{
	TianShanIce::Properties::iterator since = prop_.find(SINCE);
	if(since == prop_.end())
	{
		std::cout<<"variable since is not found"<<std::endl;
		return;
	}

	try{
		ctx_.cacheStore_->addAccessCount(contentName,countToAdd,since->second);//not implemented

		if(since->second.empty())
		{
			std::cout<<"counts "<<countToAdd<<" has been added to "<<contentName<<std::endl;
		}
		else
		{
			std::cout<<"counts "<<countToAdd<<" has been added to "<<contentName
					 <<" since "<<since->second<<std::endl;
		}

	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
	}
	
	return;
}

void CSClient::getAccessCount(const ::std::string& contentName)
{
	TianShanIce::Storage::ContentAccess sContent;

	try{
		sContent = ctx_.cacheStore_->getAccessCount(contentName);
		
		char buffSince[60],buffLatest[60];

		ZQ::common::TimeUtil::TimeToUTC(sContent.stampSince,buffSince,sizeof(buffSince),true);
		ZQ::common::TimeUtil::TimeToUTC(sContent.stampLatest,buffLatest,sizeof(buffLatest),true);
		std::string since(buffSince);
		std::string latest(buffLatest);
		timeFormat(since);
		timeFormat(latest);

		std::cout<<"Count"<<" "<<"Since\t"<<"     "<<"Till\t"<<"     "<<"ContentName"<<std::endl;
		std::cout<<std::setw(5)<<std::right<<sContent.accessCount<<" "
			<<std::left<<since<<" "
			<<std::left<<latest<<"  "
			<<std::left<<contentName
			<<std::endl;
	}
	catch(::Ice::Exception& e)
	{
		::std::cout<<e.ice_name()<<::std::endl;
	}

	return;

}

void CSClient::cacheDistance(std::string contentName, std::string storeNetId)
{
	Ice::Long distance = 0;

	try{
		distance = ctx_.cacheStore_->calculateCacheDistance(contentName,storeNetId);
		std::cout<<"the distance between "<<contentName<<" and "<<storeNetId<<" is "<<distance<<std::endl;
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
	}

	return;
}

void CSClient::cacheDistanceList(std::string contentName)
{
	CacheCandidates cacheCandidates;

	try{
		cacheCandidates = ctx_.cacheStore_->getCandidatesOfContent(contentName,true);
		CacheCandidates::iterator iter = cacheCandidates.begin();

		int count = sizeof(iter->csd.netId);
		std::cout<<"netID";
		int i = 0;
		do 
		{
			std::cout<<"\t";
			i++;
		} while (i<=count/8);
		std::cout<<"distance"<<std::endl;

		for(;iter != cacheCandidates.end();iter++)
		{
			std::cout<<iter->csd.netId<<"\t"<<iter->distance<<std::endl;
		}
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
	}
}

void CSClient::nameOfLocal(std::string contentName, std::string subfile)
{
	//subfile is similar to the extension
	std::string nameOfLocal = "";

	try{
		nameOfLocal = ctx_.cacheStore_->getFileNameOfLocalContent(contentName,subfile);
		std::cout<<"the local name of "<<contentName<<" is "<<nameOfLocal<<std::endl;
	}
	catch(Ice::Exception& e){
		std::cout<<e.ice_name()<<std::endl;
		return;
	}


	return;
}

void CSClient::sysCmd(char* sysCommand)
{
	/*
	std::string cmd(sysCommand);
	cmd.erase(0,1);

	system(cmd.c_str());
	*/
	int i = 0;
	for(;i<sizeof(sysCommand);i++)
	{
		sysCommand[i] = *(sysCommand+i+1);
	}

	system(sysCommand);

	return;
}