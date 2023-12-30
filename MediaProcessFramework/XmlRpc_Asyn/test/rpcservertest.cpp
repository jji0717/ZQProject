
#include "../RpcWpServer.h"
#include "../RpcWpValue.h"
#include "../RpcWpServerMethod.h"
#include "../RpcWpreporter.h"

#include <stdio.h>
#include <iostream>
#include <string>

using namespace ZQ::rpc;

RpcServer s(ST_UDP);

// No arguments, result is "Hello".
class Hello : public RpcServerMethod
{
public:
  Hello(RpcServer* s) : RpcServerMethod("Hello", s) {}

  void execute(RpcValue& params, RpcValue& result)
  {
    result = "Hello";
  }

  char* help(char* strBuffer, int nMax)
  {
	  return strncpy(strBuffer, "Say hello", nMax);
  }

} hello(&s);    // This constructor registers the method with the server


// One argument is passed, result is "Hello, " + arg.
class HelloName : public RpcServerMethod
{
public:
  HelloName(RpcServer* s) : RpcServerMethod("HelloName", s) {}

  void execute(RpcValue& params, RpcValue& result)
  {
    std::string resultString = "Hello, ";

	char strTemp[1024] = {0};
	RpcValue param1 = params[0];
	param1.ToString(strTemp, 1024);
    resultString += std::string(strTemp);

    result = resultString.c_str();
	printf("haha\n");
  }
} helloName(&s);


// A variable number of arguments are passed, all doubles, result is their sum.
class Sum : public RpcServerMethod
{
public:
  Sum(RpcServer* s) : RpcServerMethod("Sum", s) {}

  void execute(RpcValue& params, RpcValue& result)
  {
    int nArgs = params.size();
    double sum = 0.0;
    for (int i=0; i<nArgs; ++i)
      sum += double(params[i]);
    result = sum;
  }
} sum(&s);


int main(int argc, char* argv[])
{
	if (argc != 2) {
    std::cerr << "Usage: HelloServer serverPort\n";
    return -1;
  }
  int port = atoi(argv[1]);

  setVerbosity(5);

  // Create the server socket on the specified port
  s.bindAndListen(port);

  // Enable introspection
  s.enableIntrospection(true);

  // Wait for requests indefinitely
  s.work(-1.0);


  return 0;
}
