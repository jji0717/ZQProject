
//TailTrigger.h
//Daniel Wang
#ifndef _D_LIB_TAIL_TRIGGER_H_
#define _D_LIB_TAIL_TRIGGER_H_

#include <vector>
#include <string>
#include <windows.h>

namespace ZQ
{
	namespace common
	{
		DWORD __stdcall LogParserProc(void* pVoid);

		//TailTrigger
		class TailTrigger: public std::vector<std::string>
		{
		private:
			char	m_strLogFile[MAX_PATH];
			HANDLE	m_hMoniter;
			bool	m_bRun;
			char	m_strSSFile[MAX_PATH];
			DWORD	m_dwThreadId;

		public:
			//constructors and distructor
			TailTrigger();

			TailTrigger(const char* strLogFile, const char* strSSFile);

			virtual ~TailTrigger();

			//set log file name
			void setLogFile(const char* strLogFile);

			//read log file name
			const char* getLogFile();

			//[call back function]
			//if detected a log line which match the syntax line
			//strFileName -- log file name
			//strSyntax -- syntax line
			//strLine -- current log line
			//arrParam -- the paraments which defined in ini file
			//nBlock -- in which ini block
			virtual bool OnAction(const char* strFileName, const char* strSyntax, const char* strLine, const std::vector<std::string>& arrParam, int nBlock) = 0;

			//the main function which detected the log file
			void ParserProc(void);

			//run parser process
			bool run();

			//stop parser process
			bool stop();

			// wait for thread exit
			void unInit();

			//return true if in running
			bool isRun();
		};
	}
}

#endif//_D_LIB_TAIL_TRIGGER_H_
