#ifndef __ZQTianShan_NGODStorePortal_H__
#define __ZQTianShan_NGODStorePortal_H__

#include "GBCSSimpleHttpd.h"
#include "GBCSReq.h"
#include "TianShanDefines.h"
#include "ContentImpl.h"

#include "NativeThreadPool.h"
#include "SystemUtils.h"

//content state
#define  PORTAL_STATE_UNKNOWN		"Unknown"
#define  PORTAL_STATE_PENDING		"Pending"
#define  PORTAL_STATE_TRANSFER		"Transfer"
#define  PORTAL_STATE_STREAMABLE	"Transfer/Play"
#define  PORTAL_STATE_COMPLETE		"Complete"
#define  PORTAL_STATE_CANCELED		"Canceled"
#define  PORTAL_STATE_FAILED		"Failed"

namespace ZQTianShan {
namespace NGOD_CS {

class PortalListener;

// -----------------------------
// class GBCSStorePortal
// -----------------------------
class GBCSStorePortal : public ZQ::common::ThreadRequest
{
	friend class VolumeSyncCmd;
	friend class PortalListener;

public:

	GBCSStorePortal(ZQTianShan::ContentStore::ContentStoreImpl& store, size_t poolSize =5, int minInterval=-1);
	virtual ~GBCSStorePortal();

	void wakeup(void);
	void quit(void);
	bool bQuit(void){ return _bQuit; }

	std::string getA3Url(void);//A3 stream server url
	std::string getResponseAddr(void);//which SimpleServer listen,format(127.0.0.1:12345)

public:

	typedef struct _ContentInfo
	{
		std::string name;
		int64		contentSize;
		int64		supportFileSize;
		std::string contentState;
		int64		stampCreated;
		std::string md5Checksum;
		int64		stampMD5;
//		std::string md5DateTime;
		int			bitRate;

		_ContentInfo() { contentSize = supportFileSize = stampCreated = stampMD5 = bitRate =0; }

		bool diff(struct _ContentInfo other) const
		{
			return (contentSize != other.contentSize)
				|| (supportFileSize != other.supportFileSize)
				|| (stampCreated != other.stampCreated)
				|| (stampMD5 != other.stampMD5)
				|| (bitRate != other.bitRate)
				|| (0 != contentState.compare(other.contentState))
				|| (0 != md5Checksum.compare(other.md5Checksum));
		}

	} ContentInfo;

	typedef std::map<std::string, ContentInfo> ContentMap; // the map of content name to ContentInfo

	typedef struct _VolumeInfo
	{
//		std::string name;         // the name of volume
		std::string targetName;   // the target name of volume
		int64		stampLastTry; // the timestamp when the sync is triggered to perform
		int64		stampLastSync; // the timestamp when this volume has been sync-ed with NGOD SS
		int64		stampDirty; // the timestamp when this volume was clearly marked as dirty

		ContentMap  contents; // the contents map of the Volume
//		ZQ::common::Mutex lockContents; // the locker of the per-volume contents

		_VolumeInfo() { stampLastTry = -1;  stampLastSync = stampDirty=0; }
	} VolumeInfo;

	typedef std::map<std::string, VolumeInfo> VolumeMap; // the map of volume name to VolumeInfo

	ZQ::common::Mutex _lockVolumes; // the locker of _volumeMap
	VolumeMap               _volumeMap;
	TianShanIce::Properties _volTargetNameIdx;

	SYS::SingleObject _hStartList;//first get content list set signaled
protected:

	// implementation of NativeThread
	virtual bool init(void);
	virtual int run(void);

protected:
	int OnTransferStatus(const SimpleHttpd::MsgInfo& info, std::string& respHttpMessageBody);

protected:

	ZQTianShan::ContentStore::ContentStoreImpl& _store;

	//for response
	std::string _strIP;//local ip
	int			_port;//local port,which SimpleServer listen

	//for request
	std::string _defaultA3Url;

private:

	SYS::SingleObject _hWakeUp;
	bool		_bQuit;
	int64		_stampStarted;

	ZQ::common::NativeThreadPool _thpool;
	int			_interval;

	PortalListener*	_pWebResp;
};

class GlobalReqType
{
public:
	typedef enum GBCS_REQ_COMMAND{
		GB_A4_REQ   =  1,
		GB_A5_REQ   =  2,
		CMD_TOTAL
	}reqCmd;

	static reqCmd getReqCmdType(void){return _reqCmdType;}

	static int setReqCmdType(reqCmd setReq)
	{
		if (GB_A4_REQ > setReq || GB_A5_REQ < setReq)
			return false;

		_reqCmdType = setReq;
		return true;
	}  

private:
	static reqCmd  _reqCmdType;
};

}
}

#endif // __ZQTianShan_NGODStorePortal_H__
