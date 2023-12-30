#include "ContentImpl.h"
#include "CiscoAIMSoap11Impl.h"
#include "adiXMLDoc.h"

namespace ZQTianShan {

namespace ContentStore {

#define CSPCISCOLOG if(_pLog) (*_pLog)


class CiscoAIMContentInfo : virtual public Ice::LocalObject//IceUtil::Shared
{
public:
	CiscoAIMContentInfo()
		:contentSize(0),supportFileSize(0),bitRate(0){};
	~CiscoAIMContentInfo(){};
	typedef ::IceInternal::Handle< CiscoAIMContentInfo > Ptr;

	//::std::string	packageName;
	int64			contentSize;
	int64			supportFileSize;
	::std::string	contentState;
	::std::string	createDate;
	::std::string	md5Checksum;
	::std::string	md5DateTime;
	int				bitRate;

};

//class CiscoAIMCSCtr
class CiscoAIMCSCtr : public ZQ::common::NativeThread
{
public:
	CiscoAIMCSCtr(ContentStoreImpl& store);
	virtual ~CiscoAIMCSCtr();

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

	::ZQTianShan::CVSS::CiscoAIMSoap11Impl	*_ciscoAIMSoap11Impl;
	ADIXMLDoc			_adiXMLDoc;

	ZQ::common::Mutex	_lock;

	typedef std::map<std::string,CiscoAIMContentInfo::Ptr> CONTENTSMAP;
	typedef struct VolContent
	{
		std::string _strVolName;//volume name
		CONTENTSMAP _contentsM;//contents map
	}VOLCONTENT;

	std::vector<VOLCONTENT> _volContents;
};

}//namespace ContentStore

}//namespace ZQTianShan
