
#include "srmposter.h"
#include "MetaTask.h"
#include "listinfo.h"
#include "taskrequestposter.h"
#include "MPFLogHandler.h"


SRM_BEGIN

SRMRequestPoster::SRMRequestPoster(const char* strSessionEntry, NodeManager& nm,
								   const char* mgmnodeurl, size_t time, size_t period, 
								   int responsTimeout)
:m_pPoster(NULL), m_sessionURL(mgmnodeurl)
{
	assert(strSessionEntry);

	std::string strResourceCount = utils::NodePath::getSubPath(strSessionEntry, RESOURCE_COUNT);
	MetaRecord rescount(strResourceCount.c_str(), PM_PROP_READ_ONLY);

	ActionList al;
	char strTemp[256] = {0};
	for (int i = 0; NULL != rescount.getKey(i, strTemp, 256); ++i)
	{
		if (0 == strncmp(strTemp, HIDDEN_ATTR, strlen(HIDDEN_ATTR)))
			continue;

		ActionInfo ai;
		ai.sub_res	= strTemp;
		ai.number	= rescount.get(strTemp);

		al.push_back(ai);
	}

	//char worknodeurl[MAX_URL_LEN] = {0};
	if (!nm.allocate(strSessionEntry, m_strWorkNodeURL, MAX_URL_LEN, al, time, time+period))
	{
		MPFLog(MPFLogHandler::L_ERROR, "SessionEntry:%s, WorkNodeUrl:%s, Node allocate fail.", strSessionEntry, m_strWorkNodeURL);
		return;
	}

	m_pPoster = new REQPOST::TaskRequestPoster(m_strWorkNodeURL, mgmnodeurl, responsTimeout);

	m_sessionURL.setPath(URL_PATH_SESSION);
	m_sessionURL.setVar(URL_VARNAME_SESSION_ID, utils::NodePath::getPureName(strSessionEntry).c_str());
}

SRMRequestPoster::~SRMRequestPoster()
{
	if (NULL != m_pPoster)
		delete m_pPoster;
}

int SRMRequestPoster::postSetup(const char* tasktype, rpc::RpcValue& result)
{
	if (NULL == m_pPoster)
		return POST_ERR_NOTALLOCATED;//can not find allocated poster instance
	int nRtn = m_pPoster->TaskRequestPoster::postSetup(tasktype, result);

	char strFullTaskId[MPF_FULL_TASKID_LEN] = {0};
	_snprintf(strFullTaskId, MPF_FULL_TASKID_LEN-1, "%s.%s", getWorkNodeId(), getTaskId());
	MetaTask task(strFullTaskId);
	time_t curtime;
	time(&curtime);
	task.set(INFO_STARTTIME_KEY, (size_t)curtime);
	task.set(TASK_STATUS_KEY, TASK_STATUS_INIT);
	task.set(INFO_TYPENAME_KEY, tasktype);

	utils::URLStr taskUrl(m_strWorkNodeURL);
	taskUrl.setPath(URL_PATH_TASK);
	taskUrl.setVar(URL_VARNAME_TASK_ID, getTaskId());

	task.set(INFO_TASKURL_KEY, taskUrl.generate());
	task.set(INFO_SESSIONURL_KEY, m_sessionURL.generate());

	return nRtn;
}

int SRMRequestPoster::postUser(const char* useractionid, const rpc::RpcValue& param, rpc::RpcValue& result)
{
	if (NULL == m_pPoster)
		return POST_ERR_NOTALLOCATED;
	return m_pPoster->postUser(useractionid, param, result);
}

const char* SRMRequestPoster::getTaskId() const
{
	if (NULL == m_pPoster)
		return NULL;
	return m_pPoster->getTaskId();
}

const char* SRMRequestPoster::getWorkNodeId() const
{
	if (NULL == m_pPoster)
		return NULL;
	return m_pPoster->getWorkNodeId();
}

const char* SRMRequestPoster::getSessionURL()
{
	return m_sessionURL.generate();
}

SRM_END
