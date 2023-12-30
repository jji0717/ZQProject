
#include "../../srmapi/MetaSession.h"
#include "../../srmapi/MetaTask.h"
#include "string.h"
#include "nativethread.h"

	using namespace ZQ::MPF::SRM;

const char* TEST_STRING[] = 
{
	"hello",
	"java",
	"i",
	"love",
	"littlewhite"
};

int g_index = 0;

void writeDB()
{
	g_index = (g_index+1)%5;
	char strEntry[256] = {0};
	sprintf(strEntry, "//test/aasdf/test_%s", TEST_STRING[g_index]);
	MetaRecord mr(strEntry, 0);

	for (int i =0; i < 100; ++i)
	{
		char strTemp[256] = {0};
		sprintf(strTemp, "DB_INC_%d", i);
		if (!mr.set(strTemp, strTemp))
		{
			printf("can not set %s to %s\n", strTemp, strEntry);
		}
	}
}

class TestThread : public ZQ::common::NativeThread
{
protected:
	int run(void)
	{
		writeDB();

		return 0;
	}
public:
};

class TestLock : public ZQ::common::NativeThread
{
private:
	static ZQ::common::Mutex _lock;
protected:
	int run(void)
	{
		//ZQ::common::Guard<ZQ::comextra::Event> lock(_lock);
		Sleep(10000);

		printf("abcdefghijklmnopqrstuvwxyz1234567890ilovelittlewhite;;;;;;;aqwvzcvasdfbuopzxbsdgfqrgnxzcbvzcvwaqgtopsdsdfghabz.df,adfhadfgsdfhasdaf;asdgasdgsdfghbcvbxcbxbvsdgfasdgfasdfasdfwetert\n");

		return 0;
	}
};

ZQ::common::Mutex TestLock::_lock;

int main()
{
	SRMStartup();

	time_t curtm;
	srand(time(&curtm));

	//writeDB();

	/*
	TestThread aa[50];
	for (int i = 0; i < 50; ++i)
		aa[i].start();
		*/

	{
		MetaTask session1("haha");
		MetaTask session2("hahasda");
		MetaTask session3("hasefaha");
		MetaTask session4("asdfhaha");
	}
/*
	//===============get is ok====================
	char strTemp[256] = {0};
	if (NULL == mr.get("aa", strTemp, 256))
	{
		printf("can not get data from database\n");
		return -3;
	}
	printf("aa = %s\n", strTemp);
	*/
	/*
	//===============set many data is ok====================
	for (int i = 0; i < 5000; ++i)
	{
		std::string strTemp = ProcuceExclusiveString();
		if (!mr.set(strTemp.c_str(), strTemp.c_str()))
		{
			printf("can not set data to database\n");
			return -4;
		}
	}
	*/
	
	/*
	//===============set nesting data is ok======================
	MetaRecord mr1(rm, "test/aa");
	MetaRecord mr2(rm, "test/bb");
	MetaRecord mr3(rm, "test/cc");
	MetaRecord mr4(rm, "test/dd");
	MetaRecord mr5(rm, "test/ee");
	MetaRecord mr6(rm, "test/ff");

	mr1.set("aa", "dd");
	mr2.set("aa", "ff");
	mr3.set("aa", "hh");
	mr4.set("aa", "jj");
	mr5.set("aa", "kk");
	mr6.set("aa", "ll");
	*/
	
	//===============get nesting data is ok======================
	/*
	MetaRecord mrtemp(rm, "");
	if (!mr.getFirstChild(mrtemp))
	{
		printf("can not get first child\n");
		return -5;
	}
	char strTemp[256] = {0};
	char strEntry[256] = {0};
	mrtemp.getEntry();
	if (mrtemp.get("aa", strTemp, 256))
	{
		printf("get data %s from %s\n", strTemp, strEntry);
	}
	
	while (mrtemp.getSibling(mrtemp))
	{	
		char strTemp[256] = {0};
		char strEntry[256] = {0};
		mrtemp.getEntry();
		if (mrtemp.get("aa", strTemp, 256))
		{
			printf("get data %s from %s\n", strTemp, strEntry);
		}
	}
	*/
	
	/*
	//==================set nesting data2 is ok====================
	for (int i = 0; i < 2; ++i)
	{
		if (0 == i)
		{
			MetaRecord mr1(rm, "test1");
			mr1.set("1", "1");
		}
		else
		{
			MetaRecord mr2(rm, "test2");
			mr2.set("2", "2");
		}
	}
	*/

	system("pause");
	SRMCleanup();
	return 0;
}
