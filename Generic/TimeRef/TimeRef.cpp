// This is the main project file for VC++ application project 
// generated using an Application Wizard.

#include <windows.h>
#include <string>
#include <strstream>

void usage();
bool getParams(int argc, char* argv[]);
bool isParam(char* pBuff, char param);
__int64 timeStr2Num(const char* pTimeStr, bool nextYear=false);
bool isNum(char p);
bool parseFile();

char	szInputFile[MAX_PATH]	= {0};
DWORD	dwRefLine				= ULONG_MAX;
DWORD	dwOutputLines			= ULONG_MAX;
char	szRefTime[MAX_PATH]		= {0};
DWORD	dwReplaceTime			= 0;
DWORD	dwNextYear				= 0;

HANDLE	hFile					= INVALID_HANDLE_VALUE;
__int64	i64Reference			= 0;
bool	bGotRef					= false;
bool	bAfterRef				= false;
DWORD	dwLineCount				= 0;
DWORD	dwAfterLineCount		= 0;

int main(int argc, char* argv[])
{
	// validate params
    if(!getParams(argc, argv))
	{
		return -1;
	}

	// check input file
	hFile = ::CreateFileA(szInputFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("ERROR: Can not open log file \"%s\" : %lu\n", szInputFile, ::GetLastError());
		return -1;
	}

	// ok, we have made sure that the file is there, do the parse then
	parseFile();

	// close file
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return 0;
}

void usage()
{
	printf("\n");
	printf("Echoes succedent lines in a time-formatted log file with specified time point as time reference.\n");
	printf("\n");
	printf("TIMEREF [/L n] [/T \"time\"] [/N count] [/R] [/Y] [drive:path]filename\n");
	printf("\n");
	printf("  filename     Specify the file to parse.\n");
	printf("\n");
	printf("  /L n         Set the 'n'th line as the time reference zero point.\n");
	printf("  /T \"time\"    Set the 'time' point as the time reference zero point.\n");
	printf("               This option is not valid if /L is specified.\n");
	printf("               The format of this option must be like \"MM/DD hh:mm:ss:ttt\".\n"); 
	printf("               such as \"01/30 20:36:00:217\".\n");
	printf("  /R           Replace all time string to relative time.\n");
	printf("  /Y           If this option is specified, assume that if any succedent lines\n");
	printf("               with an earlier time is next year.\n");
	printf("               Eg: the reference is line 100 with time \"12/30 23:45:00:105\",\n");
	printf("               if line 105 has earlier time \"01/30 20:36:00:217\", if /E is specified,\n");
	printf("               we will treat this \"01/30 20:36:00:217\" as next year of \"12/30 23:45:00:105\".\n");
	printf("  /N count     Limit output to 'count' lines.\n");
	printf("\n");
	printf("eg: TIMEREF /T \"01/16 10:52:57:531\" /N 100 \"D:\\Test Folder\\DBSync.log\"\n");
	printf("\tDump out 100 lines after 01/16 10:52:57:531.\n");
	printf("\n");
	printf("eg: TIMEREF /R /L 20 /N 100 \"D:\\Test Folder\\DBSync.log\"\n");
	printf("\tDump out 100 lines after 20th line, and replace the timestamp of each line with relative time.\n");
}

bool getParams(int argc, char* argv[])
{
	int idx = 1;

	if(argc==1)
	{
		usage();
		return false;
	}

	while (idx < argc)
	{
		if( isParam(argv[idx], '?') )
		{
			usage();
			return false;
		}
		else if( isParam(argv[idx], 'R') )
		{
			dwReplaceTime = 1;
		}
		else if( isParam(argv[idx], 'Y') )
		{
			dwNextYear = 1;
		}
		else if ( isParam(argv[idx], 'L') && ++idx < argc )
		{
			_snscanf(argv[idx], 10, "%lu", &dwRefLine);
		}
		else if ( isParam(argv[idx], 'T') && ++idx < argc )
		{
			strncpy(szRefTime, argv[idx], sizeof(szRefTime)-1);
		}
		else if ( isParam(argv[idx], 'N') && ++idx < argc )
		{
			_snscanf(argv[idx], 10, "%lu", &dwOutputLines);
		}
		else if ( isParam(argv[idx], 0) )	// if not a valid param, but still starting with '-' or '/', kick its ass back
		{
			printf("Invalid option(s)!\n");
			usage();
			return false;
		}
		else	// now it should be (at least we think) the input file
		{
			strncpy(szInputFile, argv[idx], sizeof(szInputFile)-1);
		}
		++ idx;
	}

	if(strlen(szInputFile)==0)
	{
		printf("ERROR: No log file specified!\n");
		return false;
	}

	if(dwRefLine==ULONG_MAX && strlen(szRefTime)==0)
	{
		printf("ERROR: No reference line or time point specified!\n");
		return false;
	}

	if(dwRefLine==ULONG_MAX)
	{
		i64Reference = timeStr2Num(szRefTime);
		
		if(i64Reference ==0)
		{
			printf("ERROR: Incorrect time format specified in /T option!\n");
			return false;
		}
		else
		{
			bGotRef = true;	// we got the reference time value
			printf("-------------------------------------------\n");
			printf("|   Reference Time : %18s   |\n", szRefTime);
			printf("-------------------------------------------\n");
		}
	}
	
	return true;
}

