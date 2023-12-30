#include <ZQ_common_conf.h>
#include "ConsoleCommand.h"
#include <vector>
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
}
#endif

namespace Console {
#ifdef ZQ_OS_MSWIN
bool execute(DataBuffer &output, const char* pExe, const char* pCmdLine)
{
	SECURITY_ATTRIBUTES   sa1,sa2;     
	HANDLE   hInputRead,hInputWrite;     
	HANDLE   hOutputRead,hOutputWrite;     

	sa1.nLength   =   sizeof(SECURITY_ATTRIBUTES);     
	sa1.lpSecurityDescriptor   =   NULL;     
	sa1.bInheritHandle   =   TRUE;     
	if(!CreatePipe(&hOutputRead,&hOutputWrite,&sa1,0))     
	{      
		//glog(Log::L_ERROR,"Create pile 1 error");
		return false;     
	}     

	sa2.nLength   =   sizeof(SECURITY_ATTRIBUTES);     
	sa2.lpSecurityDescriptor   =   NULL;     
	sa2.bInheritHandle   =   TRUE;     
	if(!CreatePipe(&hInputRead,&hInputWrite,&sa2,0))     
	{     
		//glog(Log::L_ERROR,"Create pile 2 error");     
		return false;     
	}     

	STARTUPINFO   si;     
	PROCESS_INFORMATION   pi;     
	si.cb   =   sizeof(STARTUPINFO);     
	GetStartupInfo(&si);     
	si.hStdError   =   hOutputWrite;     
	si.hStdOutput   =   hOutputWrite;     
	si.hStdInput   =   hInputRead;   
	si.wShowWindow   =   SW_HIDE;     
	si.dwFlags   =   STARTF_USESHOWWINDOW   |   STARTF_USESTDHANDLES;     
 
    //////////////////////////////////////////////////////////////////////////
    {
	    if(!CreateProcess(pExe,(LPSTR)pCmdLine,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi))   
	    {
            DWORD err = GetLastError();
		    //glog(Log::L_ERROR,"CreateProcess failed.[error code = %u]", err);
		    return false;     
	    }     
    }

    //close unreferred handle of child process
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

	CloseHandle(hInputRead);   
	CloseHandle(hOutputWrite);    

    {
	    char   buffer[4096] = {0};     
	    DWORD   bytesRead;     
	    while(true)   
	    {   
		    memset(buffer,0,sizeof(buffer));
		    if(!ReadFile(hOutputRead,buffer,sizeof(buffer) - 1,&bytesRead,NULL))     
		    {   
			    //glog(Log::L_ERROR,"ReadFile function return false,read output pile error");
			    break;
		    }
		    output << buffer;     
	    }     
    }
	CloseHandle(hInputWrite);   
	CloseHandle(hOutputRead);   

	return true;
}

#else
bool execute(DataBuffer &output, const char* pExe, const char* pCmdLine)
{
	//glog(Log::L_INFO,"execute command line [%s]", pCmdLine);

	char chcmd[512] = {0};
	strcpy(chcmd, pCmdLine);
	char* pspec = strchr(chcmd, ' ');
	ssize_t retRBytes= 0 ;
	if(pspec != NULL)
		*pspec = '\0';
	if(strchr(chcmd, '/') != NULL)
	{
		if( 0 != access(chcmd, X_OK))
		{
			//glog(Log::L_ERROR,"execute  command line[%s] access  failed error code[%d] string[%s]", pCmdLine,errno,strerror(errno));
			return false;
		}
	}

//printf("execute: %s\n",pCmdLine);

	int fd[2] = {0};
	if(pipe(fd) < 0)
	{
		//glog(Log::L_ERROR,"Create pile error code[%d]",errno);
		return false;
	}

	pid_t pid;
	if((pid = vfork()) < 0)
	{
		//glog(Log::L_ERROR,"vfork error code[%d]",errno);
		return false;
	}
	if(pid == 0)//child
	{
		close(fd[0]);
		dup2(fd[1],STDOUT_FILENO);

        std::vector<std::string> argv;
        std::istringstream iss(pCmdLine);;
        std::string tmp;
        while(iss >> tmp) {
            argv.push_back(tmp);
        }

        char** args = new char* [argv.size()+1];
        size_t i = 0;
        for(; i < argv.size(); ++i) {
            args[i] = const_cast<char*>(argv[i].c_str());
        }
        args[i] = (char*)NULL;
	
		int re =execvp(args[0], args);
		if(re < 0) {
			//glog(Log::L_ERROR,"exec command line[%s] failed error code[%d]",pCmdLine,errno);
        }
				
		close(fd[1]);
        _exit(errno);
	}
    
	close(fd[1]);
	
	char buf[4096] = {0};
	memset(buf,0,sizeof(buf));
	
	while(retRBytes = read(fd[0],buf,sizeof(buf)-1))
	{
		if(retRBytes < 0 )
			break ;
		buf[retRBytes] = '\0';
		output << buf;
		//memset(buf,0,sizeof(buf));
	}

	close(fd[0]);

    int status;
    wait(&status);

	return true;
}
#endif
}
