// ContentOprtXml.h: interface for the ContentOprtXml class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTENTOPRTXML_H__0C923BC0_AE74_4234_A25B_B44739DD067A__INCLUDED_)
#define AFX_CONTENTOPRTXML_H__0C923BC0_AE74_4234_A25B_B44739DD067A__INCLUDED_
#pragma  warning (disable: 4503)
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Log.h"
#include <string>
#include <vector>
#include <map>


struct ContentInfo
{
	std::string providerID;
	std::string assetID;
	std::string volumeName;
	int64		contentSize;
	int64		supportFileSize;
	std::string contentState;
	std::string createDate;
	std::string md5Checksum;
	std::string md5DateTime;
	ContentInfo():
	contentSize(0),
	supportFileSize(0){
	}
};

struct TransferInfo {
	std::string providerID;
	std::string assetID;
	std::string captureStart;
	std::string captureEnd;
	int			transferBitRate;
	std::string sourceURL;
	std::string sourceIP;
	std::string sourceURL1;
	std::string sourceIP1;
	std::string userName;
	std::string password;
	std::string volumeName;
	std::string responseURL;
	std::string homeId;

	typedef std::map<std::string, std::string> ContentAssets;
	ContentAssets element;

	TransferInfo():transferBitRate(0){
	}
};

struct TransferStatus {
	std::string providerID;
	std::string assetID;
	std::string volumeName;

	std::string state;
	int			reasonCode;
	int			percentComplete;
	int			contentSize;
	int			supportFileSize;
	std::string	md5Checksum;
	std::string md5DateTime;
	int			bitrate;

	TransferStatus():
	reasonCode(0),
	percentComplete(0),
	supportFileSize(0),
	bitrate(0) {
	}
};

struct ExposeContentInfo {
	std::string providerID;
	std::string assetID;
	std::string volumeName;
	std::string protocol;
	std::string transferBitRate;
};

struct ExposeResponse
{
	std::string providerID;
	std::string assetID;
	std::string URL;
	std::string userName;
	std::string password;
	int			ttl;
	int			transferBitRate;
	
	ExposeResponse()
	{
		ttl = 0;
		transferBitRate = 0;
	}
};
//for get ContentChecksum struct
struct ContentChecksumInfo {
	std::string providerID;
	std::string assetID;
	std::string volumeName;
	std::string responseURL;
};

struct ContentChecksum {
	std::string providerID;
	std::string assetID;
	std::string volumeName;
	std::string md5Checksum;
	std::string md5DateTime;
	int			resultCode;

	ContentChecksum():resultCode(0)	{
	}
};


struct DeleteCancelContent
{
	std::string providerID;
	std::string assetID;
	std::string volumeName;
	int			reasonCode;

	DeleteCancelContent():reasonCode(0) {
	}
};

enum VolumeState {
	OPERATIONAL = 200,
	VOLUME_FAILED = 500,
	OUT_OF_SERVICE = 501,
	REDUCED_CAPACITY = 502
};

struct VolumeInfo
{
	std::string volumeName;
	VolumeState	state;
	int			volumeSize;
	int			freeSize;
	
	VolumeInfo():
	state(OPERATIONAL),
	volumeSize(0),
	freeSize(0)	{
	}
};

class ContentOprtXml  
{
public:
	ContentOprtXml(ZQ::common::Log* pLog=0);
	virtual ~ContentOprtXml();

public:
	std::string makeTransferContent(const TransferInfo& transContent);
	
	//GetTransferStatus message
	std::string makeGetTransferStatus(const TransferStatus& transStatus);
	bool parseGetTransferStatus(TransferStatus& transStatus, const char* buf, size_t bufLen);

	std::string makeCancelTransfer(const DeleteCancelContent& cancelTrans);
	std::string makeDeleteContent(const DeleteCancelContent& delContent);
	
	//Expose message
	std::string makeExposeContent(const ExposeContentInfo& exposecont);
	bool parseExposeContent(ExposeResponse& exposeResp, const char* buf, size_t bufLen);

	std::string makeGetContentChecksum(const ContentChecksumInfo& getcontCheck);

	bool parseContentChecksum(struct ContentChecksum& contCheck, const char* buf, size_t bufLen);

	//GetVolumeInfo message
	std::string makeGetVolumeInfo(struct VolumeInfo& voluInfo);
	bool parseGetVolumeInfo(struct VolumeInfo& voluInfo, const char *buf, size_t bufLen);
	
	//GetContentInfo message
	std::string makeGetContentInfo(const ContentInfo& contInfo);
	bool parseGetContentInfo(std::vector<ContentInfo>& contInfoVector, const char *buf, size_t bufLen);

private:
	std::string setAttrStr(const char* key, const char* value);
	std::string setAttrStr(const char* key, int value);

private:
	ZQ::common::Log* _pContOprtLog;
};

#endif // !defined(AFX_CONTENTOPRTXML_H__0C923BC0_AE74_4234_A25B_B44739DD067A__INCLUDED_)
