extern "C" {
#include <semaphore.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/resource.h>
}

#include <ctime>
#include <vector>
#include <string>
#include <fstream>
#include <cerrno>
#include <sstream>

#include "FileLog.h"
#include "XMLPreferenceEx.h"
#include <snmp/ZQSnmp.h>
#include "strHelper.h"

#include "version.h"

#define LOG (*logConfig.logger)
#define DEFAULT_DELAY  30

using namespace std;

#define ERR(_str_) { \
    syslog(LOG_ERR, _str_); \
    if(logConfig.logger) { \
    LOG(Log::L_ERROR, _str_); \
    } \
} 
#define INFO(_str_) { \
    syslog(LOG_INFO, _str_); \
    if(logConfig.logger) { \
    LOG(Log::L_INFO, _str_); \
    } \
}

#define VERIFY(args, opt) \
    if(!(args)) { \
    fprintf(stderr, "invalid argument for option (-%c)\n", opt); \
    usage(); \
    return (0); \
    } 
	
#ifndef MAX_PATH
#define MAX_PATH 256
#endif

char g_paths[MAX_PATH],g_patht[MAX_PATH],g_temp_paths[MAX_PATH],g_temp_patht[MAX_PATH];   
typedef std::vector<std::string> SVCSET;

using namespace ZQ::common;

extern XMLPreferenceEx* getPreferenceNode(const std::string& path, XMLPreferenceDocumentEx& config); 
extern bool checkLock(const std::string& file);

struct _tagLogConfig {
    bool loggingMask;
    bool snmpLoggingMask;
    std::string path;
    std::string name;
    unsigned long size;
    int level;
    FileLog* logger;
} logConfig;

struct _tagServiceConfig {
    std::string name;
    std::string service;
    std::string imagePath;
    std::string argument;
    std::string backfiles;
    std::string svcInstanceId;
    unsigned int componentId;
    unsigned appAliveWait;
    unsigned aliveTimeout;
    unsigned short restartTries;
    unsigned restartInterval; // substituted meaning to maximal restart interval in secs
    unsigned startupWait;
    unsigned restartTimeout; // total timeout of all restart times sum of restartTries
} serviceConfig;

namespace {
    // constants
    const char* ConfigFile  = "/etc/TianShan.xml";
    const char* LockPath = "/var/run/";

    std::string LockFile;
    
	// defaults
    volatile sig_atomic_t stop = 0;

    sem_t* aliveEvent = 0;
    sem_t* stopEvent = 0;
    std::string aliveName;
    std::string stopName;

    pid_t svc_pid = 0;

    ZQ::SNMP::ModuleMIB* mmib;
    ZQ::SNMP::SubAgent* snmpSubAgent;
}

static void childHandler(int signal, siginfo_t* info, void* data)
{
    int saved_errno = errno;
    std::ostringstream os;

    // make sure no zombie 
    int status;
    if (wait(&status) == -1)
        return;

    if(WIFEXITED(status)) 
	{
        int code = WEXITSTATUS(status);
        os << "service exited with code: (" << code << ")";

        if(code != 0 && code != (-1)) {
            char buff[200];
            os << ": (" << strerror_r(code, buff, 200) << ")";
        }
    }
    else if(WIFSIGNALED(status))
	{
        os << "service terminated by signal (" << WTERMSIG(status) << ")";
        if(WCOREDUMP(status)) {
            os << " with core dumped";
        }
    }

    ERR(os.str().c_str());

    errno = saved_errno;
}

static void terminate(int signal, siginfo_t* info, void* data)
{
    stop = 1;
    sem_post(stopEvent);

    int status;
    unsigned alive = serviceConfig.appAliveWait;
    while(alive) {
        /* quit if wait failed or service has stopped */
        if(waitpid(svc_pid, &status, WNOHANG) != 0) {
            break;
        }
        --alive;
        sleep(1);
    }
	
    if(!alive) {
        kill(svc_pid, SIGKILL);
        wait(&status);
    } 
}

static void coredump(int signal, siginfo_t* info, void* data)
{
    struct rlimit rlim;

    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);

    /* try to stop the service before crash */
    sem_post(stopEvent);
    int status = 0;
    wait(&status);

    int fd = open("/proc/sys/kernel/core_pattern", O_WRONLY); 
    if(fd != (-1)) {
        write(fd, "core.%e.%p", 11);
    }

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler=SIG_DFL;

    sigaction(SIGSEGV, &act, 0);
    raise(SIGSEGV);
}

