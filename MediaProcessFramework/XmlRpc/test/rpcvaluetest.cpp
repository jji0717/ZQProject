
#include "../RpcWpValue.h"
#include "../RpcWpreporter.h"
#include "../RpcWpReporter.h"
#include <stdio.h>

using namespace ZQ::rpc;

void TestErrorHandler(void)
{
	RpcErrorHandler* err = RpcErrorHandler::getErrorHandler();
	err->error("test ErrorHandler ok");
}

void TestLogHandler(void)
{
	RpcLogHandler* log = RpcLogHandler::getLogHandler();
	log->log(0, "test LogHandler ok");
}

void TestVerbosity(void)
{
	setVerbosity(5);
	printf("%d\n", getVerbosity());
}

void TestRpcValueInt(void)
{
	RpcValue val((int)75);

	char* strXml = new char[256];
	val.toXml(strXml, 256);
	printf("%s\n", strXml);
	delete[] strXml;
}

void TestRpcValueBool(void)
{
	RpcValue val((bool)true);
	
	char* strXml = new char[256];
	val.toXml(strXml, 256);
	printf("%s\n", strXml);
	delete[] strXml;
}

void TestRpcValueDouble(void)
{
	RpcValue val((double)42234.7545);
	
	char* strXml = new char[256];
	val.toXml(strXml, 256);
	printf("%s\n", strXml);
	delete[] strXml;
}

void TestRpcValueDoubleFormat(void)
{
	char* strFmt = new char[256];
	RpcValue::getDoubleFormat(strFmt, 256);
	printf("%s\n", strFmt);
	delete[] strFmt;

	RpcValue::setDoubleFormat("%2.2f");
}

void TestRpcValueString(void)
{
	RpcValue val("i love yane");
	
	char* strXml = new char[256];
	val.toXml(strXml, 256);
	printf("%s\n", strXml);
	delete[] strXml;
}

void TestRpcValueArray(void)
{
	RpcValue val;
	val.setArray(0, RpcValue(5));
	val.setArray(1, RpcValue(true));
	val.setArray(2, RpcValue(23.435));
	RpcValue attr;
	val.setArray(3, attr);
	val.setArray(4, RpcValue("adsf"));
	
	char* strXml = new char[1046];
	val.toXml(strXml, 1046);
	printf("%s\n", strXml);
	delete[] strXml;

	//int a = val[0];
	//printf("%d\n", a);
}

void TestRpcValueStruct(void)
{
	RpcValue val;
	val.setStruct("I", RpcValue(5));
	val.setStruct("Love", RpcValue(true));
	val.setStruct("Yane", RpcValue("adsf"));
	val.setStruct("ss", RpcValue("i love yane"));
	
	RpcValue temp;
	RpcValue::Type tp = temp.getType();
	temp = "adsf";

	tp = temp.getType();
	
	char* strXml = new char[1046];
	val.toXml(strXml, 1046);
	printf("%s\n", strXml);
	delete[] strXml;
}

void TestConv(void)
{
	RpcValue result = RpcValue("Hello, Chris");
	//delete[] strTemp.data;

	char strFmt[256] = {0};

	printf("%s", result.ToString(strFmt, 256));

}

int main(void)
{
	/*
	TestErrorHandler();

	TestLogHandler();

	TestVerbosity();


	TestRpcValueInt();

	TestRpcValueBool();

	TestRpcValueString();

	TestRpcValueDouble();

	TestRpcValueDoubleFormat();
	TestRpcValueDouble();

  */

	TestRpcValueStruct();

	/*
	TestRpcValueStruct();
*/
	char c;
	scanf("%c", &c);
	return 0;
}
