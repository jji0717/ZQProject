// XmlWriteStream.cpp: implementation of the CXmlWriteStream class.
//
//////////////////////////////////////////////////////////////////////


#include "XmlWriteStream.h"
#include "../STVMainHeaders.h"
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CXmlWriteStream::~CXmlWriteStream()
{

}


CXmlWriteStream::CXmlWriteStream(char* sBuf):m_sBuf(sBuf)
{
	m_pPtr = sBuf;
}


int CXmlWriteStream::GetStreamLength()
{
	return m_pPtr - m_sBuf;
}


char* CXmlWriteStream::GetStreamPointer()
{
	return m_sBuf;
}

void CXmlWriteStream::WriteLine(const char* sLine, bool bCR)
{
	while(*sLine)
		*m_pPtr++ = *sLine++;

	if (bCR)
	{
		*m_pPtr++ = '\r';
		*m_pPtr++ = '\n';
	}		
}

void CXmlWriteStream::Write(const char *fmt, ...)
{
	char msg[MAX_SEND_XML_BUF];
	memset(msg, 0, MAX_SEND_XML_BUF);

	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);
	
	char* p = msg;

	while(*p)
		*m_pPtr++ = *p++;
}

