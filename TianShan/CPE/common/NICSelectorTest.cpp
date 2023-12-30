#include "NICSelector.h"
#include "CppUTest/UtestMacros.h"
#include "CppUTest/Utest.h"
#include "CppUTest/TestOutput.h"

TEST_GROUP(NetworkIFSelector)
{
	ConsoleTestOutput* pr;
	NetworkIFSelector* netsel;

	TEST_SETUP()
	{
		pr = new ConsoleTestOutput();
		netsel = new NetworkIFSelector();
	}

	TEST_TEARDOWN()
	{
		delete netsel;
		delete pr;
	}
};

TEST(NetworkIFSelector,addInterface)
{
	netsel->addInterface("192.168.81.102",1000000);
	netsel->addInterface("192.168.81.103",1000000);
	netsel->addInterface("192.168.81.104",1000000);
}

TEST(NetworkIFSelector,allocInterface)
{
	std::string strIP;
	netsel->addInterface("192.168.81.102",300);
	netsel->addInterface("192.168.81.103",400);
	netsel->addInterface("192.168.81.104",100);

	netsel->allocInterface(3,strIP);
	pr->print(strIP.c_str());
	pr->print("\n");
	netsel->allocInterface(2,strIP);
	pr->print(strIP.c_str());
	pr->print("\n");
	netsel->allocInterface(1,strIP);
	pr->print(strIP.c_str());
	pr->print("\n");
	
}

TEST(NetworkIFSelector,freeInterface)
{

}