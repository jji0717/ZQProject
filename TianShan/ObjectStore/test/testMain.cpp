#include "RedisClient.h"
#include "FileLog.h"
#include "NativeThreadPool.h"

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>

ZQ::common::FileLog mylog("./redistest.log", ZQ::common::Log::L_DEBUG);
ZQ::common::NativeThreadPool thpool;

class DummyReply : public ZQ::common::RedisSink
{
public:
	DummyReply() {}

protected:
	virtual void OnRequestError(ZQ::common::RedisClient& client, ZQ::common::RedisCommand& cmd, ZQ::common::RedisSink::Error errCode, const char* errDesc=NULL) {}
	virtual void OnReply(ZQ::common::RedisClient& client, ZQ::common::RedisCommand& cmd, Data& data) {}
};

typedef unsigned int oid;
std::string oidToString(const oid* Oid, uint OidLen)
{
	std::string ret;
	char buf[16];
	for (int i=0; Oid && i< OidLen; i++)
	{
		snprintf(buf, sizeof(buf)-2, ".%d", Oid[i]);
		ret += buf;
	}

	return ret;
}

uint stringToOid(const char* strOid, oid* Oid, uint OidMaxLen)
{
	if (NULL == strOid || NULL ==Oid || OidMaxLen <=0)
		return 0;
	std::string str = strOid;
	int i=0;
	for (i =0; i < OidMaxLen && !str.empty(); i++)
	{
		size_t pos = str.find('.');
		if (std::string::npos != pos)
			str = str.substr(pos+1);
		Oid[i] = atol(str.c_str());
	}

	return i;
}

void redistest()
{
	oid aaa[] = {1,2,3,4,5};
	std::string stra = oidToString(aaa, 5);
	memset(aaa, 0, sizeof(aaa));
	stringToOid(stra.c_str(), aaa, 4);


	const char *keyv[] = {"kalle", "adam", "unknown", "bertil", "none"};
	char data[100] = "\"abc\"\12\03%";
	char data2[200]="";
	uint len;
	std::string str;
	ZQ::common::RedisClient::encode(str, data);
	memset(data, 0x00, sizeof(data));
	ZQ::common::RedisClient::decode(str.c_str(), data, sizeof(data)-2);

	ZQ::common::RedisClient::Ptr client = new ZQ::common::RedisClient(mylog, thpool, "localhost");
	client->setClientTimeout(999999,1999999);
	ZQ::common::RedisCommand::Ptr cmd;
	//cmd = client->sendPING();
	//cmd = client->sendINFO();
	//cmd = client->sendSET("key1", data,100);
	//cmd = client->sendGET("key1");
	//cmd = client->sendGETSET("key1", data,100);
	// cmd = client->sendSADD("set1", data,100);
	//cmd = client->sendSADD("set1", "1234");
	//cmd = client->sendSMEMBERS("set1", NULL);
	//cmd = client->sendSREM("set1", data,100);
	//cmd = client->sendSMEMBERS("set1", NULL);

	len = sizeof(data2);
	client->SET("key1", (uint8*)data, 100);
	client->GET("key1", (uint8*)data2, len);
	ZQ::common::RedisSink::Ptr reply = new DummyReply();
	
/*	for (int i=0; i < 999999; i++)
	{
		snprintf(data2, sizeof(data2)-2, "member%08d", i);
		client->sendSADD("set1", (char*) data2, 14, reply);
		// client->sendSET(data2, (const uint8*)data2, strlen(data2), reply);
	}
*/
	ZQ::common::StringList members;
	int64 stampStart = ZQ::common::now();
	client->SMEMBERS("set1", members);
//	client->SMEMBERS("set1", members);
	int64 diff = ZQ::common::now() - stampStart;

	Sleep(100000);
}

#include "../ObjectStore.h"
#include "./TestStore.h"

/*
class TestServant : public ZQ::ObjectDB::ObjectServant
{
public:
	TestServant() : ZQ::ObjectDB::ObjectServant() {}

	std::string _body;
	int         _int1;
};

class TestStore : public ZQ::ObjectDB::ObjectStore
{
public:
    TestStore(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, const std::string& dbUrl, const ZQ::ObjectDB::Index::List& indices =ZQ::ObjectDB::Index::List())
		: ObjectStore("TestStore", log, thrdpool, dbUrl, indices)
	{}

protected:
	bool marshal(const ZQ::ObjectDB::StoredObject::Ptr& so, std::string& ostream)
	{
		if (NULL == so || NULL == so->_servant)
			return false;

		TestServant* svnt= (TestServant*) so->_servant.get();

		ostream = svnt->_body;
		return true;
	}

	bool unmarshal(ZQ::ObjectDB::StoredObject::Ptr& so, const std::string& istream)
	{
		TestServant* svnt= new TestServant();
		svnt->_body = istream;
		so->_servant = svnt;
		return true;
	}
};

// -----------------------------
// class TestIndex
// -----------------------------
class TestIndex : public ZQ::ObjectDB::Index
{
public:
	typedef POINTER< TestIndex > Ptr;

	TestIndex() : ZQ::ObjectDB::Index("TestIndex") {}

	virtual ZQ::ObjectDB::Identities find(const std::string& body) const
	{
		std::string s = body; // TODO call	Store::marshalKey(body, s)
		return untypedFind(s);
	}

protected:

	virtual std::string accessKey(const ZQ::ObjectDB::ServantPtr& servant) const
	{
		TestServant* svnt= (TestServant*) servant.get();
		if (NULL == svnt)
			return false;

		std::string ostream = svnt->_body;
		return ostream;
	}

};

*/