static bool initSignals()
{
    struct sigaction act;

    act.sa_flags = SA_SIGINFO; 
    sigemptyset(&act.sa_mask);

    act.sa_sigaction = terminate;
    if (sigaction(SIGTERM, &act, 0) < 0 ||  sigaction(SIGINT,  &act, 0) < 0) 
        return false;

    act.sa_sigaction = childHandler;
    if(sigaction(SIGCHLD, &act, 0) < 0) 
        return false;

    act.sa_sigaction=coredump;
    if(sigaction(SIGSEGV, &act, 0) < 0)
        return false;

    act.sa_handler = SIG_IGN;
    if(sigaction(SIGHUP, &act, 0) < 0)
        return false;

    return true;
}

static bool initEvents() 
{
    mode_t mode = O_CREAT|O_EXCL;

    aliveName = serviceConfig.service + "_Alive";

sem_alive:
    aliveEvent = sem_open(aliveName.c_str(), mode, 0664, 0);

    if (aliveEvent == SEM_FAILED) 
	{
        if(errno == EEXIST) {
            syslog(LOG_WARNING, "semphore (%s) already exists", aliveName.c_str());
            sem_unlink(aliveName.c_str());

            goto sem_alive;
        }
        else {
            syslog(LOG_ERR, "failed to open alive event (%s): (%s)", aliveName.c_str(), strerror(errno));
            return false;
        }
    }

    stopName = serviceConfig.service + "_Stop";
sem_stop:
    stopEvent = sem_open(stopName.c_str(), mode, 0664, 0);

    if (stopEvent == SEM_FAILED) {
        if(errno == EEXIST) {
            syslog(LOG_WARNING, "semphore (%s) already exists", stopName.c_str());
            sem_unlink(stopName.c_str());

            goto sem_stop;
        }
        else {
            syslog(LOG_ERR, "failed to open stop event (%s): (%s)", stopName.c_str(), strerror(errno));
            return false;
        }
    }
	
    return true;
}

void cleanup()
{
    closelog();

    if (mmib)
        delete mmib;

    if (snmpSubAgent)
        delete snmpSubAgent;
    
    if (!LockFile.empty())
        remove(LockFile.c_str());

    if (logConfig.logger)
	{
        delete logConfig.logger;
        logConfig.logger = 0;
    }

    if (aliveEvent)
	{
        sem_unlink(aliveName.c_str());
    }
	
    if (stopEvent)
	{
        sem_unlink(stopName.c_str());
    }
}

bool loadConfig()
{
    XMLPreferenceDocumentEx doc;
    try {
        if(!doc.open(ConfigFile))
		{
            syslog(LOG_ERR, "failed to load configuration from (%s)", ConfigFile);
            return (-1);
        }
    }
    catch(XMLException& e)
	{
        syslog(LOG_ERR, "failed to open (%s): %s", ConfigFile, e.what());
        return (-1);
    }

    // log configuration
    char value[256];
    memset(value, '\0', 256);

    bool res = false;

    // cur off instance id from service name
    std::string serviceName;
    size_t pos = serviceConfig.service.find_first_of("0123456789");
    if (std::string::npos != pos)
        serviceName = serviceName.substr(0, pos);
	else
        serviceName = serviceConfig.service;

    // read the SNMP configuration
	XMLPreferenceEx* node = getPreferenceNode("SNMP/" + serviceName, doc);
    if(!node)
	{
        syslog(LOG_ERR, "failed to get snmp configuration");
        return false;
    }

    if (res = node->getAttributeValue("oid", value))
	{
        std::istringstream is(value);
        is >> serviceConfig.componentId;
    }
    node->free();

    node = getPreferenceNode(serviceConfig.name+"/log", doc);
    if (!node)
	{
        syslog(LOG_ERR, "failed to get log configuration");
        return false;
    }

    // loggingMask
    if(node->getAttributeValue("loggingMask", value))
	{
        std::istringstream is(value);
        is >> logConfig.loggingMask;
    }

	// snmpLoggingMask
    if(node->getAttributeValue("snmpLoggingMask", value))
	{
        std::istringstream is(value);
        is >> logConfig.snmpLoggingMask;  
    }

    if(logConfig.loggingMask)
	{
        // path is required
        if(!node->getAttributeValue("path", value))
		{
            syslog(LOG_ERR, "failed to get log path");
            node->free();
            return false;
        }
        logConfig.path = value;
		
		// size
        if(node->getAttributeValue("size", value)) 
		{
            std::istringstream is(value);
            is >> logConfig.size;
        }

        // level
        if(node->getAttributeValue("level", value))
		{
            std::istringstream is(value);
            is >> logConfig.level;
        }

        logConfig.name = logConfig.path + "/" + serviceConfig.name + ".log";
    }    
    node->free();

    // service configuration 
    node = getPreferenceNode(serviceConfig.name+"/service", doc);
    if(!node)
	{
        syslog(LOG_ERR, "failed to get service configuration");
        node->free();
        return false;
    }

    if(node->getAttributeValue("argument", value)) 
        serviceConfig.argument = value;
    
    if( node->getAttributeValue("instanceId", value))
        serviceConfig.svcInstanceId = value;
	
    if(node->getAttributeValue("restartInterval", value))
	{
        std::istringstream is(value);
        is >> serviceConfig.restartInterval;
    }

    if(node->getAttributeValue("restartTimeout", value))
	{
        std::istringstream is(value);
        is >> serviceConfig.restartTimeout;
    }

    if(node->getAttributeValue("restartTries", value)) 
	{
        std::istringstream is(value);
        is >> serviceConfig.restartTries;
    }

    if(node->getAttributeValue("appAliveWait", value))
	{
        std::istringstream is(value);
        is >> serviceConfig.appAliveWait;
    }

    if(node->getAttributeValue("aliveTimeout", value))
	{
        std::istringstream is(value);
        is >> serviceConfig.aliveTimeout;
    }

    if(node->getAttributeValue("imagePath", value))
        serviceConfig.imagePath = value; 
    else
        syslog(LOG_ERR, "failed to read image path");

    if(res = node->getAttributeValue("backupFiles", value))
        serviceConfig.backfiles = value; 
    else 
	{
        serviceConfig.backfiles="";
        syslog(LOG_ERR, "failed to read backupFiles");
        node->free();

        return true;
    }
    node->free();

    return true;
}

