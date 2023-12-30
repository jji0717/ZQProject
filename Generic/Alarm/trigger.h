
#ifndef _ZQ_TRIGGER_H_
#define _ZQ_TRIGGER_H_
	
#include "nativethread.h"
#include "alarmcommon.h"
#include "syntaxparser.h"
#pragma warning(disable:4786)
#include <string>
	
TG_BEGIN
	
	class Trigger : public ZQ::common::NativeThread
	{
	private:
	protected:
		size_t	m_zTailTime;
		size_t	m_zLogLineLength;
		size_t	m_zSourceId;
		bool	m_bQuit;

		std::string		m_strLogFile;
		SyntaxParser&	m_parser;
	
		int run();
		virtual bool OnTail(char* logline, size_t length) = 0;
		virtual bool OnAction(size_t sourceid, const char* logline);
		
	public:
		Trigger(const char* logfile, SyntaxParser& parser, size_t sourceid,
			size_t zLogLineLength = DEF_LOG_LINE_LENGTH, size_t zTailTime = DEF_LOG_TAIL_TIME);

		bool quit();
	};
	
TG_END
	
#endif//_ZQ_TRIGGER_H_
	