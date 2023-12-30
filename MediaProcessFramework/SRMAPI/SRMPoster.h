
#ifndef _ZQ_SRMPOSTER_H_
#define _ZQ_SRMPOSTER_H_

#include "metasession.h"
#include "metanode.h"

namespace ZQ{namespace MPF{namespace REQPOST{class TaskRequestPoster;}}}

SRM_BEGIN

///SRMRequestPoster
///task request poster
class DLL_PORT SRMRequestPoster
{
private:
	ZQ::MPF::REQPOST::TaskRequestPoster*	m_pPoster;
	char									m_strWorkNodeURL[MAX_URL_LEN];
	utils::URLStr							m_sessionURL;
public:
	///constructor 
	///@param strSessionEntry - session entry path
	///@param nm - node manager
	///@param mgmnodeurl - local url string
	///@param responsTimeout - post time out
	SRMRequestPoster(const char* strSessionEntry, NodeManager& nm,
		const char* mgmnodeurl, size_t time = 0, size_t period = 0, int responsTimeout = NET_SEND_TIME_OUT);
	
	///destructor
	virtual ~SRMRequestPoster();

	///post setup request to remote machine
	///@param tasktype - task type which for request
	///@param result - request result 
	int postSetup(const char* tasktype, rpc::RpcValue& result);
	
	///post a request to remote machine
	///@param useractionid - user defined action id
	///@param param - process parameters
	///@param result[out] - process return
	///@return - post error result
	int postUser(const char* useractionid, const rpc::RpcValue& param, rpc::RpcValue& result);

	///@return - task id
	const char* getTaskId() const;

	///@return - work node id
	const char* getWorkNodeId() const;

	///@return - session url
	const char* getSessionURL();
};

SRM_END

#endif//_ZQ_SRMPOSTER_H_
