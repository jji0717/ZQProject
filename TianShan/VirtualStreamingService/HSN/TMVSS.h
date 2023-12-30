//gsoap ZQ service name:	TMVSS
//gsoap ZQ service style:	rpc
//gsoap ZQ service encoding:	encoded
//gsoap ZQ service namespace:	http://www.i-zq.com/TMVSS.wsdl
//gsoap ZQ service location:	http://localhost:8895

//gsoap ZQ schema namespace: ZQ:TMVSS
#import "stlvector.h"

/// private pair
typedef struct ZQ__pair
{
	std::string	key;
	std::string value;
}xsd__pair;

/// private map
typedef std::vector<struct ZQ__pair> ZQ__pairVector;
class ZQ__map
{
public:
	ZQ__pairVector _ptr;
	int _size;
	struct soap *soap;
	ZQ__map();
	virtual ~ZQ__map();
	//size_t size();
	//void resize(int);
	//void push_back(struct ZQ__pair &pair);
	//std::string& operator[](std::string);
	//std::string& find(std::string);

//private:
	//static std::string strDefaultValue = "";
}xsd__map;

/// setup a session with TM
class ZQ__setupInfo
{
public:
	/// A map to resource data, Indicates the resource result of the allocation
	ZQ__map															*resource;
	/// A map to param data, The parameters that TianShan received as the application-params
	ZQ__map															*params;
	/// A endpoint for the Server callback
	std::string														cbNotification;
	/// A The context Id must be given when the TreeMain tries to call the callback cbNotification
	std::string														ctxNotification;
	/// A handle to the soap struct that manages this instance (automatically set)
	struct soap														*soap;
};

class ZQ__setupInfoResponse
{
public:
	/// A map to resource data, The confirmation of the input resources
	ZQ__map															*resource;
	/// A string identity for the session of this entertainment run
	std::string														sessionId;
	/// A URL string of TreeMachine for the Polling Machine
	std::string														controlURL;
	/// A return value to show if this call is success, 1=success, 0=fail
	int															ret;
	/// A handle to the soap struct that manages this instance (automatically set)
	struct soap														*soap;
};

typedef enum
{
	UNKNOWN = 0,
	SETUP = 1,
	TEARDOWM = 2
}State;

class ZQ__getStatusInfoResponse
{
public:
	/// A time for next heartbeat time
	long															*upTime;
	/// A status of current session
	State															state;
	/// A string of current session's last error
	std::string														lastError;
	/// A return value to show if this call is success, 1=success, 0=fail
	int															ret;
	/// A handle to the soap struct that manages this instance (automatically set)
	struct soap														*soap;
};

class ZQ__notifyStatusInfo
{
public:
	/// A endpoint for the Server callback
	std::string														ctxNotification;
	/// A The context Id must be given when the TreeMain tries to call the callback cbNotification
	std::string														sessionId;
	/// A status of current session
	State															state;
	/// A string of current session's last error
	std::string														lastError;
	/// A handle to the soap struct that manages this instance (automatically set)
	struct soap														*soap;
};

int ZQ__setup(ZQ__setupInfo* setupInfo, ZQ__setupInfoResponse *setupInfoResponse);
int ZQ__teardown(std::string sessionId, int &ret);
int ZQ__getStatus(std::string sessionId, ZQ__getStatusInfoResponse *getStateInfoResponse);

//callback function
int ZQ__notifyStatus(ZQ__notifyStatusInfo *notifyStatusInfo, int &ret);