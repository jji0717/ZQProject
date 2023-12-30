
#include <screporter.h>
//#include <regex.h>
#include <boost/regex.hpp>
#include <assert.h>
#include <exception>

#include "TailTrigger.h"
#include "formatstr.h"
#include "safemalloc.h"
#include "..\SNMPService\readline.h"
#include "LogScanPos.h"

#define DEF_NUM_BYTES_TO_READ 1024
#define MAX_LINE_LEN 1024
#define SIZE_OF_CIRC_POS 10

#define  DEFAULT_COMPLAIN_TIME		3

namespace ZQ
{
	namespace common
	{
		bool TailTriggerCheck(const char* const strSyntax, const char* strSource, std::vector<std::string>& arrParaments)
		{
			if (!(strSyntax && strSource))
				return false;
	
			boost::regex	_regSyntax;
			try
			{
				_regSyntax.assign(strSyntax);
			}
			catch(boost::bad_expression& ex)
			{
				glog(Log::L_ERROR, "Syntax [%s] error at %s", strSyntax, ex.what());
				return false;
			}

			if (_regSyntax.empty())
			{
				glog(Log::L_ERROR, "Syntax [%s], no syntax found", strSyntax);
				return false;
			}

			typedef boost::match_results<const char*> res_t;

			res_t results;
			std::string value;
			
			if (!boost::regex_match(strSource, results, _regSyntax))
				return false;
						
			std::string strParament;
			arrParaments.clear();
			for (int i = 1; i < results.size(); ++i)
			{
				strParament.assign(results[i].first, results[i].second);
				arrParaments.push_back(strParament);
			}
			return true;
		}
		
		bool TailTriggerCheck(const boost::regex& regSyntax, const char* strSource, std::vector<std::string>& arrParaments)
		{
			if (!strSource || regSyntax.empty())
				return false;
	
			typedef boost::match_results<const char*> res_t;

			res_t results;
			std::string value;
			
			if (!boost::regex_match(strSource, results, regSyntax))
				return false;
						
			std::string strParament;
			arrParaments.clear();
			for (int i = 1; i < results.size(); ++i)
			{
				strParament.assign(results[i].first, results[i].second);
				arrParaments.push_back(strParament);
			}
			return true;
		}

		void FormatCopy(const char* strSource, char*& strDest, int nMaxLen, const std::vector<std::string>& arrLineParaments)
		{
			if (NULL != strSource)
			{
				std::string strFormat = FormatString(strSource, arrLineParaments);
				safe_new(strDest, nMaxLen);
				strncpy(strDest, strFormat.c_str(), nMaxLen-1);
			}
		}
		
		TailTrigger::TailTrigger()
			:m_hMoniter(NULL), m_bRun(false)
		{
			m_strLogFile[0] = '\0';
			m_strSSFile[0] = '\0';
		}
		
		TailTrigger::TailTrigger(const char* strLogFile, const char* strSSFile)
			:m_hMoniter(NULL), m_bRun(false)
		{
			strncpy(m_strLogFile, strLogFile, MAX_PATH);
			strncpy(m_strSSFile, strSSFile, MAX_PATH);
		}
		
		TailTrigger::~TailTrigger()
		{
			unInit();
		}
		
		void TailTrigger::setLogFile(const char* strLogFile)
		{ 
			strncpy(m_strLogFile, strLogFile, MAX_PATH);
		}
		
		const char* TailTrigger::getLogFile()
		{
			return m_strLogFile; 
		}
		
		DWORD __stdcall LogParserProc(void* pVoid)
		{
			assert(pVoid);
			
			TailTrigger* pLogReg = static_cast<TailTrigger*>(pVoid);
			pLogReg->ParserProc();
			
			return 0;
		}
		
