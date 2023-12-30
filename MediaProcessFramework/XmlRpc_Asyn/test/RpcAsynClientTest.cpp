
#include <stdio.h>

#include "../RpcWpReporter.h"
#include "../RpcWpAsynClient.h"
#include "../RpcWpValue.h"
#include "../xmlrpc_plus/src/xmlrpcvalue.h"

//using namespace ZQ::rpc;
#include <string>
#include <iostream>
#include <vector>

using namespace std;
using namespace ZQ::rpc;


vector<string> vec_result;

ZQ::common::Mutex g_mt;

void response_handle(const char * server_url, 
                           const char * method_name, 
                           RpcValue &params, 
                           RpcValue &result)
{
	ZQ::common::Guard<ZQ::common::Mutex> gd(g_mt);
	std::cout << "enter response callback funciton" << std::endl;
	char szBuff[8192];
	result.toXml(szBuff, 8192);
	vec_result.push_back(szBuff);
	std::cout << "leave response callback funciton" << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		std::cerr << "Usage: HelloClient serverHost serverPort\n";
		return -1;
	}
	int port = atoi(argv[2]);
	//setVerbosity(5);
	
	// Use introspection API to look up the supported methods
	RpcAsynClient c(argv[1], port, 0, ST_UDP);

	//c.setUdp();

	for(int i = 0; i < 10; ++i)
	{
	cout << i << endl;
	RpcValue noArgs;
	c.execute("system.listMethods", noArgs, response_handle);
	}

	std::cout << "befor waitAsyn()" << std::endl;
	c.waitAsyn();
	std::cout << "after waitAsyn()" << std::endl;

	vector<string>::iterator it;
	cout << "whole result: " << endl;
	for(it = vec_result.begin(); it != vec_result.end(); ++it)
	{
		cout << *it << endl;
	}
	cout << vec_result.size() << endl;


/*	
	// Use introspection API to get the help string for the Hello method
	RpcValue oneArg;
	oneArg.clear();
	oneArg.setArray(0, RpcValue("Hello"));
	if (c.execute("system.methodHelp", oneArg, result))
	{
		std::cout << "Help for 'Hello' method: ";
		std::cout<< "\n\n";
	}
	else
		std::cout << "Error calling 'methodHelp'\n\n";
	
	// Call the Hello method
	if (c.execute("Hello", noArgs, result))
	{
		std::cout<<"Method Hello: ";
		std::cout<<"\n\n";
	}
	else
		std::cout << "Error calling 'Hello'\n\n";
	
	// Call the HelloName method
	oneArg.clear();
	oneArg.setArray(0, RpcValue("Chris"));
	//oneArg[0] = "Chris";
	if (c.execute("HelloName", oneArg, result))
	{
		//ANSIString str = oneArg[0];
		char str[1024] = {0};
		RpcValue arg1 = oneArg[0];
		arg1.ToString(str, 1024);
		printf("ahahhahah: %s\n", str);
		std::cout<<"Method HelloName: ";
		std::cout<<"\n\n";
	}
	else
		std::cout << "Error calling 'HelloName'\n\n";
	
	// Add up an array of numbers
	RpcValue numbers;
	numbers[0] = 33.33;
	numbers[1] = 112.57;
	numbers[2] = 76.1;
	std::cout << "numbers.size() is " << numbers.size() << std::endl;
	if (c.execute("Sum", numbers, result))
	{
		std::cout<<"Method Sum: ";
		std::cout<<"\n\n";
	}
    //std::cout << "Sum = " << double(result) << "\n\n";
	else
		std::cout << "Error calling 'Sum'\n\n";
	
	// Test the "no such method" fault
	if (c.execute("NoSuchMethod", numbers, result))
	{
		std::cout << "NoSuchMethod call: fault: " << c.isFault() << ", result = " ;
		std::cout<<std::endl;
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
		std::cout << "\nmulticall  result = ";
		std::cout<<std::endl;
	}
	else
		std::cout << "\nError calling 'system.multicall'\n";
*/	
//	getchar();
	return 0;
}
