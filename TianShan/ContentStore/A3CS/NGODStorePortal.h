#ifndef __ZQTianShan_NGODStorePortal_H__
#define __ZQTianShan_NGODStorePortal_H__

#include "SimpleHttpd.h"
#include "A3HttpReq.h"
#include "TianShanDefines.h"
#include "ContentImpl.h"

#include "NativeThreadPool.h"
#include "SystemUtils.h"

#define  CONTENT_SYNC_TIMEOUT_MINUTE  (10)
#define  CONTENT_ATTR_TTL_MINUTE      (CONTENT_SYNC_TIMEOUT_MINUTE +5)

namespace ZQTianShan {
namespace NGOD_CS {

class A3Listener;

// -----------------------------
// class NGODStorePortal
// -----------------------------
class NGODStorePortal : public ZQ::common::ThreadRequest
{
	friend class VolumeSyncCmd;
	friend class A3Listener;
	friend class ::A3Request;

public:

	NGODStorePortal(ZQTianShan::ContentStore::ContentStoreImpl& store, size_t poolSize =5, int minInterval=-1);
	virtual ~NGODStorePortal();

	void wakeup(void);
	void quit(void);
	bool bQuit(void){ return _bQuit; }

	std::string getA3Url();//A3 stream server url
	std::string getResponseAddr();//which SimpleServer listen,format(127.0.0.1:12345)

public:

	typedef struct _ContentInfo
	{
		std::string name;
		int64		contentSize;     // in Bytes
		int64		supportFileSize; // in Bytes
		std::string contentState;
		int64		stampCreated;
		std::string md5Checksum;
		int64		stampMD5;
//		std::string md5DateTime;
		int			bitRate;

		int64       stampFromServer;

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
//		std::string name;          // the name of volume
		std::string targetName;    // the target name of volume
		int64		stampLastTry;  // the timestamp when the sync is triggered to perform
		int64		stampLastSync; // the timestamp when this volume has been sync-ed with NGOD SS
		int64		stampDirty;    // the timestamp when this volume was clearly marked as dirty

		ContentMap  contents; // the contents map of the Volume
//		ZQ::common::Mutex lockContents; // the locker of the per-volume contents

		_VolumeInfo() { stampLastTry = -1;  stampLastSync = stampDirty=0; }
	} VolumeInfo;

	typedef std::map<std::string, VolumeInfo> VolumeMap; // the map of volume name to VolumeInfo

	bool queryInfoByContent(ContentInfo& ci, const std::string& contentName, const std::string& volName, bool byTargetVol =false);

	ZQ::common::Mutex _lockVolumes; // the locker of _volumeMap
	VolumeMap               _volumeMap;
	TianShanIce::Properties _volTargetNameIdx;

	SYS::SingleObject _hStartList;//first get content list set signaled
protected:

	// implementation of NativeThread
	virtual bool init(void);
	virtual int run(void);

protected:

	int OnTransferStatus(const SimpleHttpd::MsgInfo& info, A3Request::MessageCtx& msgData);
	int OnContentChecksum(const SimpleHttpd::MsgInfo& info, A3Request::MessageCtx& msgData);

protected:

	ZQTianShan::ContentStore::ContentStoreImpl& _store;
/*
	typedef std::map<std::string, ContentInfo> ContentMap; // the map of content name to ContentInfo

	typedef struct _VolumeInfo
	{
		std::string name;  // the name of volume
		int64		stampLastTry; // the timestamp when the sync is triggered to perform
		int64		stampLastSync; // the timestamp when this volume has been sync-ed with NGOD SS
		int64		stampDirty; // the timestamp when this volume was clearly marked as dirty

		ContentMap  contents; // the contents map of the Volume
//		ZQ::common::Mutex lockContents; // the locker of the per-volume contents

		_VolumeInfo() { stampLastTry = -1;  stampLastSync = stampDirty=0; }
	} VolumeInfo;

	typedef std::map<std::string, VolumeInfo> VolumeMap; // the map of volume name to VolumeInfo

	VolumeMap _volumeMap;
	ZQ::common::Mutex _lockVolumes; // the locker of _volumeMap
*/

//	::TianShanIce::StrValues _dirtyVolumes;
//	ZQ::common::Mutex _lockDirties; // the locker of _dirtyVolumes

	//for response
	std::string _strIP;//local ip
	int			_port;//local port,which SimpleServer listen

	//for request
	std::string _defaultA3Url, _2ndA3Url;

private:

	SYS::SingleObject _hWakeUp;
	bool		_bQuit;
	int64		_stampStarted;

	ZQ::common::NativeThreadPool _thpool;
	int			_interval;

	A3Listener*	_pWebResp;
};

}
}

#endif // __ZQTianShan_NGODStorePortal_H__
