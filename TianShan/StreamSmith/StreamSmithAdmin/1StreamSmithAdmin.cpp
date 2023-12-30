

#include "SSAdminWrapper.h"
#include "ScriptEnv.h"
#include "ScriptRequest.h"
#include <iostream>
#include <fstream>



using namespace std;

NativeThreadPool pool;
ScriptEnv _env(pool);
void StartScript(const char* inifile)
{
	_env.init(inifile);
}

int main(int argc,char* argv[])
{
	SSAdminWrapper par;
	char	szBuf[2048];
	
	if(argc>=2)
	{
		if ( stricmp(argv[1],"script") == 0 ) 
		{
			StartScript(argv[2]);
			while(1)
			{
				Sleep(100);
			}
			return 1;
		}
	}
	par.ParseCommand("version");
	
	while (1) 	
	{
		cout<<">>";
		if( gets(szBuf) == NULL)
		{
			return 1;
		}
		if( par.ParseCommand(szBuf) == RETERN_QUIT )
		{		
			break;			
		}		
	}	
	return 1;
}