bool isParam(char* pBuff, char param)
{
	// if param=0, we will check if it is a param like thing
	if(param==0)
	{
		if(pBuff[0]=='-' || pBuff[0]=='/' )
			return true;

		return false;
	}

	// check if it is '?'
	if(param=='?')
	{
		if(strlen(pBuff)>1 && pBuff[1]=='?')
			return true;

		return false;
	}

	// check if alphabetic
	if(param<'A' || (param>'Z'&&param<'a') || param>'z')	
		return false;

	// make it lower case first
	char lower = (param<'A')? (param) : (param+('a'-'A'));
	char upper = lower - ('a'-'A');

	std::string slashLower	= "/";
	slashLower += lower;
	std::string slashUpper	= "/";
	slashUpper += upper;
	std::string dashLower	= "-";
	dashLower += lower;
	std::string dashUpper	= "-";
	dashLower += upper;

	if( strcmp(pBuff, slashLower.c_str())==0 
		|| strcmp(pBuff, slashUpper.c_str())==0 
		|| strcmp(pBuff, dashLower.c_str())==0 
		|| strcmp(pBuff, dashUpper.c_str())==0 )
	{
		return true;
	}

	return false;
}

__int64 timeStr2Num(const char* pTimeStr, bool nextYear/*=false*/)
{
	std::string timeStr = pTimeStr;
	__int64 retVal = 0;
	
	if(timeStr.length()!=18)
	{
		return 0;
	}

	if( !isNum(timeStr[0]) 
		|| !isNum(timeStr[1])
		|| timeStr[2]!='/'
		|| !isNum(timeStr[3])
		|| !isNum(timeStr[4])
		|| timeStr[5]!=' '
		|| !isNum(timeStr[6])
		|| !isNum(timeStr[7])
		|| timeStr[8]!=':'
		|| !isNum(timeStr[9])
		|| !isNum(timeStr[10])
		|| timeStr[11]!=':'
		|| !isNum(timeStr[12])
		|| !isNum(timeStr[13])
		|| timeStr[14]!=':'
		|| !isNum(timeStr[15])
		|| !isNum(timeStr[16])
		|| !isNum(timeStr[17]) )
	{
		return 0;
	}

	SYSTEMTIME sysT;
	memset(&sysT, 0, sizeof(sysT));
	
	{
		SYSTEMTIME localT;
		::GetLocalTime(&localT);
		sysT.wYear = localT.wYear;	// since in Seac log there is no year, we use current year

		if(nextYear)
			sysT.wYear += 1;
	}

	_snscanf(pTimeStr, 18, "%02d/%02d %02d:%02d:%02d:%03d", &(sysT.wMonth), &(sysT.wDay), &(sysT.wHour), &(sysT.wMinute), &(sysT.wSecond), &(sysT.wMilliseconds));

	// convert to FILETIME then to int64
	FILETIME fileT;
	memset(&fileT, 0, sizeof(fileT));

	::SystemTimeToFileTime(&sysT, &fileT);

	ULARGE_INTEGER largeI;
	largeI.LowPart = fileT.dwLowDateTime;
	largeI.HighPart = fileT.dwHighDateTime;

	retVal = largeI.QuadPart;

	return retVal;
}

bool isNum(char p)
{
	if(p>='0' && p<='9')
		return true;
	return false;
}