pid_t startProcess() 
{
    pid_t pid = vfork();

    if (pid < 0)
	{
        std::ostringstream os;
        os << "failed to fork child[" << serviceConfig.imagePath << "] " 
			<< strerror(errno) << "(" << errno << ")";
        
		ERR(os.str().c_str());
        return (-1);
    }

    if(!pid) 
	{
        execl(serviceConfig.imagePath.c_str(), 
            serviceConfig.imagePath.c_str(), 
            "-s",
            serviceConfig.service.c_str(),
            "-i",
            serviceConfig.svcInstanceId.c_str(),
            "-n",
            serviceConfig.argument.c_str(),
            "-m", // shell mode flag
            (char*)0);

        std::ostringstream os;
        os << "failed to execute[" << serviceConfig.imagePath << "] " 
			<< strerror(errno) << "(" << errno << ")";
        
		ERR(os.str().c_str());
        _exit(errno);
    }

    return pid;
}

void initConfig()
{
    logConfig.loggingMask         = 0;
    logConfig.snmpLoggingMask     = 0;
    logConfig.size                = ZQLOG_DEFAULT_FILESIZE;
    logConfig.level               = Log::L_DEBUG;

    serviceConfig.appAliveWait    = 15;
    serviceConfig.aliveTimeout    = 90;
    serviceConfig.restartInterval = 120;
    serviceConfig.restartTries    = 3;
    serviceConfig.startupWait     = 2;
    serviceConfig.restartTimeout  = 180;
    serviceConfig.svcInstanceId   = "";
}

void printConfig() {
    std::cerr << "Log:"
        << "\nenabled: "             << std::boolalpha << logConfig.loggingMask
        << "\npath: "                << logConfig.path
        << "\nname: "                << logConfig.name
        << "\nsize: "                << logConfig.size
        << "\nlevel: "               << logConfig.level
        << "\nsnmp logging: "        << std::boolalpha << logConfig.snmpLoggingMask
        << std::endl << std::endl
        << "Service:"         
		<< "\nargument: "            << serviceConfig.argument
        << "\naliveTimeout: "        << serviceConfig.aliveTimeout
        << "\nrestartTimeout: "      << serviceConfig.restartTimeout
        << "\nrestartMaxInterval: "  << serviceConfig.restartInterval
        << "\nrestartTries: "        << serviceConfig.restartTries
        << "\nappAliveWait: "        << serviceConfig.appAliveWait
        << "\nimagePath: "           << serviceConfig.imagePath
        << "\nstartupWait: "         << serviceConfig.startupWait
        << std::endl << std::endl;
}

static void usage() {
    std::cerr << "\nSystemShell [options] <service>\n\n"
        << "<service>        run service\n"
        << "-l               list running services\n"
        << "-s [all|service] stop a specified service or all\n" 
        << "-d <delay>       wait 'delay' seconds for service to stop\n"
        << "--version        show the version\n"
        << std::endl;
}