		void TailTrigger::ParserProc(void)
		{			
			HANDLE	hLogFile = NULL;
			char	szBuf[DEF_NUM_BYTES_TO_READ] = {0};
			DWORD	dwNumBytesRead = 0;
			DWORD	dwCircpos = 0;
			char	strLine[MAX_LINE_LEN] = {0};
			std::vector<boost::regex>  regSynatxs;
			{
				int nSyntax = size();
				regSynatxs.resize(nSyntax);
				for(int i=0;i<nSyntax;i++)
				{
					const char* strSyntax = this->operator[](i).c_str();
					try
					{
						regSynatxs[i].assign(strSyntax);
					}
					catch(boost::bad_expression& ex)
					{
						glog(Log::L_ERROR, "Syntax [%Ss error at %s", strSyntax, ex.what());
						continue;
					}

					if (regSynatxs[i].empty())
					{
						glog(Log::L_ERROR, "Syntax [%s], no syntax found", strSyntax);
					}
				}
			}

			if (!m_strLogFile[0])
			{
				glog(ZQ::common::Log::L_ERROR, "LogLocation option is empty or not exists in INI file");
				return;
			}
			
			
			//
			// get safestore position
			//
			LogScanPos  logPos;
			logPos.init(m_strSSFile);

			bool bSSValid;
			bool bBootUp = true;
			int nSSByteOffset;
			std::string  strSSLine;
			bSSValid = logPos.getSafeStoreScanPos(nSSByteOffset, strSSLine);
			//time stamp
			::std::string strTimeStamp;
			{
				//check the valid safestore data
				int nMonth,nDay,nHour,nMinute,nSecond,nMilliSec;
				if(sscanf(strSSLine.c_str(), "%2d/%2d %2d:%2d:%2d:%3d", &nMonth,&nDay,&nHour,&nMinute,&nSecond,&nMilliSec)==6)
					strTimeStamp = strSSLine.substr(0,18);
				else
				{
					glog(ZQ::common::Log::L_WARNING, "Safestore line[%s] is invalid", strSSLine.c_str());
				}
			}
			

			int	nLogFilePosition = -1;
			int nComplainTime = DEFAULT_COMPLAIN_TIME;
			
			while (m_bRun)
			{
				hLogFile = CreateFileA(m_strLogFile,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				
				if (hLogFile == INVALID_HANDLE_VALUE)
				{						
					if (nComplainTime>0)
					{
						DWORD dwRet = GetLastError();
						if (dwRet == ERROR_FILE_NOT_FOUND)
							glog(ZQ::common::Log::L_WARNING, "Can not open log file - Filename: %s ,file not found", m_strLogFile);
						else if (dwRet == ERROR_PATH_NOT_FOUND)
							glog(ZQ::common::Log::L_WARNING, "Can not open log file - Filename: %s ,path not found", m_strLogFile);
						else if (dwRet == ERROR_ACCESS_DENIED)
							glog(ZQ::common::Log::L_WARNING, "Can not open log file - Filename: %s ,access is denied", m_strLogFile);
						else
							glog(ZQ::common::Log::L_WARNING, "Can not open log file - Filename: %s , error code: %d", m_strLogFile, GetLastError());

						nComplainTime--;
					}
					nLogFilePosition=-1;

					Sleep(500);
					
					continue;
				}
				else
				{
					nComplainTime = DEFAULT_COMPLAIN_TIME;
				}

				int nLogEnd = GetLogEnd(hLogFile);
				if (nLogEnd <= 0)
				{
					CloseHandle(hLogFile);
					hLogFile = INVALID_HANDLE_VALUE;
					
					Sleep(250);
					
					continue;
				}
				
				if (nLogFilePosition == -1)
				{
					//means we just started
					if (bBootUp)
					{
						bBootUp = false;
						if (bSSValid)
						{
							bool bFoundSSPos = false;
							DWORD dwRet = SetFilePointer(hLogFile, nSSByteOffset - strSSLine.length() - 2, 0, FILE_BEGIN);
							if (dwRet != INVALID_SET_FILE_POINTER)
							{
								if (ReadLogLine(hLogFile, strLine, MAX_LINE_LEN) >= 0)
								{
									if (strSSLine == strLine)
									{
										bFoundSSPos = true;
									}
								}
							}

							if (bFoundSSPos)
							{
								nLogFilePosition = (int)SetFilePointer(hLogFile, 0, 0, FILE_CURRENT);
		
								//continue the general operation
								CloseHandle(hLogFile);
								hLogFile = INVALID_HANDLE_VALUE;
								continue;
							}

							//
							// get to the first log line, so we can read the whole log
							//
							GoFirstLogLine(hLogFile);

							nLogFilePosition = (int)SetFilePointer(hLogFile, 0, 0, FILE_CURRENT);

							//continue the general operation
							CloseHandle(hLogFile);
							hLogFile = INVALID_HANDLE_VALUE;
							continue;
						}
					}

					nLogFilePosition = SKIP_HEAD_CHARS + 2;
					
					CloseHandle(hLogFile);
					hLogFile = INVALID_HANDLE_VALUE;
					
					continue;
				}
				
				SetFilePointer(hLogFile, nLogFilePosition, 0, FILE_BEGIN);
				
				strLine[0]='\0';
				while (ReadLogLine(hLogFile, strLine, MAX_LINE_LEN) >= 0)
				{
					int nMonth,nDay,nHour,nMinute,nSecond,nMilliSec;
					if(sscanf(strLine, "%2d/%2d %2d:%2d:%2d:%3d", &nMonth,&nDay,&nHour,&nMinute,&nSecond,&nMilliSec)!=6)
					{
						continue;
					}

					if(strTimeStamp.compare(0, 18, strLine) > 0)
					{
						int nMonth1;
						sscanf(strTimeStamp.c_str(), "%2d", &nMonth1);
						if(nMonth1 - nMonth < 7)
						{
							glog(ZQ::common::Log::L_WARNING, "Line[%s] timestamp less then safestore_line_timestamp[%s]", strLine, strTimeStamp.c_str());
							continue;
						}						
					}					

					strTimeStamp.assign(strLine, 18);
//					glog(ZQ::common::Log::L_INFO, "Trigger Line : %s", strLine);
					std::vector<std::string> arrParaments;
					bool bActioned = false;
					for (int i = 0; i < size(); ++i)
					{
//						glog(ZQ::common::Log::L_INFO, "Match compare to syntax line : %s", this->operator[](i).c_str());
						if (TailTriggerCheck(regSynatxs[i], strLine, arrParaments))
						{
							bActioned = OnAction(m_strLogFile, this->operator[](i).c_str(), strLine, arrParaments, i);
							break;
						}
					}

					int nPosition = (int)SetFilePointer(hLogFile, 0, 0, FILE_CURRENT);
					logPos.setScanPos(nPosition, strLine, bActioned);
				}
				
				nLogEnd = GetLogEnd(hLogFile);
				nLogFilePosition = nLogEnd;
				
				CloseHandle(hLogFile);
				hLogFile = INVALID_HANDLE_VALUE;
				
				Sleep(250);
			}
		}
		
		
		bool TailTrigger::run()
		{
			if (isRun())
			{
				return false;
			}
			m_bRun = true;
			
			m_hMoniter = CreateThread(NULL, 0, LogParserProc, static_cast<void*>(this), 
				0, &m_dwThreadId);
			return (NULL != m_hMoniter);
		}
		
		bool TailTrigger::stop()
		{
			m_bRun = false;
			
			return true;
		}
		
		void TailTrigger::unInit()
		{
			m_bRun = false;
			
			if (m_hMoniter)
			{
				DWORD dwTimeout = 3000;
				glog(ZQ::common::Log::L_INFO, "Waiting for trigger thread 0x%04x exit", m_dwThreadId);
				DWORD dwRet = WaitForSingleObject(m_hMoniter, dwTimeout);
				
				if (dwRet == WAIT_TIMEOUT)
				{
					glog(ZQ::common::Log::L_INFO, "Waiting for trigger thread 0x%04x exit timeout in %d ms, terminate thread", m_dwThreadId, dwTimeout);
					TerminateThread(m_hMoniter, 1);
				}

				CloseHandle(m_hMoniter);
				m_hMoniter = NULL;
				glog(ZQ::common::Log::L_INFO, "End of waiting for trigger thread 0x%04x exit", m_dwThreadId);
			}
		}

		bool TailTrigger::isRun()
		{
			return m_bRun;
		}
		
	}
}
