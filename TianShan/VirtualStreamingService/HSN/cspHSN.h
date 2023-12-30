#ifndef __CSPHSN_H__
#define __CSPHSN_H__

#include "ContentImpl.h"

namespace ZQTianShan {

namespace ContentStore {

#define CSPCISCOLOG if(_pLog) (*_pLog)


class HSNContentInfo : virtual public Ice::LocalObject//IceUtil::Shared
{
public:
	HSNContentInfo()
		:contentSize(0),supportFileSize(0),bitRate(0){};
	~HSNContentInfo(){};
	typedef ::IceInternal::Handle< HSNContentInfo > Ptr;

	//::std::string	packageName;
	int64			contentSize;
	int64			supportFileSize;
	::std::string	contentState;
	::std::string	createDate;
	::std::string	md5Checksum;
	::std::string	md5DateTime;
	int				bitRate;
};

//class HSNCSCtr
class HSNCSCtr : public ZQ::common::NativeThread
{
public:
	HSNCSCtr(ContentStoreImpl& store);
	virtual ~HSNCSCtr();

	void quit(void);
	std::string getHostURL(){return _strLocalEndpoint;}

protected:
	virtual bool init(void);
	virtual int run(void);

private:
	bool _bQuit;

	ZQ::common::Log* _pLog;
	HANDLE	_hStop;	

public:
	//for request(AIM Notification)
	::std::string	_strLocalEndpoint;	//local end point with protocal header

	//for response(AIM client soap call)
	::std::string	_strRemoteEndpoint;	//remote end point with protocal header

	HANDLE				_hBegin;
	HANDLE				_hHVolume;
	ContentStoreImpl&	_store;

	ZQ::common::Mutex	_lock;

	typedef std::map<std::string,HSNContentInfo::Ptr> CONTENTSMAP;
	typedef struct VolContent
	{
		std::string _strVolName;//volume name
		CONTENTSMAP _contentsM;//contents map
	}VOLCONTENT;

	std::vector<VOLCONTENT> _volContents;
};

}//namespace ContentStore

}//namespace ZQTianShan

#endif __CSPHSN_H__