static bool stopService(const std::string& svc, unsigned delay)
{
    std::string name = svc;
    std::ostringstream oss;
    oss << LockPath << name << ".pid"; 

    pid_t pid = 0;

    std::ifstream inf(oss.str().c_str());
    if(!inf.is_open())
	{
        std::cerr << "service [" << name << "] not found or is not running" << std::endl;
        return true;
    }

    std::string line;
    getline(inf, line);
    std::istringstream iss(line);

    iss >> pid;
    inf.close();

    if(kill(pid, SIGTERM) == (-1)) {
        std::cerr << "failed to stop service (" 
            << svc << ") [" << errno << "] [" << strerror(errno) << "]"
            << std::endl;
        return false;
    }

    oss.str("");
    oss << "/proc/" << pid;

    bool stopped = false;    

    unsigned wait = delay;
    if(wait <= 0)
        wait = DEFAULT_DELAY;

    std::cerr << "stopping [" << svc << "] ";
    while(wait--)
	{
        sleep(1);

        struct stat info;
        if(stat(oss.str().c_str(), &info)) {
            stopped = true;
            break;
        }
        std::cerr << ".";
    }

    if(!stopped)
	{
        kill(pid, SIGKILL);
        std::cerr << "    [KILLED]";
    }
    else
        std::cerr << "    [OK]";

    std::cerr << std::endl;
    return true;
}

static void listService(SVCSET* set = 0)
{
    if(set)
        set->clear();

    const std::string basedir = "/proc/";

    DIR* dir = opendir(basedir.c_str());
    if(!dir)
	{
        fprintf(stderr, "failed to open /proc, no permission or broken system?\n");
        return;
    }

    //  we traverse all the running processes and try to find a command start with 
    // 'SystemShell' and that's it.
    std::ostringstream myself;
    myself << getpid();

    struct dirent* entry;
    while((entry = readdir(dir))!=0)
	{
        std::string tmp = basedir + entry->d_name;

        struct stat st;
        if(!stat(tmp.c_str(), &st) 
			&& S_ISDIR(st.st_mode)
			&& ((*entry->d_name)>'0' && (*entry->d_name)<='9') && strcmp(entry->d_name, myself.str().c_str()))
		{
                tmp += "/cmdline";
        }    
        else continue;

        FILE* f = fopen(tmp.c_str(), "r");
        if(!f)
            continue;

        char buff[255];
        memset(buff, '\0', sizeof(buff));
        if(!fgets(buff, sizeof(buff), f))
		{
            fclose(f);
            continue;
        }

        char* name  = basename(buff);
        if(!strcmp(name, "SystemShell"))
		{
            long pos = ftell(f);
            // take care of the overflow
            if(pos < (long)sizeof(buff)) 
			{
                if((name = strrchr(buff, '\0')))
                    ++name;
            }

            if(set)
                set->push_back(name);
            else printf("%s\n", name); 
        }
		
        fclose(f);
    }
	
    closedir(dir);
}

int  Copy_FileWithMMap (char* spathname,char* tpathname)
{
    LOG(Log::L_DEBUG, "Copy_FileWithMMap Enter");
    int fdin,fdout;
    void *src,*dst;
    size_t pagesize;
    struct stat statbuf;

    if((fdin = open(spathname,O_RDONLY)) <=0)
        return 0;

    if((fdout = open(tpathname,O_RDWR | O_CREAT | O_TRUNC,0644))<=0)
        return 0;

    fstat(fdin,&statbuf);
    ftruncate(fdout, statbuf.st_size) ;
    fstat(fdout, &statbuf) ;     
    LOG(Log::L_DEBUG, "both file open success");
    src = mmap(0,statbuf.st_size,PROT_READ,MAP_SHARED,fdin,0);
    if( src == MAP_FAILED )
        return 0;

    dst = mmap(0,statbuf.st_size,PROT_READ | PROT_WRITE,MAP_SHARED,fdout,0);
    if( dst == MAP_FAILED )
        return 0;

    memcpy(dst,src,statbuf.st_size);

    msync(dst,statbuf.st_size,MS_SYNC);//MS_ASYNC

    munmap(src,statbuf.st_size);
    munmap(dst,statbuf.st_size);

    close(fdin);
    close(fdout);
    chown(tpathname,statbuf.st_uid,statbuf.st_gid);
    chmod(tpathname,statbuf.st_mode);
    LOG(Log::L_INFO, "Copy_FileWithMMap done");
    return 1 ;
}

