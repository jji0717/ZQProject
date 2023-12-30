
#include "srmapi.h"
#include "getopt.h"

using namespace ZQ::MPF::SRM;
#include "zqsafemem.h"
#include "mpfcommon.h"

using ZQ::comextra::sfstrncpy;

//
class Logger : public ZQ::MPF::MPFLogHandler
{
private:
	FILE* m_hFile;
public:
	Logger(const char* logfile)
		:m_hFile(NULL)
	{
		m_hFile = fopen(logfile, "w");
	}

	virtual ~Logger()
	{
		if (m_hFile)
			fclose(m_hFile);
	}

	void writeMessage(const char* msg)
	{
		time_t tm;
		time(&tm);

		//printf("%s\n", msg);
		if (m_hFile)
		{
			//fwrite(msg, strlen(msg), 1, m_hFile);
			char strTemp[1024] = {0};
			_snprintf(strTemp, 1023, "<%ld> %s\n", (long)tm, msg);
			int wirteLen = strlen(strTemp);
			size_t writeCount = fwrite(strTemp, sizeof(char), wirteLen, m_hFile);

//			fputs(" - <%ld>\n", m_hFile);
		}
	}
};

class CopyFileTask : public TaskUpdateSubscriber
{
private:
	std::string	m_strSession;

public:
	CopyFileTask(
		const char* strSessionEntry,
		TaskSubScriberStack& tsss)
		:TaskUpdateSubscriber(tsss), m_strSession(strSessionEntry)
	{
	}

