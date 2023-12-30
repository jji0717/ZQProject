#pragma once
#define WIN32_LEAN_AND_MEAN

#include "ContentStore.h"
#include "TsStorage.h"
#include <../../../common/ZQ_common_conf.h>
#include <string>

#ifdef ZQ_OS_MSWIN
	#pragma warning(disable : 4996)
#else
#define stricmp strcasecmp
#endif

class Parse
{
public:
	Parse(void);
	~Parse(void);
	bool parseCom(std::string& strCom);
	bool connectSer(std::string& strEndpoint);	
	void Parse::usage();

	//contentstore test function
	std::string netId();
	std::string type();
	bool valid();
	TianShanIce::Storage::VolumePrx openVolume(std::string& volname);
	std::vector<TianShanIce::Storage::VolumeInfo> listVolumes(std::string strFrom, bool includeVirtual);
	TianShanIce::Storage::ContentPrx openContentByName(std::string strFullName);
	
	uint8 cacheLevel();
	TianShanIce::Replicas streamServices();

	void event(std::string& strEventType,std::string& strContentName,std::map<std::string,std::string> params);;
	void provisionEvent(std::string& ProvisionEvent, std::string& storeNetId, std::string& volumeName, std::string& contentName, TianShanIce::Properties& params);

	//content test function
	void populate(void);
	bool dirty();
	void restore(std::string& bFileExist);
	std::string filePath();
	void fileModify();
	void fileRename(std::string& strNewName);
	bool isInUse();
	TianShanIce::Storage::ContentState enterState(TianShanIce::Storage::ContentState targetState);
	uint64 checkResidentialStatus(uint64& flagsToTest);

	TianShanIce::Storage::ContentStorePrx getStore();
	TianShanIce::Storage::VolumePrx	getVolume();
	std::string getName();
	bool metaData();	
	void setMetaData(std::string& strKey, std::string strValue);
	//setMetaData2();
	std::string getState();
	bool provisioned();
	std::string provisionTime();

	void destroy();
	void destroy2(bool mandatory);
	std::string localType();
	std::string subType();
	Ice::Float frameRate();
	void resolution(int& horizontalPixel, int& verticalPixel);
	long fileSize();
	long supportFileSize();
	long playTime();
	//playTimeEX();
	int bitRate();
	std::string md5Checksum();
	TianShanIce::Storage::TrickSpeedCollection trickSpeed();
	std::string exportURL(std::string& transferProtocol, int transferBitrate, int& ttl, TianShanIce::Properties& params);
	std::string sourceURL();
	void provision(std::string& sourceUrl,std::string& sourceContentType,bool overwrite,std::string startTimeUTC,std::string stopTimeUTC,int maxTransferBitrate);
	std::string provisionPassive(std::string& sourceContentType,bool overwrite,std::string startTimeUTC,std::string stopTimeUTC,int maxTransferBitrate);
	void cancelProvision();

	//volume test function
	std::string mountPath();
	TianShanIce::Storage::VolumeInfo getInfo();
	void syncFileSystem();

	std::string getVolumeName();
	void capacity(Ice::Long& freeMB, Ice::Long& totalMB);
	std::vector<std::string> listAll(std::string condition);
	TianShanIce::Storage::ContentInfos listContent(TianShanIce::StrValues& metaDataNames, std::string& startName, int& maxCount);
	

	TianShanIce::Storage::ContentPrx openContent(const std::string& strName, const std::string& strContentType, bool createIfNotExist);
	TianShanIce::Storage::VolumePrx openSubVolume(std::string& subname, bool createIfNotExist, long& quotaSpaceMB);
	TianShanIce::Storage::VolumePrx parent();
	void destroyVolume();


private:
	Ice::CommunicatorPtr		_commun;
	TianShanIce::Storage::ContentStorePrx	_contentStorePrx;
	TianShanIce::Storage::VolumePrx			_volumePrx;
	TianShanIce::Storage::UnivContentPrx		_contentPrx;;
};
