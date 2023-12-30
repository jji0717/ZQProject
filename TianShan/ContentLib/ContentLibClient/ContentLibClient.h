#ifndef __ERM_CLIENT__
#define __ERM_CLIENT__

#include "TianShanDefines.h"
#include "ContentReplicaEx.h"

#ifdef _WIN32
#   include <io.h>
#	include <process.h>
#   define isatty _isatty
#   define fileno _fileno
#   ifdef _MSC_VER
#       pragma warning( disable : 4273 )
#   endif
#endif

using namespace TianShanIce::Repository;

class ContentLibClient {

public:
	ContentLibClient();
	~ContentLibClient();

public:
	typedef struct {
		::TianShanIce::Repository::ContentLibPrx clPrx_;
		::TianShanIce::Repository::ContentStoreReplicaPrx csPrx_;
		::TianShanIce::Repository::MetaVolumePrx mvPrx_;
		::TianShanIce::Repository::ContentReplicaPrx ctPrx_;
		std::string csName_;
		std::string mvName_;
		std::string ctName_;
	}  Context;

public:
	void usage(const std::string& key="") const;

	void prompt() const;

	void connect(const std::string& endpoint);
	void close();
	void exit();

	void info() const;
	void current() const;
	void destroy(bool force=false);
	void reset();

	void list();

	/* StoreReplica */
	void toStoreReplica(const std::string& name);
	void listVolume();
	
	/* MetaVolume */
	void toVolume(const std::string& name);
	void listContent();

	/* ContentReplica */
	void toContentReplica(const std::string& name);

public:

	bool isInteractive() const;
	void setInteractive(bool=true);

	/* set prop for the current context */
	void setProperty(const std::string& key, const std::string& val);

	bool quit() const;

private:

	void init();

	bool checkConnection() const;

private:
	Ice::CommunicatorPtr ic_;

private:

	bool quit_;
	bool interactive_;

	Context ctx_;

	/* metadata to list */
	TianShanIce::StrValues listMetadata_;

	/* current context */
	TianShanIce::Properties prop_;
};

// class ListContentsCB : public AMI_Volume_listContents {
// 	virtual void ice_response(const ContentInfos&);
// 	virtual void ice_exception(const Ice::Exception&);
// };


#endif
