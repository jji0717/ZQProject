#ifndef ___ZQMODApplication_Config_H__
#define ___ZQMODApplication_Config_H__

#include <configloader.h>
#include <string>
#include <map>

namespace ZQMODApplication
{
/*
class ModConfigLoader : public ZQ::common::ConfigLoader
{
public: 
	ModConfigLoader();
	~ModConfigLoader();

protected: 
	virtual ZQ::common::ConfigLoader::PConfigSchema getSchema();

}; // class ModConfigLoader


struct TestItemInfo
{
	std::string name;
	int cueIn;
	int cueOut;
	int bandWidth;
};
typedef std::map<std::string, std::string> PARAMMAP;
// the information about Authorization config
struct AuthInfo
{
	bool enableAuthorize;
	std::string moduleName;
	std::string funcEntry;
	PARAMMAP paramMap;
};

// the information about Playlist config
struct GetPLInfo
{
	std::string moduleName;
	std::string funcEntry;
	PARAMMAP paramMap;
};

struct AppPathInfo
{
	std::string name;				// the app path name
	bool useDftAppPath;				// indicates whether or not use the default app path
	AuthInfo authInfo;
	GetPLInfo getPLInfo;
};
typedef std::map<std::string, AppPathInfo>PATHMAP;*/
class ModConfig
{
public: 
	ModConfig() : enableCrushDump(false), enableIceTrace(false), iceLogLevel(6), iceLogSize(10000000), 
			evctSize(100), useTestItem(false), purchaseTimeout(1800000),initRecordBufferSize(5000), useTestAuthParam(false)
	{
		crushDumpPath.clear();
		safeStorePath.clear();
		icePropMap.clear();
		iceStormEndPoint.clear();
		iceLogPath.clear();
		adapEndPoint.clear();
//		testItems.clear();
//		pathMap.clear();
	}
	virtual ~ModConfig()
	{
	}

public: 
	bool enableCrushDump;
	std::string crushDumpPath;

	// the directory of safeStore
	std::string safeStorePath;

	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	// stores the parameters to authorization
	bool useTestAuthParam;
	std::map<std::string, std::string> testAuthParamMap;

	// defines the ice storm endpoint
	std::string iceStormEndPoint;

	// defines the ice log properties
	bool enableIceTrace;
	std::string iceLogPath;
	int iceLogLevel;
	int iceLogSize;

	// defines the local endpoint of adapter
	std::string adapEndPoint;

	// the endpoint of the lam system
	//std::string lamEndPoint;

	// defines whether or not to use the configured testing items
	// as the asset elements gained from the LAM system
	bool useTestItem;
//	std::vector<TestItemInfo> testItems;

	// defines the authorization properties
	//bool oteAuthEnable;
	//std::string oteEndPoint;

	// the timeout value of mod purchase
	int purchaseTimeout; // ms
	// defines the size of the active queue of purchase's evictor
	int evctSize;
	// defines the initial size when load from db
	int initRecordBufferSize;

	// map a path such as "60010000" to a context which contains all the information
	// to process the special path, the key should be equal to AppPathInfo's name member
//	PATHMAP pathMap;

};

class SmartPreference
{
public: 
	SmartPreference(ZQ::common::XMLPreferenceEx*& p);
	virtual ~SmartPreference();
	// return the name property's int value
	// name[in], the property name
	// value[out], stores the property's int value
	// if name property not found, return false, etherwise return true;
	bool getIntProp(const char* name, int& value);

	// return the name property's string value
	// name[in], the property name
	// buff[out], the buffer to receive the property value
	// buffSize[in], the buffer's size
	// if name property not found, return false, etherwise return true;
	bool getStrProp(const char* name, char* buff, const int buffSize);

protected: 
	ZQ::common::XMLPreferenceEx*& _p;
};

} // namespace ZQMODApplication

#endif // #define ___ZQMODApplication_Config_H__