class AutoMem
{
private:
	char* &_ptr;
public:
	AutoMem(char* &p): _ptr(p) 
	{
		_ptr;
	}
	~AutoMem() { if(_ptr) delete[] _ptr; _ptr=NULL; }
};

#define	 BUFFER_SIZE	(1024*1024)
bool parseFile()
{
	// double check handle
	if(hFile==INVALID_HANDLE_VALUE)
		return false;

	// read into buffer than parse
	DWORD	bytesRead = 0;
	DWORD	bytesToRead = BUFFER_SIZE-1;
	BOOL	nCode = 0;
	char*	pBuffer = new char[BUFFER_SIZE];
	if(pBuffer==NULL)
	{
		printf("ERROR: Unable to allocate buffer, not enough memory?!\n");
		return false;
	}

	AutoMem	auto_pBuffer(pBuffer);

	bool bOver = false;
	while(!bOver)
	{
		// read into big buffer
		memset(pBuffer, 0, BUFFER_SIZE);
		nCode = ::ReadFile(hFile, pBuffer, bytesToRead, &bytesRead, NULL);
		
		if(nCode==0 || bytesRead==0)
			break;

		if(bytesRead<bytesToRead)
			bOver = true;

		// get rid of starting \r \n
		char *pIoBuff = pBuffer;
		int	 ioSize = bytesRead;
		while( (pIoBuff[0]==0x0A || pIoBuff[0]==0x0D) && ioSize>0 )
		{
			pIoBuff ++;
			ioSize --;
		}

		if(ioSize<=0)
			continue;

		std::istrstream	strBuff(pIoBuff, ioSize+1);
		if(strBuff.fail())
		{
			printf("ERROR: Unable to copy buffer into istrstream object!\n");
			return false;
		}

		// get each line from the buffer and parse
		std::string tmpString="";
		for(;;) 
		{
			if(dwAfterLineCount>dwOutputLines)	// test output lines
				return true;

			std::string tmpString="";
			char carriage = 0x0A;
			std::getline(strBuff, tmpString, carriage);

			if(tmpString.length()==0 || tmpString[0]==0) 
				break;

			size_t lastPos;
			
			lastPos = tmpString.find_first_of("\x0A\x0D");
			if(lastPos!=tmpString.npos)
			{
				tmpString = tmpString.substr(0, lastPos);
			}
			else
			{
				if(!bOver)
				{
					// not end of file, but not end with new line, return this to the file buffer
					LONG moveVal = 0-tmpString.length();
					::SetFilePointer(hFile, moveVal, NULL, FILE_CURRENT);
					break;
				}
			}
			
			

			dwLineCount ++;
			
			std::string timeStr = tmpString.substr(0, 18);
			__int64	iTime = timeStr2Num(timeStr.c_str());

			if(!bGotRef)	// we did not get the reference time yet
			{
				if(dwRefLine==dwLineCount)
				{
					if(iTime==0)
					{
						printf("ERROR: Reference line (%d) does not have time stamp at begining!\n");
						return false;
					}

					// ok, this is just the line specified, we have the reference time value now
					bGotRef = true;
					bAfterRef = true;
					i64Reference = iTime;
					printf("-------------------------------------------\n");
					printf("|   Reference Time : %18s   |\n", timeStr.c_str());
					printf("-------------------------------------------\n");
				}
			}


			if(bGotRef)	// we already have the reference time value
			{
				if(iTime==0)	// not starting with time
				{
					if(dwReplaceTime)
						printf("               %s\n", tmpString.c_str());
					else
						printf("                  %s\n", tmpString.c_str());
					
					dwAfterLineCount ++;
					continue;
				}
				
				if(iTime>=i64Reference)
					bAfterRef = true;
				
				if(bAfterRef && dwNextYear && iTime<i64Reference)
					iTime = timeStr2Num(timeStr.c_str(), true);

				if(bAfterRef)
				{
					__int64 iDiff = iTime - i64Reference;
					char tmpBuff[18]={0};
					sprintf(tmpBuff, "%10I64d.%03I64d", iDiff/10000000, (iDiff/10000)%1000);

					if(dwReplaceTime)
						printf("%18s%s\n",tmpBuff,tmpString.substr(18).c_str());
					else
						printf("%15s   %s\n",tmpBuff,tmpString.c_str());
					
					dwAfterLineCount ++;
					continue;
				}
			}

		}	//end for

		continue;

	}	// end while

	return true;
}