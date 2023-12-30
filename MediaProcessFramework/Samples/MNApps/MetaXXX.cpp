
#include "../../srmapi/MetaSession.h"
#include "../../srmapi/MetaTask.h"
#include "../../srmapi/MetaResource.h"
#include "../../srmapi/MetaNode.h"
#include "../../MPFException.h"
#include "../../mpfloghandler.h"

using namespace ZQ::MPF::SRM;

class MyLog : public ZQ::MPF::MPFLogHandler
{
public:
	void writeMessage(const char* msg)
	{
		printf("%s\n", msg);
	}
};

class PersonManager : public ResourceManager
{
public:
	PersonManager(MPFDatabase& database)
		:ResourceManager(database, "Person")
	{
	}

	virtual BYTE score(MetaResource& dbrts, MetaSession& sess)
	{
		char strScoreNumber[256] = {0};
		int nScore = atoi(dbrts.get("score", strScoreNumber, 256));
		
		return (BYTE)nScore;
	}
};

int main(void)
{
	MyLog log;
	log.setVerbosity(6);
	log.setLogHandler(&log);

	MPFDatabase db;

	if (!db.connect("edbb4://localhost/c:\\test.db"))
	{
		printf("can not connect to db\n");
		return -1;
	}

	SessionManager sm(db);

	/*
	//=============create session is ok===============
	for (int i = 0; i < 2; ++i)
	{
		MetaSession ss(sm);
		char strId[256] = {0};
		printf("create session %s\n", ss.getId(strId, 256));

		std::string strKey = ProcuceExclusiveString();
		std::string strValue = ProcuceExclusiveString();

		ss.set(strKey.c_str(), strValue.c_str());

		for (int j = 0; j < 10; ++j)
		{
			if (ss.set(ProcuceExclusiveString().c_str(), j))
			{
				printf("set session %s in position %d\n", strId, j);
			}
		}
	}
*/

	/*
	//==========simple test is ok=============
	for (int i = 0; i < 2; ++i)
	{
		if (0 == i)
		{
			MetaSession ss(sm, "test");
			ss.set("1", "1");
		}
		else
		{
			MetaSession ss(sm, "test2");
			ss.set("2", "2");
		}
		//MetaSession ss2(sm, "test2");
		//ss1.set("1", "1");
		//ss2.set("2", "2");
	}
	*/

	/*
	//=====================test meta task is ok==================
	TaskManager tm(db);

	std::string strTaskId = ProcuceExclusiveString();
	MetaTask mt(tm, strTaskId.c_str());

	char strEntryName[256] = {0};
	printf("current entry: %s\n", mt.getEntry());

	char strRecordRoot[256] = {0};
	printf("task root: %s\n", tm.getEntry());
	*/

	/*
	//=====================test resource is ok=========================
	try
	{
		PersonManager personmgr(db);

		MetaSession msson(sm);

		MetaResource mrp(personmgr, NULL);
		char strResourceEntry[256] = {0};
		personmgr.allocate(msson, strResourceEntry, 256);

		printf("allocated a resource in entry: %s\n", strResourceEntry);
	}
	catch(ZQ::MPF::SRMException& e)
	{
		printf("!!!get an exception: %s\n", e.what());
	}
	*/

	//=====================test node is ok==========
	ZQ::rpc::RpcServer sv;
	sv.bindAndListen(12000);

	NodeManager mg(sv, db, 200);

	mg.start();


//	MetaNode mn(mg, "msh1");
//	MetaNode mn(mg, "msh2");
//	MetaNode mn(mg, "msh3");

	sv.work(-1.0);
	

	system("pause");
	return 0;
}

