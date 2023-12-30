#include <stdlib.h>
#include <string>
#include <iostream>
#include <boost/regex.hpp>

using namespace std;


// process_ftp:
// on success returns the ftp response code, and fills
// msg with the ftp response message.
int process_ftp(const char* response, boost::regex& expression)
{
	typedef boost::match_results<std::string::const_iterator> res_t;
	res_t results;
	
	std::string strMsg;
	if(boost::regex_match(response, results, expression))
	{
		// what[0] contains the whole string
		// what[1] contains the response code
		// what[2] contains the separator character
		// what[3] contains the text message.
		for (int i = 0; i < results.size(); ++i)
		{
			strMsg.assign(results[i].first, results[i].second);
			cout<<"message"<<i<<" : "<<strMsg<<endl;
		}
		return atoi(results[1].first);
	}
	// failure did not match
	return -1;
}

int main()
{
	cout<<"Regular syntax tester\n(C) Copyright 2005 SeaChange\n"<<endl;

	boost::regex				expression;
start:
	cout<<"Please input Regular syntax line:>"<<flush;
	char strRegular[256] = {0};
	gets(strRegular);

	try
	{
		expression.assign(strRegular);
	}
	catch(boost::bad_expression& ex)
	{
		cout<<"Syntax error : "<<ex.what()<<endl;
		goto start;
	}

	std::string in;
	while(true)
	{
		cout << "(\"quit\" enter test line):>" << flush;
		//std::getline(cin, in);
		char strGet[256] = {0};
		gets(strGet);
		in = strGet;
		if(in == "quit")
			break;
		int result;
		result = process_ftp(in.c_str(), expression);
		if(result != -1)
		{
			cout << "Match found" << endl;
			cout << "	Response code: " << result << endl;
		}
		else
		{
			cout << "Match not found" << endl;
		}
		cout << endl;
	}
	goto start;
	return 0;
}






