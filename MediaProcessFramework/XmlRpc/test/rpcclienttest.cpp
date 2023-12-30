
#include <stdio.h>

#include "../RpcWpClient.h"
#include "../RpcWpValue.h"
#include "../RpcWpreporter.h"

#include "../../../Common/NativeThread.h"

//using namespace ZQ::rpc;
#include <string>
#include <iostream>
using namespace ZQ::rpc;

class Logger: public RpcErrorHandler
{
private:
	FILE*	m_hFile;
public:
	Logger(const char* strFile)
	{
		m_hFile = fopen(strFile, "w+");
	}

	void error(const char* msg)
	{
		fprintf(m_hFile, "%s\n", msg);
	}
};

class ClientTestThread : public ZQ::common::NativeThread
{
	RpcClient	m_rpcClient;
	FILE*	m_hFile;
public:
	ClientTestThread(const char* ip, int port, const char* strFile)
		:m_rpcClient(ip, port)
	{
		//m_hFile = fopen(strFile, "w+");
	}

	static bool bFirst;
	static int nNumber;

protected:
	int run(void)
	{
		++nNumber;
		//while (true)
		{
			RpcValue noArgs, result;
			if (!m_rpcClient.execute("Hello", noArgs, result))
			{
				if (bFirst)
				{
					bFirst = false;
					printf("!!!!first error in %d\n", nNumber);
				}
				//fprintf(m_hFile, "Can not execute\n");
			}
		}
		return 0;
	}
};
bool ClientTestThread::bFirst = true;
int ClientTestThread::nNumber = 0;

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		std::cerr << "Usage: HelloClient serverHost serverPort\n";
		return -1;
	}
	int port = atoi(argv[2]);
	//Rpc::setVerbosity(5);
	
	setVerbosity(0);

	// Use introspection API to look up the supported methods
	//

	for (int i = 0; i < 100; ++i)
	{
		char strTemp[21] = {0};
		_snprintf(strTemp, 20, "c:\\rpc%d.log", i);
		ClientTestThread* pthread = new ClientTestThread(argv[1], port, strTemp);
		pthread->start();
	}
	
	system("pause");
RpcClient c(argv[1], port);
	RpcValue noArgs, result;
	if (c.execute("system.listMethods", noArgs, result))
	{
		std::cout << "\nMethods:\n ";
		std::cout<< "\n\n";
	}
	else
		std::cout << "Error calling 'listMethods'\n\n";
	
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
	
	getchar();
	return 0;
}
