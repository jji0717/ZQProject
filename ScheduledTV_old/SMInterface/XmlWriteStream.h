// XmlWriteStream.h: interface for the CXmlWriteStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLWRITESTREAM_H__5A320A3C_BE35_47D2_9622_9FF65C79A8D9__INCLUDED_)
#define AFX_XMLWRITESTREAM_H__5A320A3C_BE35_47D2_9622_9FF65C79A8D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



/// this class is for holding xml strings
class CXmlWriteStream  
{
public:
	
	CXmlWriteStream(char* sBuf);

	
	virtual ~CXmlWriteStream();

	
	int GetStreamLength();
	
	
	char* GetStreamPointer();

	///write a line to buffer
	///@param sLine			string to write
	///@param bCR			whether to change line, defaut is true	
	void WriteLine(const char* sLine, bool bCR = true);

	///write specific format string
	///@param all		same to sprintf's	
	void Write(const char *fmt, ...);

protected:
private:
	
	char* m_sBuf;
	
	char* m_pPtr;

};

#endif // !defined(AFX_XMLWRITESTREAM_H__5A320A3C_BE35_47D2_9622_9FF65C79A8D9__INCLUDED_)
