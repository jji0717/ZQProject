#ifndef __COMMAND_LINE__
#define __COMMAND_LINE__

#include <string>
#include <getopt.h>

#define STR2INT(__str, __dest) { \
	std::istringstream is(__str);	\
	is >> __dest;	\
} 

namespace {
	const std::string OPTIONS = "s:n:d:t:l:b:c:p:fDoh";
	const unsigned DEFAULT_TIMEOUT = 10; /* in seconds */
}


class CommandLine {
	
public:
	
	CommandLine():
		_options(OPTIONS),
		_port(0),
		_timeout(DEFAULT_TIMEOUT),
		_duration(3600),
		_concurrentCount(1),
		_oldTrickType(false),
		_runningOnNode(true),
		_streamablePlaytime(10),
		_trace(false){
	}
		
	~CommandLine(){
	}

	bool parse(int argc, char** argv);
	void usage() const;
	
	void _print() const;

	inline std::string error() const {
		return _error;
	}

public:
	
	inline std::string url() const {
		return _url;
	}
	inline std::string IP() const {
		return _IP;
	}
	inline unsigned short port() const {
		return _port;
	}
	inline std::string name() const {
		return _name;
	}
	inline unsigned timeout() const {
		return _timeout;
	}
	inline bool dumpFile() const {
		return _dumpFile;
	}
	inline unsigned duration() const {
		return _duration;
	}
	inline std::string log() const {
		return _log;
	}
	inline std::string localIP() const {
		return _interface;
	}
	inline bool trace() const {
		return _trace;
	}

	inline unsigned int concurrentCount() const {
		return _concurrentCount;
	}

	inline bool oldTrickType() const {
		return _oldTrickType;
	}

	inline bool isRunningOnNode() const {
		return _runningOnNode;
	}

	inline unsigned int streamablePlaytime() const {
		return _streamablePlaytime;
	}
private:
	const std::string _options;
	
	std::string _url;
	std::string _IP;
	unsigned short _port;
	std::string _name;
	unsigned _timeout; /* in seconds */
	bool _dumpFile;
	unsigned _duration;
	std::string _log;
	std::string _interface;
	unsigned short _concurrentCount;
	bool _trace;
	unsigned short _streamablePlaytime;

	bool _oldTrickType;  // trick type: false - RTFLib, true - TrickFilesUserLibrary
	bool _runningOnNode; //
	std::string _error;
	
};


bool CommandLine::parse(int argc, char** argv) {
	if(argc == 1) {
		_error.assign("no parameters specified");
		return false;
	}

	int ch;
	while((ch = getopt(argc, argv, _options.c_str())) != EOF) {
		switch (ch) {
		
		case 'h':
			return false;

		case 's':
			_url = optarg;
			{
				char* __t = strchr(optarg, ':');
				
				if(__t) {
					STR2INT(__t+1, _port)
				
					_IP.assign(optarg, __t-optarg);
				}
			}
			break;
		
		case 'n':
			_name = optarg;
			break;
			
		case 'f':
			_dumpFile = true;
			break;

		case 'd':
			STR2INT(optarg, _duration)
			break;

		case 't':
			STR2INT(optarg, _timeout)
			break;
		
		case 'l':
			_log = optarg;
			break;
		
		case 'b':
			_interface = optarg;
			break;
		
		case 'D':
			_trace = true;
			break;

		case 'c':
			STR2INT(optarg, _concurrentCount)
			if(_concurrentCount == 0)
				_concurrentCount = 1;
			
			break;

		case 'p':
			STR2INT(optarg, _streamablePlaytime);
			if(0 == _streamablePlaytime)
				_streamablePlaytime = 10;
			
			break;
		case 'o':
			_oldTrickType = true;
			break;

		case 'N':
			_runningOnNode = false;
			break;

		default:
			_error.assign("invalid parameter");
			return false;
		}
	}
	
	if(_url.empty()  || 
	   _IP.empty()   || 
	   !_port		 ||
	   _name.empty()) {

		_error.assign("invalid url or empty name");
		
		return false;
	}

	return true;
}
#undef STR2INT


void CommandLine::usage() const {
	std::cerr << "\nusage: McastIngest " 
			  << "-s <MulticastIP>:<port> " 
			  << "-n <name> " 
			  << std::endl;

	std::cerr << "\n  -s <IP:port>  multicast IP and (base) port (*)"
			  << "\n  -n <name>     content name (*)"
			  << "\n  -d <duration> duration of the capture in seconds, defauls is 3600"
			  << "\n  -t <timeout>  timeout in seconds, defaults to 10 seconds"
			  << "\n  -b <IP>       bind a specific local interface to receive packets"
			  << "\n  -l <path>     path to log file folder, default to the current folder"
			  << "\n  -c <count>    the supported concurrent multicast captures, port is from base_port to base_port + count"
			  << "\n  -p <seconds>  the playtime of firing streamable(playable) event, default is 10 seconds"
			  << "\n  -f            turn on local copy of the stream, file name is same to <name>"
			  << "\n  -o            turn to use TrickFilesUserLibrary to do trick generation, default is RTFLib"
//			  << "\n  -N            the generated trick files output to ntfs, default is vstrm if not specified"
			  << "\n  -D            turn on trace log"
			  << "\n  -h            display this help"
			  << std::endl;
}

void CommandLine::_print() const {
	std::cout << "options: " << _options << "\n" 
	<< "URL: " << _url << "\n" 
	<< "IP: " << _IP << "\n" 
	<< "port: " << _port << "\n" 
	<< "name: " << _name << "\n" 
	<< "timeout: " << _timeout << "\n" 
	<< "duration: " << ((!_duration)?INFINITE:_duration) << "\n" 
	<< "StreamablePlaytime: " << _streamablePlaytime << "\n"
	<< "log: " << (_log.empty()?".":_log) << "\n"
	<< "local: " << (_interface.empty()?"DEFAULT":_interface)
	<< std::endl;
}

#endif