#ifndef __ERM_CLIENT__
#define __ERM_CLIENT__

#include "TianShanDefines.h"
#include "EdgeRM.h"

#ifdef _WIN32
#   include <io.h>
#	include <process.h>
#   define isatty _isatty
#   define fileno _fileno
#   ifdef _MSC_VER
#       pragma warning( disable : 4273 )
#   endif
#endif

using namespace TianShanIce::EdgeResource;

class EdgeRMClient {

public:
	EdgeRMClient();
	~EdgeRMClient();

public:
	typedef struct {
		EdgeResouceManagerPrx erm_;
		EdgeDevicePrx device_;
		std::string deviceName_;
		EdgeChannelPrx channel_;
		std::string channelName_;
		AllocationPrx allocation_;
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

	/* device */
	void openDevice(const std::string& name);
	void addDevice(const std::string& name);
	void removeDevice(const std::string& name);
	void updateDevice(const std::string& name);
	void listDevice() const;
	void importDevice(const std::string&xmlDefFile);
	void exportDevice(const std::string& deviceName, const std::string& xmlFile);
	void importDevice(const std::string&name, const std::string&deviceGroup, const std::string&xmlDefFile, int bCompress, int count);
	void importRoutes(const std::string& routeFilePath);
	void exportRoutes(const std::string& deviceName,const std::string& outRoutesFilePath);
	void linkRouteName(const int EdgePortId, const std::string& routeName, const std::string& freqs);
	void linkRouteName(const int EdgePortId, const std::string& routeName, const int freq);
	void linkRouteName(const std::string& routeName, const std::string& freqs);
	void unlinkRouteName(const int EdgePortId, const std::string& routeName);
	void unlinkRouteName(const std::string& servicegroup);

	/* channel */
	void openChannel(const std::string& name);
	void openChannel(const int EdgePortId, const int chNum);
	void enableChannel(int flag);
	void updateChannel();
	void addChannel(const int portId,const int channelId,const int frequency,const int symbolrate,
		const int startUDPport,const int portStep,const int startPn,const int maxSession,
		const int lowBandwidth,const int highBandwidth);

	void listChannel(const int EdgePortId, bool enabledOnly = false) const;
	void populateChannel(const int EdgePortId);

	/* port */
	void openPort(const int& id);
	void addPort(const int& id);
	void removePort(const int& id);
	void updatePort(const int& id);
	void listPort() const;

	/* allocation */
	void openAllocation(const std::string& id);
	void createAllocation(const std::string& id);
	void removeAllocation(const std::string& id);
	void listAllocation() const;

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
