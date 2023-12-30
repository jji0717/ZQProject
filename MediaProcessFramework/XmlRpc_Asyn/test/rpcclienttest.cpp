
#include <stdio.h>

#include "../RpcWpClient.h"
#include "../RpcWpValue.h"
#include "../RpcWpReporter.h"

//using namespace ZQ::rpc;
#include <string>
#include <iostream>
using namespace ZQ::rpc;

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		std::cerr << "Usage: HelloClient serverHost serverPort\n";
		return -1;
	}
	int port = atoi(argv[2]);
//	setVerbosity(5);
	
	// Use introspection API to look up the supported methods
	RpcClient c(argv[1], port, 0, ST_UDP);

//	c.setUdp();

	RpcValue noArgs, result;
	if (c.execute("system.listMethods", noArgs, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
	else
		std::cout << "Error calling 'listMethods'\n\n";
	
	// Use introspection API to get the help string for the Hello method
	RpcValue oneArg;
	oneArg.clear();
	oneArg.setArray(0, RpcValue("Hello"));
	if (c.execute("system.methodHelp", oneArg, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
	else
		std::cout << "Error calling 'methodHelp'\n\n";
	
	// Call the Hello method
	if (c.execute("Hello", noArgs, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
	else
		std::cout << "Error calling 'Hello'\n\n";
	
	// Call the HelloName method
	oneArg.clear();
	oneArg.setArray(0, RpcValue("Chris"));
	//oneArg[0] = "Chris";
	if (c.execute("HelloName", oneArg, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
	else
		std::cout << "Error calling 'HelloName'\n\n";
	
	// Add up an array of numbers
	RpcValue numbers;
	numbers.setArray(0, RpcValue(33.33));
	numbers.setArray(1, RpcValue(112.57));
	numbers.setArray(2, RpcValue(76.1));
	std::cout << "numbers.size() is " << numbers.size() << std::endl;
	if (c.execute("Sum", numbers, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
    //std::cout << "Sum = " << double(result) << "\n\n";
	else
		std::cout << "Error calling 'Sum'\n\n";
	
	// Test the "no such method" fault
	if (c.execute("NoSuchMethod", numbers, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
	else
		std::cout << "Error calling 'Sum'\n";
	
	// Test the multicall method. It accepts one arg, an array of structs
	RpcValue multicall;
	multicall[0][0]["methodName"] = "Sum";
	multicall[0][0]["params"][0] = 5.0;
	multicall[0][0]["params"][1] = 9.0;
	
	multicall[0][1]["methodName"] = "NoSuchMethod";
	multicall[0][1]["params"][0] = "";
	
	multicall[0][2]["methodName"] = "Sum";
	// Missing params
	
	multicall[0][3]["methodName"] = "Sum";
	multicall[0][3]["params"][0] = 10.5;
	multicall[0][3]["params"][1] = 12.5;
	
	if (c.execute("system.multicall", multicall, result))
	{
		char strXml[1024] = {0};
		result.toXml(strXml, 1024);
		std::cout << "\nMethods:\n ";
		std::cout<< strXml<<"\n\n";
	}
	else
		std::cout << "\nError calling 'system.multicall'\n";

//	getchar();
	return 0;
}