void Copy_DirWithMMap(char* spathname,char* tpathname)
{
    LOG(Log::L_DEBUG, "Copy_DirWithMMap Enter");
    char sp1[MAX_PATH]={0};
    char tp1[MAX_PATH]={0} ;
    strcpy(sp1,spathname);
    strcpy(tp1,tpathname);
    struct stat s,t,temp_s,temp_t;
    struct dirent *s_p;
    DIR *dirs,*dirt;

    stat(spathname,&s);
    mkdir(tpathname,s.st_mode);
    chown(tpathname,s.st_uid,s.st_gid);
    dirt=opendir(tpathname);
    dirs=opendir(spathname);
    strcpy(g_temp_paths,spathname);
    strcpy(g_temp_patht,tpathname);
    while((s_p=readdir(dirs))!=NULL)
    {
        if (strcmp(s_p->d_name,".")!=0&&strcmp(s_p->d_name,"..")!=0)
        {
            strcat(g_temp_paths,"/");
            strcat(g_temp_paths,s_p->d_name);
            strcat(g_temp_patht,"/");
            strcat(g_temp_patht,s_p->d_name);
            lstat(g_temp_paths,&s);
            temp_s.st_mode=s.st_mode;
			
            if(S_ISLNK(temp_s.st_mode))
                continue ;
			
			if (S_ISREG(temp_s.st_mode))
            {
                LOG(Log::L_INFO, "begin copy %s to %s ",g_temp_paths,g_temp_patht);
                Copy_FileWithMMap(g_temp_paths,g_temp_patht);
                LOG(Log::L_INFO, " copy %s  ended ",g_temp_paths);
                strcpy(g_temp_paths,sp1);//spathname
                strcpy(g_temp_patht,tp1);//tpathname
            }
            else if(S_ISDIR(temp_s.st_mode))
            {
                Copy_DirWithMMap(g_temp_paths,g_temp_patht);
                strcpy(g_temp_paths,sp1);//spathname
                strcpy(g_temp_patht,tp1);//tpathname
            }
        }
    }
	
    LOG(Log::L_INFO, "Copy_DirWithMMap done");
    return  ;
}

static bool _CopyFile(string _pFrom, string _pTo, short int flags)
{
    LOG(Log::L_INFO, "_CopyFile Enter");
    struct dirent *sp,*tp;
    char spath[MAX_PATH]={0},tpath[MAX_PATH]={0} ;
    char temp_spath[MAX_PATH]={0},temp_tpath[MAX_PATH]={0};

    struct stat sbuf,tbuf,temp_sbuf,temp_tbuf;

    char sdirect[MAX_PATH]={0},tdirect[MAX_PATH]={0};
    // is file 
    strcpy(sdirect,_pFrom.c_str());
    stat(sdirect,&sbuf);
    temp_sbuf.st_mode=sbuf.st_mode;
    if(S_ISLNK(temp_sbuf.st_mode))
    {
        LOG(Log::L_DEBUG, "skipped link file[%s]", _pFrom.c_str());
        return 0 ;
    }   
       
    if((S_IFMT&temp_sbuf.st_mode)==S_IFREG)
    {
        strcpy(temp_spath,_pFrom.c_str()) ;
        char *pFile = strrchr(temp_spath,'/') ;
        strcpy(temp_tpath,_pTo.c_str());
        strcat(temp_tpath,pFile);
        LOG(Log::L_INFO, "begin copy %s to %s ",_pFrom.c_str(),temp_tpath);
        Copy_FileWithMMap(temp_spath,temp_tpath);
        LOG(Log::L_INFO, " copy %s  ended ",_pFrom.c_str());
        return 0 ;
    }
    //
    DIR *dir_s,*dir_t;

    // strcpy(sdirect,_pFrom.c_str());
    dir_s=opendir(sdirect);
    if(dir_s==NULL)
    {
        LOG(Log::L_WARNING, "open %s failed",sdirect);
        return 0;
    }  
	
    if(stat(sdirect,&sbuf)!=0)
    {
        LOG(Log::L_WARNING, "get %s stat failed",sdirect);
        return 0;
    }
	
    strcpy(tdirect,_pTo.c_str());
    dir_t=opendir(tdirect);
    if(dir_t==NULL)
    {  
        mkdir(tdirect,sbuf.st_mode);
        chown(tdirect,sbuf.st_uid,sbuf.st_gid);
        dir_t=opendir(tdirect);
        LOG(Log::L_DEBUG, "created dir[%s]", tdirect);
    }
    else
    {
        LOG(Log::L_DEBUG, "chmod/chown for dir[%s]", tdirect);
        chmod(tdirect, sbuf.st_mode);
        chown(tdirect, sbuf.st_uid, sbuf.st_gid);
	}   
	
    strcpy(spath,sdirect);
    strcpy(tpath,tdirect);
    strcpy(temp_spath,sdirect);
    strcpy(temp_tpath,tdirect);

    while((sp=readdir(dir_s))!=NULL)
    {
        if (strcmp(sp->d_name,".") !=0 && strcmp(sp->d_name,"..") !=0)
        {
            strcat(temp_spath,"/");
            strcat(temp_spath,sp->d_name);
            strcat(temp_tpath,"/");
            strcat(temp_tpath,sp->d_name);
            lstat(temp_spath,&sbuf);
            temp_sbuf.st_mode=sbuf.st_mode;
            if(S_ISLNK(temp_sbuf.st_mode))
            {
                LOG(Log::L_DEBUG, "symbol link[%s] skipped", temp_spath);
                continue ;
            } 
			
            if ((S_IFMT&temp_sbuf.st_mode)==S_IFREG)
            {
                LOG(Log::L_DEBUG, "copying file[%s] to [%s]",temp_spath, temp_tpath);
                Copy_FileWithMMap(temp_spath, temp_tpath);
                LOG(Log::L_INFO, "finished copyting %s", temp_spath);
                strcpy(temp_tpath,tpath);
                strcpy(temp_spath,spath);
            }
            else if((S_IFMT&temp_sbuf.st_mode)==S_IFDIR)
            {
                LOG(Log::L_DEBUG, "copying directory %s to %s ",temp_spath,temp_tpath);
                Copy_DirWithMMap(temp_spath, temp_tpath);
                strcpy(temp_tpath,tpath);
                strcpy(temp_spath,spath);
            }
        }
    }

    closedir(dir_t);
    closedir(dir_s);
    LOG(Log::L_INFO, "_CopyFile done");
    return true ;
}

