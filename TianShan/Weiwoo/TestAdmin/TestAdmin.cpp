// TestAdmin.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "weiwooAdmin.h"
#include <iostream>
#include <string.h>
#include <vector>
#include "ScriptEnv.h"
using namespace std;

ZQ::common::NativeThreadPool pool;
int main(int argc, char* argv[])
{
	char	szBuf[2048];
	int iRet=0;
	WeiwooAdmin admin;
	if(argc>1)
	{	
		if (stricmp(argv[1],"script") == 0) 
		{
			ScriptEnv env(pool);
			env.init(argv[2]);
			while(1)
			{
				Sleep (1000);
			}
		}
		for(int i=1;i<argc;i++)
		{
			if(admin.ParseCommand(argv[i])==RETERN_QUIT)
			return 0;
		}		
	}
	do 
	{
		cout<<">>";
		cin.getline(szBuf,2048);
		iRet=admin.ParseCommand(std::string(szBuf));
	} while(iRet!=RETERN_QUIT);
	return 0;
}

