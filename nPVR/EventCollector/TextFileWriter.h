// TextFileWriter.h: interface for the TextFileWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTFILEWRITER_H__266384F0_143C_4945_A8F2_DCA6BE0AC7DC__INCLUDED_)
#define AFX_TEXTFILEWRITER_H__266384F0_143C_4945_A8F2_DCA6BE0AC7DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KeyDefine.h"
#include "BaseMessageReceiver.h"

class TextFileWriter : public BaseMessageReceiver  
{
public:
	TextFileWriter(int channelID);
	virtual ~TextFileWriter();

	virtual bool init(InitInfo& initInfo, const char* szSessionName);
	virtual void close();

	virtual void OnMessage(int nMessageID, MessageFields* pMessage);

	virtual void requireFields(std::vector<std::string>& fields);

	static const char* getTypeInfo()
	{
		return KD_KV_RECEIVERTYPE_TEXTFILE;
	}

protected:
	
	HANDLE	_hFile;

	static	const char*	_requiredFields[];
	static  int			_nRequiredField;
};

#endif // !defined(AFX_TEXTFILEWRITER_H__266384F0_143C_4945_A8F2_DCA6BE0AC7DC__INCLUDED_)
