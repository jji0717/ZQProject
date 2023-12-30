// CSClient.cpp : Defines the entry point for the console application.
//

#include "Parse.h"
#include <string>
#include <iostream>


using namespace std;


int main(int argc, char* argv[])
{
	std::string strCom;
	if(argc < 3)
	{
		cout<<"Please input arguments like this: "<<"ContentStore:tcp -h 192.168.81.114 -p 10400"<<endl;
		return -1;
	}
	for(int i = 1; i < argc; i++)
	{
		strCom += argv[i];
		strCom += " ";
	}
	for(int j = (int)strCom.length(); j>=0; j++)
	{
		if(strCom[j-1] == ' ')
			strCom[j-1] = '\0';
		else
			break;
	}
	Parse parseC;
	if(!parseC.connectSer(strCom))
	{
		cout<<"Connect to server "<<strCom<<" error"<<endl;
		return -1;
	}
	else
		cout<<"Connect to server "<<strCom<<" successful"<<endl;
	
	while(1)
	{
		cout<<">:"<<endl;
		getline(cin,strCom);
		if(strCom.length() == 0)
			continue;
		if(stricmp(strCom.c_str(),"quit") == 0)
			break;
		parseC.parseCom(strCom);
	}
	return 0;
}

