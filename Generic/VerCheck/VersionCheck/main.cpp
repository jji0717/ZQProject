// VersionCheck.cpp : Defines the entry point for the console application.
//
#include "File.h"
#include "Directory.h"
#include <iostream>
#include <fstream>
using namespace std;

void showHelp()
{
	cout<<"Help"<<endl;
}

int main(int argc, char* argv[])
{
	bool bOutput = false;
	bool bSub = false;
	if(argc!=5)
	{
		showHelp();
		return 0;
	}
	if(strcmp(argv[2],"out")!=0 && strcmp(argv[2],"check")!=0 )
	{
		showHelp();
		return 0;
	}
	if(strcmp(argv[4],"true")!=0 && strcmp(argv[4],"false")!=0 )
	{
		showHelp();
		return 0;
	}
	if(strcmp(argv[2],"out")==0)
	{
		bOutput = true;
	}
	if(strcmp(argv[4],"true")==0)
	{
		bSub = true;
	}
	try
	{
		std::string directory,query;
		Directory pSystem(argv[1]);
		if(bSub)
			pSystem.parse(true);
		else
			pSystem.parse(false);
		if(bOutput)
			pSystem.output(argv[3],bSub);
		else
		{
		//	pSystem.check(argv[3],bSub);
		}
	}
	catch(exception& e)
	{
		cout<<e.what()<<endl;
	}
	return 0;
}
