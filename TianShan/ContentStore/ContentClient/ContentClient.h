#ifndef __CS_CLIENT__
#define __CS_CLIENT__

#include "TianShanDefines.h"
#include "ContentStore.h"
#include "CacheStore.h"


using namespace TianShanIce::Storage;

class CSClient {

public:

	CSClient();
	~CSClient();

public:

	typedef std::map<ContentState, std::string> State;

	typedef struct {
		ContentStorePrx store_;
		std::string netId_;
		CacheStorePrx   cacheStore_;
		std::string cacheStoreNetId_;
		bool bContentStore;
		ContentPrx content_;
		std::string contentName_;
        FolderPrx folder_; 
        std::string folderName_;
		VolumePrx volume_;
		std::string volumeName_;
	}  Context;

public:

	std::string prompt() const;

	void usage(const std::string& key="") const;
	void connect(const std::string& endpoint);
	void close();
	void exit();

	/* show info of ContentStore currently connected */
	void info() const;

    /* go to parent folder */
    void parent();

	/* display current context */
	void current() const;
	
	/* destroy current content/folder */
	void destroy(bool force=false);

    /* folder */
	void openFolder(const std::string& name, bool create=false);
    void listFolder();
	void listContent(const std::string& pattern="", int count=0) const;

	/* volume */

	/* contentstore */
	void openContent(const std::string& name, bool create=false);
	void listVolume(const std::string& name="", bool includeFolder=false) const;
	void openVolume(const std::string& name);

	/* cacheStore */
	void exportContent(const ::std::string& contentName,const::std::string& subFile,int idleStreamTimeout,int cacheStoreDepth);
	void cache(const ::std::string& contentName,const ::std::string& srcCacheStoreName);
	void findContent(const ::std::string& contentName);
	void hashFolderName(const ::std::string& contentName);
	void getAccessCount(const ::std::string& contentName);
	void addAccessCount(std::string contentName,int countToAdd);
	void getMissedList(int maxNum);
	void getHotList(int maxNum);
	void setAccessThreshold();
	void getAccessThreshold();
	void cacheDistance(std::string contentName, std::string storeNetId);
	void cacheDistanceList(std::string contentName);
	void nameOfLocal(std::string contentName, std::string subfile);
	void exportContentWithCost(const ::std::string& contentName,const::std::string& subFile,int idleStreamTimeout,int cacheStoreDepth,bool bCost);

	void sysCmd(char* sysCommand);
	/* content */
	void provision(const std::string& url = "", int = 0);
	void cancel() const;
	void expose(const std::string& proto) const;

	void adjustSchedule(const std::string& start="", const std::string& stop="") const;

	void timer(bool start);

	void mySleep(int msec);

public:

	bool isInteractive() const;
	void setInteractive(bool=true);

	/* set prop for the current context */
	void setProperty(const std::string& key, const std::string& val);

	/* display all metadata of current content */
	void getMetaData() const;
	/* set a single metadata pair for the current content */
	void setMetaData(const std::string& key, const std::string& val);
	
    void setSysMD(const std::string& key, const std::string& val);
	void syncWithFS() const;

	
	bool quit() const;

private:
	
	void init_();

    bool checkConnection_() const;

private:

	Ice::CommunicatorPtr ic_;

private:
	
	bool quit_;
	bool interactive_;

	Context ctx_;

private:
    
	/* metadata to list */
	TianShanIce::StrValues listMetadata_;

	/* current context */
	TianShanIce::Properties prop_;
	bool  _bCost;

};
#if  ICE_INT_VERSION / 100 >= 306
    class WithFSCB : public IceUtil::Shared
    {   
    public:
        WithFSCB(){}   
    private:
        void handleException(const Ice::Exception& ex){}
    public:
        void syncWithFileSystem(const ::Ice::AsyncResultPtr& r)
		{ 
		::TianShanIce::Storage::VolumeExPrx VolumeEx = ::TianShanIce::Storage::VolumeExPrx::uncheckedCast(r->getProxy()); 
            try 
            {   
                VolumeEx->end_syncWithFileSystem(r);
            }   
            catch(const Ice::Exception& ex) 
            {   
                handleException(ex);
            } 
		}   
	};  
	typedef IceUtil::Handle<WithFSCB> WithFSCBPtr;
#else
class SyncWithFSCB : public AMI_VolumeEx_syncWithFileSystem {
public:
	virtual void ice_response();
	virtual void ice_exception(const Ice::Exception&);

	void setVolumeName(const std::string& name) {volumeName_ = name;}

private:
	std::string volumeName_;
};
#endif

#endif //__CS_CLIENT__
