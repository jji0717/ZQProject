#pragma once
#include "BlockingSocket.h"
#include "Log.h"
#include <string>
#include <queue>

#define FTPRECV_BUFFERSIZE 2048//2k byte

typedef struct LoginInfo
{
	std::string strHostName;
	int         nPort;
	std::string strUserName;
	std::string strPassWord;

}LOGONINFO;

class FtpClient
{
public:
	enum DatachannelCmd
	{
		FTP_LIST,
		FTP_STOR
	};
	enum DataType
	{
		FTP_ASCII,
		FTP_IMAGE
	};

	FtpClient(std::string& strHostName, int& nPort, std::string& strUserName, std::string& strPassWord, std::string& strLocalIP);
	~FtpClient(void);

	void SetLog(ZQ::common::Log* pLog);
	void SetTimeOut(int nSecond);
	bool Login();
	void Logout(void);

	bool Delete(const std::string& strFile, std::string& strR);

	bool SendCommand(const std::string& strCommand);
	bool SendCommand(const std::string& strCommand, std::string& strResponse);
	bool GetResponse(std::string& strResponse);

	bool GetSingleResponseLine(std::string& strResponse);
	bool ChangeWorkingDirectory(std::string& strDir, std::string& strResponse);
	bool PrintWorkingDirectory(std::string& strWorkDir);
	bool System(std::string& strR);
	
	bool ExecuteDatachannelCommand(const std::string& strPath, std::string& strData, const DatachannelCmd& crDatachannelCmd, const DataType& dataType = FTP_ASCII);
	bool OpenActiveDataConnection(BlockingSocket& sckDataConnection, const DatachannelCmd& crDatachannelCmd, const std::string& strPath);
	
	bool DataPort(const std::string& strHostIP, int& nPort, std::string& strR);
	bool RepresentationType(const DataType& dataType);

	bool TransferData(const DatachannelCmd& crDatachannelCmd, std::string& strData, BlockingSocket& sckDataConnection);
	bool ReceiveData(std::string& strData, BlockingSocket& sckDataConnection);
	bool SendData(std::string& strData, BlockingSocket& sckDataConnection);

	bool IsConnected(void);
	bool OpenControlChannel(const std::string& strServerHost, int nPort = 21);
	void CloseControlChannel(void);
private:
	BlockingSocket	_ctrlSock;
	LOGONINFO		_logonInfo;
	ZQ::common::Log*	_pLog;
	int				_nTimeOut;
	std::string		_strLocalIP;

	mutable std::queue<std::string> _qResponseBuffer;//buffer for server-responses
	char	_vBuf[FTPRECV_BUFFERSIZE];
	DataType _dataType;
	
};

class Reply
{
private:
	std::string _strResponse;

	/// Holds the reply code.
	class Code
	{		
	public:
		Code()
		{
			memset(_szCode,0,sizeof(_szCode));
		}
		std::string Value() const { return _szCode; }
		bool Set(const std::string& strCode)
		{
			if( strCode.length()!=3 ||
				strCode[0]<('1') || strCode[0]>('5') ||
				strCode[1]<('0') || strCode[1]>('5') )
			{
				memset(_szCode,0,sizeof(_szCode));
				return false;
			}
			strcpy(_szCode,strCode.c_str());
			return true;
		}

		bool IsPositiveReply() const { return IsPositivePreliminaryReply() || IsPositiveCompletionReply() || IsPositiveIntermediateReply(); }
		bool IsNegativeReply() const { return IsTransientNegativeCompletionReply() || IsPermanentNegativeCompletionReply(); }

		bool IsPositivePreliminaryReply() const         { return _szCode[0] == ('1'); }
		bool IsPositiveCompletionReply() const          { return _szCode[0] == ('2'); }
		bool IsPositiveIntermediateReply() const        { return _szCode[0] == ('3'); }
		bool IsTransientNegativeCompletionReply() const { return _szCode[0] == ('4'); }
		bool IsPermanentNegativeCompletionReply() const { return _szCode[0] == ('5'); }

		bool IsRefferingToSyntax() const                      { return _szCode[1] == ('0'); }
		bool IsRefferingToInformation() const                 { return _szCode[1] == ('1'); }
		bool IsRefferingToConnections() const                 { return _szCode[1] == ('2'); }
		bool IsRefferingToAuthenticationAndAccounting() const { return _szCode[1] == ('3'); }
		bool IsRefferingToUnspecified() const                 { return _szCode[1] == ('4'); }
		bool IsRefferingToFileSystem() const                  { return _szCode[1] == ('5'); }
	
	private:
		char _szCode[4];
	} _Code;
public:
	Reply(std::string& strResponse)
	{
		if(strResponse.length() < 3)
			return;
		_strResponse = strResponse;
		_Code.Set(strResponse.substr(0,3));
	}
	~Reply(){}
	bool Set(const std::string& strResponse)
	{
		_strResponse = strResponse;
		if( _strResponse.length()>2 )
			return _Code.Set(_strResponse.substr(0, 3));
		return false;
	}
	const std::string Value() const { return _strResponse; }
	const Code& Code() const { return _Code; }
};
