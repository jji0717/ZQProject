#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "Hammer.h"
#include "getopt.h"


#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif

static void usage() {
	printf("\n"
		   "-h\tshow this help\n"
		   "-c\tscript file path\n"
		   "-l\tlog file path\n"
   		   "-n\tdo not print message\n\n");
}

int main(int argc, char* argv[]) {

#ifdef ZQ_OS_MSWIN
	ZQ::common::MiniDump dump;
	dump.setDumpPath(".");
#else
	signal(SIGPIPE, SIG_IGN);
#endif

	std::string hammerScript, logPath;
	bool print = true;
	int opt = 0;
	while((opt = getopt(argc, argv, "hc:l:n")) != (-1)) {
		switch(opt) {
		case 'h':
			usage();
			return (0);
		case 'c':
			hammerScript = optarg;
			break;
		case 'l':
			logPath = optarg;
			break;
		case 'n':
			print = false;
			break;
		default:
			fprintf(stderr, "unknown option: -?\n");
			usage();
			return (-1);
		};
	}

	if(hammerScript.empty()) {
		fprintf(stderr, "please specify script file\n");
		usage();	
		return (1);
	}

	try {
		ScriptParser parser(hammerScript);
		
		std::string hammerLog = (logPath.empty())?(parser.logger.name):(logPath+FNSEPS+parser.logger.name);
		
		ZQ::common::FileLog logger(hammerLog.c_str(), parser.logger.level, parser.logger.count, parser.logger.size);
		ZQ::common::NativeThreadPool hammerPool;

		Hammer hammer(parser, logger, hammerPool, print);
		hammer.start();
	}
	catch(const ZQ::common::ExpatException& e) {
		fprintf(stderr, "%s\n", e.what());
	}
	catch(const std::exception& e) {
		fprintf(stderr, "%s\n", e.what());
	} 
	catch(...) {
		fprintf(stderr, "unknown error\n");
	}

	return (0);
}

// vim: ts=4 sw=4 bg=dark nu