	virtual ~CopyFileTask()
	{}

protected:
	long OnTaskInit(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
	{
		MetaTask task((std::string(strWorkNodeId)+"."+std::string(strTaskId)).c_str());

		printf("received task init method- worknode:%s, task:%s\n", strWorkNodeId, strTaskId);

		//todo: code here
		ZQ::rpc::RpcValue attr = params[0][ATTR_KEY];
		ZQ::rpc::RpcValue expattr = params[0][EXP_ATTR_KEY];

		ZQ::rpc::RpcValue retattr;

		MetaRecord ms(m_strSession.c_str(), PM_PROP_READ_ONLY);

		char strKey[256] = {0};
		char strValue[256] = {0};
		for (int i = 0; i < expattr.size(); ++i)
		{
			memset(strValue, 0, 256);
			memset(strKey, 0, 256);

			ms.get(expattr[i].toString(strKey, 256), strValue, 256);

			retattr.setStruct(strKey, ZQ::rpc::RpcValue(strValue));
		}

		result.setStruct(ERROR_CODE_KEY, ZQ::rpc::RpcValue(0));
		result.setStruct(COMMENT_KEY, ZQ::rpc::RpcValue("success"));
		result.setStruct(SESSION_ATTR_KEY, retattr);

		return TaskUpdateSubscriber::SUCC;
	}

	long OnTaskProgress(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
	{

		//printf("received task progress method- worknode:%s, task:%s\n", strWorkNodeId, strTaskId);
		//todo: code here
		//ZQ::rpc::RpcValue attr = params[0][ATTR_KEY];
		//ZQ::rpc::RpcValue expattr = params[0][EXP_ATTR_KEY]; 	

		result.setStruct(ERROR_CODE_KEY, ZQ::rpc::RpcValue(0));
		result.setStruct(COMMENT_KEY, ZQ::rpc::RpcValue("success"));
		//result.setStruct(SESSION_ATTR_KEY, retattr);

		return TaskUpdateSubscriber::SUCC;
	}

	long OnTaskFinal(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
	{

		printf("received task final method- worknode:%s, task:%s\n", strWorkNodeId, strTaskId);
		//todo: code here
		//ZQ::rpc::RpcValue attr = params[0][ATTR_KEY];
		//ZQ::rpc::RpcValue expattr = params[0][EXP_ATTR_KEY];

		result.setStruct(ERROR_CODE_KEY, ZQ::rpc::RpcValue(0));
		result.setStruct(COMMENT_KEY, ZQ::rpc::RpcValue("success"));
		//result.setStruct(SESSION_ATTR_KEY, retattr);


		return TaskUpdateSubscriber::SUCC;
	}

	long OnTaskUser(const char* strActionId, const char* strWorkNodeId, const char* strTaskId, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
	{
		//printf("OnTaskUser : action id(%s), work node id(%s), task id(%s)\n", strActionId, strWorkNodeId, strTaskId);

		return TaskUpdateSubscriber::HANDLED_NEXT;
	}
};

void usage()
{
	printf("-a ip_address -p port_number\n");
}

void about()
{
	printf("SRM API Tester 0.1\n\n");
	printf("this is only for SRM API test\n");
	printf("Author: Little White\n");
	printf("Date: 2005-5-25\n");
}

__declspec( dllimport ) SessionManager* ZQ::MPF::SRM::g_mpf_session_manager_instance;

int main(int argc, char* argv[])
{
	SRMStartup();
	
	// parse the command options
	if (argc <5)
	{
		usage();
		return -1;
	}

	int ch;
	
	char wip[MAX_PATH];
	int wport = 0; 

	ZeroMemory(wip,MAX_PATH);
	
	while((ch = getopt(argc, argv, "p:a:h:H")) != EOF)
	{
		switch (ch)
		{
		case 'p'://task acceptor port
			if(optarg==0)exit(1);
			if(*optarg==0)exit(1);
			if ((wport = atoi(optarg)) <=0)
			{
				printf("Error: illegal work node port specified!\n");
				exit(1);
			}
			break;
		case 'a'://work node ip
			if(optarg==0)exit(1);
			if(*optarg==0)exit(1);
			if(*optarg==0)
			{
				printf("Error: illegal work node ip specified!\n");
				exit(1);
			}
			else
				sfstrncpy(wip,optarg,MAX_PATH);
			break;
		case '?':
		case 'h':
		case 'H':
			usage();
			return 0;
		}
	}

	{
	Logger log("c:\\srm.log");
	ZQ::MPF::MPFLogHandler::setLogHandler(&log);

	using namespace ZQ::MPF::SRM;

	using ZQ::MPF::MPFLogHandler;

	SRMDaemon daemon(wip, wport);

	NodeManager nm(daemon);
	nm.begin();

	daemon.start();

	MetaSession sess;

	sess.set(SESS_PARAM_ATLEAST_TRAFFIC, size_t(0));
	sess.set(SESS_PARAM_LEASETERM, 60000);
	sess.set(SESS_PARAM_TASKTYPE, "CopyFileWork");
	sess.set("CopyFileWork.SrceFile", "c:\\test.db");
	sess.set("CopyFileWork.DestFile", "d:\\test.db");

	std::string strResourceCount = ZQ::MPF::utils::NodePath::getSubPath(sess.getEntry(), RESOURCE_COUNT);
	MetaRecord resCount(strResourceCount.c_str());

	sess.print(print_screen);

	system("pause");

	printf("start deamon!\n");

	char managerUrl[MAX_PATH] = {0};
	_snprintf(managerUrl, MAX_PATH-1, "MPF://%s:%d/session?", wip, wport);


	//==========test update session
	TaskSubScriberStack tsm(daemon);

	ZQ::rpc::RpcValue setupResult;
	CopyFileTask cft(sess.getEntry(), tsm);

	SRMRequestPoster poster(sess.getEntry(), nm, managerUrl);

	cft.open();

	if (0 == poster.postSetup("CopyFileWork", setupResult))
	{
		while(true)
		{
			if (cft.isFinal())
				break;

			Sleep(255);
		}
	}
	else
		printf("can not post setup task request to work node\n");

	cft.close();
	daemon.shutdown();

	}

	SRMCleanup();

	system("pause");
	return 0;
}
