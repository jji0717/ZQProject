#include "ZQDaemon.h"
#include <syslog.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <semaphore.h>
#include <sstream>
#include <algorithm>
#include <sys/time.h>
#include <sys/resource.h>

#include "version.h"

using namespace ZQ::common;

extern XMLPreferenceEx* getPreferenceNode(const std::string& path, XMLPreferenceDocumentEx& config);
extern bool checkLock(const std::string& file);
extern void app_main(int, char**);

const char* DUMP_PATH = 0;


namespace {
    const char* ConfigFile = "/etc/TianShan.xml";
	const char* LockPath = "/var/run/";

    unsigned AppAliveWait = 30; // seconds

    sem_t* stopEvent = 0;
    sem_t* aliveEvent = 0;
    sem_t* appStopEvent = 0;

    std::string appStop;
    std::string stop;
    std::string alive;

	std::string LockFile;

    bool shellMode = false;
    
    int app_argc = 0;
    char* app_argv[] = {NULL, NULL, NULL, NULL};
}

extern BaseZQServiceApplication* Application;

static void terminate(int signal, siginfo_t* info, void* data) {
    sem_post(stopEvent);
}

void coredump(int signal, siginfo_t* info, void* data) {
    glog.flush();

    struct rlimit rlim;
    
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);

    /* ignore any error here */
    int fd = open("/proc/sys/kernel/core_pattern", O_WRONLY); 
    if(fd != (-1)) {
        write(fd, "core.%e.%s.%t", 14);
    }


    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler=SIG_DFL;

    sigaction(signal, &act, 0);
    raise(signal);
}

bool initSignals(bool shellmode) {
    /*
     * install signals
     */
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;

    if(!shellmode) {
        act.sa_flags |= SA_SIGINFO;
        act.sa_sigaction = terminate;
    }
    else {
        act.sa_handler = SIG_IGN;
    }

    if(sigaction(SIGTERM, &act, 0) < 0 || 
       sigaction(SIGINT,  &act, 0) < 0 ||  
       sigaction(SIGHUP,  &act, 0) < 0) { 
        return false;
    }

    act.sa_sigaction=coredump;
    act.sa_flags |= SA_SIGINFO;
    if(sigaction(SIGSEGV, &act, 0) < 0) {
        return false;
    }
    if(sigaction(SIGABRT, &act, 0) < 0) {
        return false;
    }
    
    return true;
}

void app_main(int argc, char* argv[]) {
    if(DUMP_PATH) {
        chdir(DUMP_PATH);
    }
   	else {
		char path1[64] = {0};
		sprintf(path1, "/proc/%d/exe", getpid());

		char path2[PATH_MAX] = {0};
		readlink(path1, path2, PATH_MAX-1);

		char* p = strstr(path2, "/bin");
		if(p) {
			*p = '\0';
			strcat(path2, "/logs/crashdump");
			chdir(path2);
		}
	} 


	uint32 instanceId = 0;
	if( argc > 0 )
	{
		instanceId = (uint32)atoi(argv[0]);
		argc--;
		argv++;
	}
	Application->setInstanceId( instanceId );

    bool res = Application->init(argc, argv);

    if(!res) {
        sem_post(stopEvent);
        return; 
    }
    
    res = Application->start();
	if(res) {
		Application->stop();
	}

    Application->unInit();

    sem_post(stopEvent);
}

void* service_main(void*) {
    app_main(app_argc, app_argv);

    return (void*)0;
}

void* heartbeat(void* arg) {
    unsigned appAliveWait = *((unsigned*)arg);
    
    timespec t;
    t.tv_nsec = 0;

wait:
    t.tv_sec = time(0) + appAliveWait;

    if(sem_timedwait(stopEvent, &t) == (-1)) {
        if(errno == ETIMEDOUT) {
            sem_post(aliveEvent);
		
			Application->isHealth();
            goto wait;
        }
        if(errno == EINTR) {
            goto wait;
        }
    }
    sem_post(appStopEvent);

    return (void*)0;
}


static void cleanup() {
    closelog();
	
	if(!LockFile.empty()) {
		remove(LockFile.c_str());
	}

    if(appStopEvent) {
        sem_unlink(appStop.c_str());
    }
    if(!shellMode) {
        if(stopEvent) {
            sem_unlink(stop.c_str());
        }
        if(aliveEvent) {
            sem_unlink(alive.c_str());
        }
    }
}

void usage()
{
    fprintf(stderr, "service -s <serviceName> -p <productName> -i <instanceId> -n <netId>\n\n");
    fprintf(stderr, "-s <serviceName>     Service Name\n");
    fprintf(stderr, "-p <productName>     Product Name\n");
    fprintf(stderr, "-i <instanceId>      Instance ID\n");
    fprintf(stderr, "-n <netId>           NetID\n");
    fprintf(stderr, "-v                   Version\n");
    fprintf(stderr, "-h                   Help\n");
}