#define TestServant TianShanIce::TestClass

void osfill(ZQ::ObjectDB::ObjectStore::Ptr store)
{
	mylog(ZQ::common::Log::L_INFO, "osfill() ********");

	for (int i=0; i<999; i++)
	{
		try {
			TestServant* sv2 = new TestServant();
			char buf[10];
			snprintf(buf, sizeof(buf)-2, "T%03d", i);
			sv2->id = "ABCD0";
			store->remove(buf);
			ZQ::ObjectDB::StoredObject::Ptr so1 = store->add(sv2, buf);
			store->close(so1);
			// store->remove(buf);
		}
		catch(const ZQ::ObjectDB::ObjectStoreException&) {}
	}
}

void osupdate(ZQ::ObjectDB::ObjectStore::Ptr store)
{
	mylog(ZQ::common::Log::L_INFO, "osupdate() ********");
	ZQ::ObjectDB::StoredObject::Ptr so;

	for (int i=0; i<999; i++)
	{
		char buf[10];
		snprintf(buf, sizeof(buf)-2, "T%03d", i);
		std::string id = buf;
		bool bUpdated = false;

		try {
			TianShanIce::TestClassPtr sv = TianShanIce::TestClassPtr::dynamicCast(store->locate(id, so));
			if (sv)
			{
				snprintf(buf, sizeof(buf)-2, "V%03d", i);
				sv->id = buf;
				bUpdated = true;
			}

			store->close(so, bUpdated);
		}
		catch(const ZQ::ObjectDB::ObjectStoreException&) {}
	}
}

void osopen(ZQ::ObjectDB::ObjectStore::Ptr store)
{
	mylog(ZQ::common::Log::L_INFO, "osupdate() ********");
	ZQ::ObjectDB::StoredObject::Ptr so;

	for (int i=0; i<999; i++)
	{
		store->locate("T061", so);
	}
}

void ostest()
{
	ZQ::ObjectDB::Index::List indices; 
	TianShanIce::TestIdx::Ptr tIdx = new TianShanIce::TestIdx();
	indices.push_back(tIdx);

	mylog(ZQ::common::Log::L_INFO, "================== ostest() starts ==================");

	int i=0;
	Ice::PropertiesPtr proper = Ice::createProperties(i, NULL);
	Ice::CommunicatorPtr ic   = Ice::initializeWithProperties(i, NULL, proper);
	Ice::ObjectAdapterPtr adp = ic->createObjectAdapterWithEndpoints("ABC", "default -p 1234");

	ZQ::ObjectDB::ObjectStore::Ptr store = new TianShanIce::TestClassStore(adp, mylog, thpool, "redis://localhost", indices);
	store->start();

	// ZQ::ObjectDB::ServantPtr sv = svnt;
	// ZQ::ObjectDB::StoredObject::Ptr so = store->add(sv, "123");
	ZQ::ObjectDB::StoredObject::Ptr so;
	TianShanIce::TestClassPtr svnt = TianShanIce::TestClassPtr::dynamicCast(store->locate("123", so));
	if (svnt)
		svnt->id = "HelloW";
	store->close(so);

	ZQ::ObjectDB::ObjectIterator::Ptr oi = store->getIterator();

	while(oi->hasNext())
	{
	   std::string id = oi->next();
	}

	// osfill(store);
	osupdate(store);

	ZQ::ObjectDB::Identities ids = tIdx->find("ABCD");
	ids = tIdx->find("ABCD0");
	std::sort(ids.begin(), ids.end());

	for (ZQ::ObjectDB::Identities::iterator itId = ids.begin(); itId < ids.end(); itId++)
	{
		if (NULL != store->locate(*itId, so))
			printf("%s found\n", itId->c_str());
	}

	// sv =store->remove("123");

	Sleep(10000);
	mylog(ZQ::common::Log::L_INFO, "================== ostest() ends ==================");
}

#include "FileSystemOp.h"

int main(int argc, char **argv)
{
	std::string filename = "2012-01-02T12:24:23";
	// compress the stamp string
	for (size_t pos=0; std::string::npos != (pos = filename.find_first_of("-:+.")); )
		filename.erase(pos, 1);

   ostest();
	return 0;
}