static bool  _CopyFileExCMD(string _pFrom, string _pTo,short int flags)
{
    LOG(Log::L_DEBUG, "_CopyFileExCMD Enter");
    char exec_buf[MAX_PATH * 2]={0};
    sprintf(exec_buf,"cp -R -f %s  %s", _pFrom.c_str(),_pTo.c_str());
    LOG(Log::L_DEBUG, "executing cmd[%s]", exec_buf);
    system(exec_buf) ;
    LOG(Log::L_INFO, "executed cmd[%s]", exec_buf);
	return true ;   
}

static void DatafileToLogdir(void* pvoid)
{
    //if serviceConfig.backfiles  is NULL ?
    if(serviceConfig.backfiles.empty())
    {
        LOG(Log::L_DEBUG, "skip backing up DB per serviceConfig.backfiles[F]");
        return ;
    }

    LOG(Log::L_DEBUG, "backing up: %s", serviceConfig.backfiles.c_str());
    string tp_logpath=logConfig.path ;
    string tp_signpath;
    std::vector<std::string> result_vecstr;
    ZQ::common::stringHelper::SplitString(serviceConfig.backfiles,result_vecstr,":",":") ;
    vector<string>::iterator itr ;

    for(itr=result_vecstr.begin();itr != result_vecstr.end();itr++)
    {   
        tp_signpath=*itr;
        LOG(Log::L_INFO, "copying %s", tp_signpath.c_str());
        _CopyFileExCMD(tp_signpath,tp_logpath, 0) ;
        //_CopyFile(tp_signpath, tp_logpath, 0);
    }

    LOG(Log::L_INFO, "backed up: %s", serviceConfig.backfiles.c_str());
    return ;
}

