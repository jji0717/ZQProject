#include "ContentImpl.h"
#include "SimpleServer.h"
#include "A3Request.h"

namespace ZQTianShan {

namespace ContentStore {

#define CSPA3LOG if(_pLog) (*_pLog)


class A3ContentInfo : virtual public Ice::LocalObject//IceUtil::Shared
{
public:
	A3ContentInfo()
		:contentSize(0),supportFileSize(0),bitRate(0){};
	~A3ContentInfo(){};
	typedef ::IceInternal::Handle< A3ContentInfo > Ptr;

	int64		contentSize;				//kbytes
	int64		supportFileSize;			//kbytes
	std::string contentState;
	std::string createDate;
	std::string md5Checksum;
	std::string md5DateTime;
	int			bitRate;

};

//class A3ResponseMsg
//it is the class for web listen
class A3CSCtr;
class A3ResponseMsg : public SimpleServer
{
public:
	A3ResponseMsg(const char* localIP, const int& port, ZQ::common::Log* pLog);
	~A3ResponseMsg(void);

public:
	int handleMsg(const std::string& type, const std::string& msg);
	void setA3Handle(A3CSCtr* pCtr);

private:
	ContentOprtXml	_oprtXml;
	A3CSCtr*		_pCtr;
	ZQ::common::Log* _pLog;
};

//class A3CSCtl
class A3CSCtr : public ZQ::common::NativeThread
{
public:
	A3CSCtr(ContentStoreImpl& store);
	virtual ~A3CSCtr();

	void quit(void);
	std::string getHostURL(){return _strHost;}
protected:
	virtual bool init(void);
	virtual int run(void);
private:
	

	ZQ::common::Log* _pLog;
	HANDLE	_hStop;	


public:
	bool _bQuit;
	//for response
	std::string _strIP;//local ip
	int			_port;//local port,which SimpleServer listen

	//for request
	std::string _strHost;

	HANDLE				_hBegin;
	bool				_bBegin;
	HANDLE				_hHVolume;
	ContentStoreImpl&	_store;
	A3ResponseMsg*		_pWebMsg;

//	std::map<std::string,A3ContentInfo::Ptr> _contentsM;//map for contents
	ZQ::common::Mutex	_lock;

	typedef std::map<std::string,A3ContentInfo::Ptr> CONTENTSMAP;
	typedef struct VolContent
	{
		std::string _strVolName;//volumename
		CONTENTSMAP _contentsM;//contents map
	}VOLCONTENT;

	std::vector<VOLCONTENT> _volContents;
};
}
}