int main(int argc, char* argv[]) {
    /* get my name */
    char* name = 0;
    if((name = std::strrchr(argv[0], '/')) != 0) {
        ++name;
    }
    else {
        name = argv[0];
    }

    app_argc = 1;
    app_argv[0]  ="0";

    std::string serviceName;
    std::string productName;

    int ch;
    while((ch = getopt(argc, argv, "hvms:p:i:n:")) != EOF)
    {
        switch (ch)
        {
        case '?':
        case 'h':
            fprintf(stderr, "invalid parameters\n");
            fprintf(stderr, "%s -s <serviceName>\n", name);
            return (0);

        case 'v':
            fprintf(stderr, "%s\n", VERSION);
            return (0);

        case 'm':
            shellMode = true;
            break;

        case 's':
            if (!optarg)
            {
                usage();
                return 0;
            }else{
                serviceName = optarg;
            }
            break;

        case 'p':
            if (!optarg)
            {
                usage();
                return 0;
            }else{
                productName = optarg;
            }
            break; 

        case 'i':
            app_argv[0] = optarg ? optarg : (char*)"0";
            break;

        case 'n':
            if (optarg)
            {
                app_argv[1]  = optarg;
                app_argc =2;
            }
            break;
        }
    }
    
    Application->setServiceName(serviceName);

    openlog(serviceName.c_str(), LOG_PID, LOG_DAEMON);
	
    if(!shellMode) {
        LockFile = std::string(LockPath) + serviceName + ".pid";		
        if(!checkLock(LockFile)) {
            fprintf(stderr, "unable to lock [%s], another instance might be running or permission denied\n", LockFile.c_str());
            cleanup();
            return (-1);
        }
    }

    struct rlimit rlim;
    
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);

    /*
     * init signals
     */
    if(!initSignals(shellMode)) {
        syslog(LOG_ERR, "failed to init signals");

        cleanup();
        return (-1);
    }

    /*
     * init semaphores
     */
    mode_t mode = shellMode ? 0 : O_CREAT|O_EXCL;

    stop = serviceName + "_Stop";
sem_stop:
    stopEvent = sem_open(stop.c_str(), mode, 0664, 0);
    if(stopEvent == SEM_FAILED) {
        if(errno == EEXIST) {
            syslog(LOG_WARNING, "semphore (%s) already exists", stop.c_str());
            sem_unlink(stop.c_str()); 
            
            goto sem_stop;
        }
        else {
            syslog(LOG_ERR, "failed to open stop event (%s): (%s)", stop.c_str(), strerror(errno));
            closelog(); 

            return (-1);
        }
    }

    alive = serviceName + "_Alive";
sem_alive:
    aliveEvent = sem_open(alive.c_str(), mode, 0664, 0);
    if(aliveEvent == SEM_FAILED) {
        if(errno == EEXIST) {
            syslog(LOG_WARNING, "semphore (%s) already exists", alive.c_str());
            sem_unlink(alive.c_str()); 
            
            goto sem_alive;
        }
        else {
            syslog(LOG_ERR, "failed to open alive event (%s): (%s)", alive.c_str(), strerror(errno));
            closelog(); 

            return (-1);
        }
    }

    appStop = serviceName + "APP_Stop";
sem_appStop:
    appStopEvent = sem_open(appStop.c_str(), O_CREAT|O_EXCL, 0664, 0);
    if(appStopEvent == SEM_FAILED) {
        if(errno == EEXIST) {
            syslog(LOG_WARNING, "semphore (%s) already exists", appStop.c_str());
            sem_unlink(appStop.c_str()); 
            
            goto sem_appStop;
        }
        else {
            syslog(LOG_ERR, "failed to open appStop event (%s): (%s)", appStop.c_str(), strerror(errno));
            closelog(); 

            return (-1);
        }
    }

    /*
     * load configuration
     */
    XMLPreferenceDocumentEx doc;
    try {
        if(!doc.open(ConfigFile)) {
            syslog(LOG_ERR, "failed to load configuration (%s)", ConfigFile);
            cleanup();
            return (-1);
        }
    } 
    catch (XMLException&) {
        syslog(LOG_ERR, "failed to load configuration (%s)", ConfigFile);
        cleanup();
        return (-1);
    }

    std::string nodeName = serviceName + "_Shell/service";
    XMLPreferenceEx* node = getPreferenceNode(nodeName, doc);
    if(!node) {
        syslog(LOG_ERR, "failed to get service configuration");
        cleanup();
        return (-1);
    }

    char value[255];
    memset(value, '\0', sizeof(value));

    bool res = false;

    /* appAliveWait */
    unsigned appAliveWait = AppAliveWait;
    res = node->getAttributeValue("appAliveWait", value);
    if(!res) {
        syslog(LOG_WARNING, "failed to get alive wait time");
    }
    std::istringstream is(value);
    is >> appAliveWait;

    node->free();
    
    /*
     * starting service
     */
    pthread_t app;
    res = pthread_create(&app, 0, service_main, 0); 
    if(res) {
        syslog(LOG_ERR, "failed to start service thread");

        cleanup();
        return (res);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t hb;
    res = pthread_create(&hb, &attr, heartbeat, &appAliveWait);
    if(res) {
        syslog(LOG_ERR, "failed to start heartbeat thread");
        
        /* shall I stop the service thread? */
        pthread_kill(app, SIGTERM);
    }
    pthread_attr_destroy(&attr);

    pthread_join(app, 0);

    cleanup();

    return 0;
}

// vim: ts=4 sw=4 nu bg=dark