int main(int argc, char* argv[]) 
{
    // command line arguments
    if(argc < 2)
	{
        usage();
        return (0);
    }
	
    if(argc == 2 && (!strcmp(argv[1], "--version")))
	{
        fprintf(stderr, "%s\n", VERSION);
        return (0);
    }

    int opt = (-1), delay = 0;
    std::string svcName; 

    while((opt = getopt(argc, argv, "s:d:l")) != (-1))
	{
        switch(opt) {
        case 's':
            VERIFY(optarg[0] != '-', opt);
            svcName = optarg; 
            break;
        case 'd': {
            VERIFY(optarg[0] != '-', opt);
            std::istringstream iss(optarg); 
            iss >> delay;
                  }
                  break;
        case 'l':
            // VERIFY(optarg, opt);
            listService();
            return (0);
        case '?':
        default:
            usage();
            return (1);
        }    
    }

    if(!svcName.empty())
	{
        if(svcName != "all")
            return stopService(svcName, delay)?0:1;
        
        SVCSET set;
        listService(&set);
        SVCSET::const_iterator iter = set.begin();
        for(; iter != set.end(); ++iter)
                stopService(*iter, delay);
                           
        return 0;
    }

    // get Service name
    serviceConfig.service = argv[1];
    serviceConfig.name = serviceConfig.service + "_Shell";

    // starting as a daemon 
    pid_t shell_pid = fork();

    if(shell_pid < 0)
	{
        fprintf(stderr, "failed to fork[%s] %s(%d)\n", 
            serviceConfig.imagePath.c_str(), strerror(errno), errno);
        return (errno);
    }
	
   if(shell_pid > 0)
        _exit(0);   

    // check service lock
    LockFile = std::string(LockPath) + serviceConfig.service + ".pid";
    if(!checkLock(LockFile)) 
	{
        fprintf(stderr, "failed to lock [%s], another instance might be running or permission denied\n", LockFile.c_str());
        LockFile.clear();
        cleanup();
        _exit(1);
    }

    shell_pid = getpid();
	
    // detatch from terminal
    if(setsid() == (-1)) 
	{
        fprintf(stderr, "failed to set session id %s(%d)", strerror(errno), errno);
        return (errno);
    }
	
    // change work-dir to root
    if(chdir("/") < 0)
	{
        fprintf(stderr, "can't change working dir to '/'");
        return (errno);
    }

    // attach file 0,1,2 to null
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

    // init system log
    openlog(serviceConfig.name.c_str(), LOG_PID, LOG_DAEMON);

    // signals
    if(!initSignals())
	{
        syslog(LOG_ERR, "failed to init signals %s(%d)", strerror(errno), errno);
        return (-1);
    }

    //  load configuration
    initConfig();
	bool bLoadConfig = false;
    try{
		bLoadConfig = loadConfig();
    }
    catch(const ZQ::common::XMLException& ex)
	{
        syslog(LOG_ERR, "%s", ex.getString());
    }
    catch(const std::exception& ex) {
        syslog(LOG_ERR, "%s", ex.what());   
    }
    catch(...) {
        syslog(LOG_ERR, "%s", "error occured when loading config");    
    }
	
    if(!bLoadConfig)
	{
		cleanup();
		return (-1);
	}

#ifdef _DEBUG
    printConfig();
#endif

	// init shell log
    if(logConfig.loggingMask)
    {
        try
		{
            logConfig.logger = new FileLog(logConfig.name.c_str(), logConfig.level, ZQLOG_DEFAULT_FILENUM, logConfig.size);
        }
        catch(const FileLogException& ex)
		{
            syslog(LOG_ERR, "failed to initialize log[%s]: %s", logConfig.name.c_str(), ex.getString());
            cleanup();
            return (-1);
        }
    }

    unsigned int svcInstanceId = 0;
    if (!serviceConfig.svcInstanceId.empty())
    {
        svcInstanceId = atoi(serviceConfig.svcInstanceId.c_str());
    } 
    else if (!serviceConfig.service.empty())
    {
        size_t pos = serviceConfig.service.find_first_of("0123456789");
        if (std::string::npos == pos)
            svcInstanceId = 0;
        else
            svcInstanceId = atoi(serviceConfig.service.substr(pos).c_str());

        //std::istringstream svcInstanceIs;
        const unsigned int svcMaxLimit = 10;
        //svcInstanceIs.str(serviceConfig.svcInstanceId);
        //svcInstanceIs >> svcInstanceId;
        svcInstanceId = (svcInstanceId < svcMaxLimit) ? svcInstanceId : 0;

        std::ostringstream oss;
        oss << svcInstanceId;
        serviceConfig.svcInstanceId = oss.str();
    }

    // register snmp
    mmib = new ZQ::SNMP::ModuleMIB(*logConfig.logger, serviceConfig.componentId, ZQ::SNMP::ModuleMIB::ModuleOid_SvcShell, svcInstanceId);
    snmpSubAgent = new ZQ::SNMP::SubAgent(*logConfig.logger, *mmib, 5000); // timeout 5000ms

    mmib->addObject(new ZQ::SNMP::SNMPObject("loggingMask",         (int32&)logConfig.loggingMask),   ".1");
    mmib->addObject(new ZQ::SNMP::SNMPObject("snmpLoggingMask",     (int32&)logConfig.snmpLoggingMask),   ".2");
    mmib->addObject(new ZQ::SNMP::SNMPObject("logDir",              logConfig.path),  ".3");
    mmib->addObject(new ZQ::SNMP::SNMPObject("logSize",             (uint64&)logConfig.size),   ".4");
    mmib->addObject(new ZQ::SNMP::SNMPObject("logLevel",            (int32&)logConfig.level),   ".5");

    mmib->addObject(new ZQ::SNMP::SNMPObject("imagePath",           serviceConfig.imagePath),  ".6");
    mmib->addObject(new ZQ::SNMP::SNMPObject("aliveTimeout",        (uint32&)serviceConfig.aliveTimeout),   ".7");
    mmib->addObject(new ZQ::SNMP::SNMPObject("restartTries",        (uint32&)serviceConfig.restartTries),   ".8");
    mmib->addObject(new ZQ::SNMP::SNMPObject("restartInterval",     (uint32&)serviceConfig.restartInterval),   ".9");
    mmib->addObject(new ZQ::SNMP::SNMPObject("startupWait",         (uint32&)serviceConfig.startupWait),   ".10");
    mmib->addObject(new ZQ::SNMP::SNMPObject("backupFiles",         serviceConfig.backfiles),  ".11");

    mmib->addObject(new ZQ::SNMP::SNMPObject("startTimeout",        (uint32&)serviceConfig.restartTimeout),   ".12");

    snmpSubAgent->start();

    // loop to wait for heartbeats
    time_t stampNow = time(NULL);
    time_t start = stampNow;
    time_t end   = stampNow;

    int restartCount = 0;

    while (!stop) 
    {
        stampNow = time(NULL);
        char buf[100];

        //      double elapsed = difftime(end, start);
        time_t elapsed = stampNow - start;

        if (elapsed > serviceConfig.restartTimeout || ++restartCount > serviceConfig.restartTries)
        {
            snprintf(buf, sizeof(buf)-2, "service[%s] failed to start by [%d]retries within [%d]sec", argv[1], restartCount-1, (int)elapsed);
            ERR(buf);
            stop = 1;
            break;
        }

        // determin the sleep time (interval between two start retries) piror the this start
        int sleepPriorRestart = (restartCount-3)*10;
        if (sleepPriorRestart > (int)serviceConfig.restartInterval)
            sleepPriorRestart = serviceConfig.restartInterval;
        else if (sleepPriorRestart <=0)
            sleepPriorRestart = 1;

        if (logConfig.loggingMask) 
            LOG(Log::L_INFO, "service[%s] creating process[%s] at %d-th try, retry-interval[%d]sec", argv[1], serviceConfig.imagePath.c_str(), restartCount, sleepPriorRestart);

        // open system events
        if(!initEvents())
        {
            cleanup();
            return (errno);
        }

        struct timespec t;

        // sleep between restarts
        if (sleepPriorRestart >0)
        {
            t.tv_sec = stampNow + sleepPriorRestart;
            t.tv_nsec = 0;
            sem_timedwait(aliveEvent, &t);
        }

        // starting the service process
        svc_pid = startProcess();
        if (svc_pid == (-1))
        {
            // error occur to run the program, quit directly
            cleanup();
            return (errno);
        }

        // reopen syslog for child process
        openlog(serviceConfig.name.c_str(), LOG_PID, LOG_DAEMON);

        // see if service not successfully started
        sleep(serviceConfig.startupWait);

        int res = waitpid(svc_pid, NULL, WNOHANG);

        // case 1. service process started but soon dead
        if (res == svc_pid)
        { 
            snprintf(buf, sizeof(buf)-2, "service[%s] child[%s] started but quit soon", argv[1], serviceConfig.imagePath.c_str());
            ERR(buf);
            continue;
        }

        // not a child, could not happen?
        if (res != 0)
        {
            snprintf(buf, sizeof(buf)-2, "service[%s] invalid child[%s] waitpid[%d] err[%d]%s", argv[1], serviceConfig.imagePath.c_str(), res, errno, strerror(errno));
            ERR(buf);
            continue;
        }

        // successfully created the child if reached here
        snprintf(buf, sizeof(buf)-2, "service[%s] shell[%d] image[%s] pid[%d] started", argv[1], shell_pid, serviceConfig.imagePath.c_str(), svc_pid);
        INFO(buf);

        // start receiving the heatbeats from the service process
        int timeoutCountLeft =3;
        int heartbeatCount = 0 ;
        while(!stop) //loop of receiving heart beat
        {
            stampNow = time(&stampNow);

            // renew wait time every time getting signaled
            t.tv_sec = stampNow + serviceConfig.aliveTimeout;
            t.tv_nsec = 0;
            if (sem_timedwait(aliveEvent, &t) == 0)
            {
                timeoutCountLeft =3; // reset the timeout count if got a heartbeat here
                if( ++ heartbeatCount > 10 )
                    restartCount =0;
            }
            else
            {
                // timeout receiving any heartbeat
                if (errno == ETIMEDOUT)
                {
                    if (--timeoutCountLeft >0)
                        continue;

                    if(logConfig.loggingMask) 
                    {
                        LOG(Log::L_INFO, "service[%s] shell[%d] heartbeat timeout, stopping service", argv[1], shell_pid);
                    }

                    // stop service
                    sem_post(stopEvent);
                    int status;
                    waitpid(svc_pid, &status, 0);

                    break;
                }

                // received SIGCHLD, quit the while
                if(errno == EINTR) 
                    break;
            }

        } // end of while(true) loop for heartbeats
        
        start = time(0);//reset start so that the target app can be start again

        if(!stop && restartCount == 0 )
        {
            LOG(Log::L_INFO, "service[%s] copy data-db before serviceApp restart.....", argv[1]);
            DatafileToLogdir(NULL) ;
        }

    } // while(!stop)

    cleanup();

    return 0;
}

// vim: ts=4 sw=4 bg=dark
