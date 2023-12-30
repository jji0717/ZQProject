#ifndef __NATIVE_SERVICE_CONFIG___
#define __NATIVE_SERVICE_CONFIG___

#include "ConfigHelper.h"

#define CONFIGURATION_XML	"NativeCS.xml"

using namespace ZQ::common;

struct VolumeConfig {
	std::string name;
	std::string path;
	int32 isDefault;	
	
	static void structure(Config::Holder<VolumeConfig>& holder) {
		holder.addDetail("", "name", &VolumeConfig::name);
		holder.addDetail("", "path", &VolumeConfig::path);
		holder.addDetail("", "default", &VolumeConfig::isDefault, "0");
	}
};

struct NativeServiceConfig {
	std::string netId;
	int32 mediaClusterID;
	int32 dispatchSize, dispatchMax;
	int32 workerThreadSize;
	int32 contentEvictorSize;
	int32 volumeEvictorSize;

	int32 isCacheMode;
	int32 cacheLevel;

	int32 defaultProvisionBW;
	std::string	strTrickSpeeds;
	std::vector<float>  trickSpeedCollection;

	std::string clusterEndpoint;

	std::string			csStrReplicaGroupId;
	std::string			csStrReplicaId;
	int32				csIReplicaPriority;
	int32				csReplicaTimeout;		//in milisecond


	/* <IceProperties> */
	typedef std::map<std::string, std::string> ICEProperties;
	ICEProperties iceProp;

	typedef std::vector< Config::Holder<VolumeConfig> > Volumes;
	Volumes volumes;

	/* <Bind> */
	std::string cpcEndPoint;
	int registerInterval; // milli seconds

	static void structure(Config::Holder<NativeServiceConfig>& holder);

	void readICEProp(XMLUtil::XmlNode node, const Preprocessor* hPP);
	void registerICEProp(const std::string& path) ;

	void readVolumes(XMLUtil::XmlNode node, const Preprocessor* hPP);
	void registerVolumes(const std::string& path) ;
	
	void registerNothing(const std::string&){}

};

typedef std::map<std::string, Config::Holder<NativeServiceConfig> > ConfigMap;

struct ConfigGroup {

	int32 iceTraceEnabled;
	int32 iceTraceLevel;
	int32 iceTraceSize;

	std::string dbPath;

	Config::Holder<NativeServiceConfig>		mccsConfig;

	//////////////////////////////////////////////////////////////////////////
	//
	//	set contentstore netId, after this the mccsConfig is asigned to 
	//  right configugration
	bool setContentStoreNetId(const std::string& netId = "");

protected:
	static void structure(Config::Holder<ConfigGroup>& holder);

	void readConfigGroup(XMLUtil::XmlNode node, const Preprocessor* hPP) ;

	void registerConfigGroup(const std::string& full_path);

private:
	ConfigMap configMap;
};

extern Config::Loader<ConfigGroup>	configGroup;

#endif
