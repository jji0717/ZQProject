#include "Daemon.h"
#include <string>
using namespace std;

HANDLE  hConsoleOut;                   /* Handle to the console */
CONSOLE_SCREEN_BUFFER_INFO csbiInfo;   /* Console information */

void ClearScreen( void )
{
    DWORD    dummy;
    COORD    Home = { 0, 0 };
    FillConsoleOutputCharacter( hConsoleOut, ' ', csbiInfo.dwSize.X * csbiInfo.dwSize.Y, Home, &dummy );
}

void usage()
{
	printf("\nCurrent supported commands are:\n\n");
	printf("o <ip>:<port>		Open a connection to a work node\n");
	printf("r <work type>		Request a certain work type to work node\n");
	printf("t			Send a 'Test' user request to last successful task\n");
	printf("c			Close connection\n");
	printf("q			Quit utility\n");
	printf("h/?			Display this help\n\n");
}

void main()
{	
	//////////////////////////////////////////////////////////////////////////
	// input parameters declarations
	std::string wipStr;			// work node ip string
	std::string wportStr;		// work node port str
	std::string worktypeStr;	// work type

	std::string testtaskid="";
	
	URLStr mnurl;
	// set dummy session id
	mnurl.setProtocol("MPF");
	mnurl.setHost("localhost");
	mnurl.setPort(20000);
	mnurl.setPath(URL_PATH_SESSION);
	mnurl.setVar(URL_VARNAME_SESSION_ID,URL_VAR_SESSION_DUMMY);
	

	//////////////////////////////////////////////////////////////////////////
	hConsoleOut = GetStdHandle( STD_OUTPUT_HANDLE );
    GetConsoleScreenBufferInfo( hConsoleOut, &csbiInfo );
    ClearScreen();

	//////////////////////////////////////////////////////////////////////////
	
	printf("\nWelcome to ZQ MPF Work Node Dummy Session Test Utility\n");
	printf("Use ? to display command help\n\n");
	char keyline[514];
	char keyc[2];
	char keyp[512];
	bool bConnected = FALSE;
	bool bSuc = FALSE;
	bool bOver = FALSE;
	
	ZQ::rpc::RpcClient* fakeClient=NULL;
	while(!bOver)
	{
		// get input
		bSuc = FALSE;
		wipStr		="";
		wportStr	="";
		worktypeStr ="";
		int port=0;
		ZeroMemory(keyline,514);
		ZeroMemory(keyc, 2);
		ZeroMemory(keyp, 512);
		
		printf("Dummy Test> ");
		gets(keyline);
		sscanf(keyline, "%1s %511s", keyc, keyp);

		std::string keyStr = keyp;
		
		int cpos;
		ZQ::rpc::RpcValue param1, result1, parAttr1, reqAttr1, param2, result2, parAttr2, reqAttr2;
		int code;
		char taskid[256], comment[256];

		// judge input
		switch(keyc[0]) 
		{
		case 'o':
			if(bConnected)
			{
				printf("Work node connection already opened, please close first.\n");
				break;
			}
			if(keyStr.length()<=5)
			{
				printf("Invalid work node ip/port.\n");
				break;
			}
			
			cpos = keyStr.find_first_of(":");
			if(keyStr.empty() || cpos==keyStr.npos)
			{
				printf("Invalid work node ip/port \"%s\".\n", keyStr.c_str());
				break;
			}
			wipStr = keyStr.substr(0, cpos);
			wportStr = keyStr.substr(cpos+1);
			
			port = atoi(wportStr.c_str());
			if(port==0) 
			{
				printf("Invalid work node port \"%s\".\n", wportStr.c_str());
				break;
			}
			
			fakeClient = new ZQ::rpc::RpcClient(wipStr.c_str(), port);
			fakeClient->setResponseTimeout(5);
			bConnected = TRUE;
			break;
		case 'c':
			if(!bConnected)
			{
				printf("Work node connection not opened, please open first.\n");
				break;
			}
			fakeClient->close();
			delete fakeClient;
			fakeClient = NULL;
			bConnected = FALSE;
			break;
		case 'r':
			if(keyStr.length()==0)
			{
				printf("Invalid work type name.\n");
				break;
			}
			reqAttr1.SetStruct(TASK_TYPE_KEY, RpcValue(keyp));
			reqAttr1.SetStruct(MGM_SESSION_URL_KEY, RpcValue(mnurl.generate()));
			parAttr1.SetStruct(ACTION_ID_KEY, RpcValue(REQUEST_SETUP));
			parAttr1.SetStruct(REQUEST_ATTR_KEY, RpcValue(reqAttr1));
			param1.SetArray(0, parAttr1);
			
			ZeroMemory(taskid, 256);
			ZeroMemory(comment, 256);

			if(fakeClient->execute(TASKREQUEST_METHOD, param1, result1))
			{
				if(result1.getType()==result1.TypeStruct)
				{
					code = (int)result1[ERROR_CODE_KEY];
					ZQ::rpc::RpcValue rpccomment = result1[COMMENT_KEY];
					rpccomment.ToString(comment, 256);
					if(code==0)
					{
						bSuc = TRUE;
						ZQ::rpc::RpcValue rpctaskid = result1[TASK_ID_KEY];
						rpctaskid.ToString(taskid, 256);
					}
				}
			}

			if(!bSuc)                                                                                                                                        
			{
				printf("Error: task request \"%s\" failed. - %s\n", keyStr.c_str(), comment);
				break;
			}

			reqAttr2.SetStruct(TASK_ID_KEY, RpcValue(taskid));
			parAttr2.SetStruct(ACTION_ID_KEY, RpcValue(REQUEST_PLAY));
			parAttr2.SetStruct(REQUEST_ATTR_KEY, reqAttr2);
			param2.SetArray(0, parAttr2);
			
			ZeroMemory(comment, 256);
			
			if(fakeClient->execute(TASKREQUEST_METHOD, param2, result2))
			{
				if(result2.getType()==result2.TypeStruct)
				{
					code = (int)result2[ERROR_CODE_KEY];
					ZQ::rpc::RpcValue rpccomment = result2[COMMENT_KEY];
					rpccomment.ToString(comment, 256);
					if(code==0)
					{
						bSuc = TRUE;
					}
				}
			}
			
			if(!bSuc)
			{
				printf("Error: task request \"%s\" failed. - %s\n", keyStr.c_str(), comment);
			}
			else
			{
				testtaskid = taskid;
				printf("Succsess: task request \"%s\" succeeded.\n", keyStr.c_str());
			}
			break;
		case 't':
			{
			
			reqAttr1.SetStruct(TASK_ID_KEY, RpcValue(testtaskid.c_str()));
			reqAttr1.SetStruct(USER_ACTION_ID_KEY, RpcValue("Test"));
			RpcValue t;
			reqAttr1.SetStruct(USER_ATTR_KEY, t);
			parAttr1.SetStruct(ACTION_ID_KEY, RpcValue(REQUEST_USER));
			parAttr1.SetStruct(REQUEST_ATTR_KEY, RpcValue(reqAttr1));
			param1.SetArray(0, parAttr1);
			fakeClient->execute(TASKREQUEST_METHOD, param1, result1);
			}
			break;
		case 'T':
			{
			
			reqAttr1.SetStruct(TASK_ID_KEY, RpcValue(testtaskid.c_str()));
			reqAttr1.SetStruct(USER_ACTION_ID_KEY, RpcValue("Haha"));
			RpcValue t;
			reqAttr1.SetStruct(USER_ATTR_KEY, t);
			parAttr1.SetStruct(ACTION_ID_KEY, RpcValue(REQUEST_USER));
			parAttr1.SetStruct(REQUEST_ATTR_KEY, RpcValue(reqAttr1));
			param1.SetArray(0, parAttr1);
			fakeClient->execute(TASKREQUEST_METHOD, param1, result1);
			}
			break;
		case 'h':
		case '?':
			usage();
			break;
		case 'q':
			if(fakeClient)
			{
				fakeClient->close();
				delete fakeClient;
				fakeClient = NULL;
			}

			bOver = TRUE;
			break;
		default:
			printf("\"%s\" - Incorrect command", keyc);
			usage();
			break;
		}
		
	}
	
}