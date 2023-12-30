#include "FtpClient.h"

#define FTPCLILOG if(_pLog != NULL) (*_pLog)
#define FTPCLIENTDEF_TIMEOUT  15//15 second
#define FTPCLIENTMAX_TIMEOUT   75//75 second

FtpClient::FtpClient(std::string& strHostName, int& nPort, std::string& strUserName, std::string& strPassWord, std::string& strLocalIP)
{
	_logonInfo.strHostName = strHostName;
	_logonInfo.nPort = nPort;
	_logonInfo.strUserName = strUserName;
	_logonInfo.strPassWord = strPassWord;
	_strLocalIP = strLocalIP;
	_pLog = NULL;
	_nTimeOut = FTPCLIENTDEF_TIMEOUT;
	memset(_vBuf,0,sizeof(_vBuf));
	_dataType = FTP_ASCII;
}

FtpClient::~FtpClient(void)
{
}

void FtpClient::SetLog(ZQ::common::Log* pLog)
{
	_pLog = pLog;
}

void FtpClient::SetTimeOut(int nTimeOut)
{
	if(nTimeOut <= 0)
	{
		FTPCLILOG(ZQ::common::Log::L_WARNING,"FtpClient::SetTimeOut() the time out '%d' is invalidation");
		return;
	}
	if(nTimeOut > FTPCLIENTMAX_TIMEOUT)
		_nTimeOut = FTPCLIENTMAX_TIMEOUT;

	FTPCLILOG(ZQ::common::Log::L_INFO,"FtpClient::SetTimeOut() set time out '%d'",nTimeOut);
	_nTimeOut = nTimeOut;
}

bool FtpClient::Login()
{
	
	if(IsConnected())
		Logout();

	if(!OpenControlChannel(_logonInfo.strHostName,_logonInfo.nPort))
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::Login() connect host '%s',port '%d' failed",_logonInfo.strHostName.c_str(),_logonInfo.nPort);
		return false;
	}

	std::string strR;
	if(!GetResponse(strR))
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::Login() not get connect response");
		return false;
	}

	Reply rp(strR);
    if(!rp.Code().IsPositiveCompletionReply())
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::Login() response string is '%s'",strR.c_str());
		return false;
	}

	std::string strTemp;
	strTemp = "USER ";
	strTemp += _logonInfo.strUserName + "\r\n";

	if(!SendCommand(strTemp, strR))
         return false;


	rp.Set(strR);
    if(!rp.Code().IsPositiveCompletionReply() && !rp.Code().IsPositiveIntermediateReply())
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::Login() 'USER' is failed, response string is '%s'",strR.c_str());
         return false;
	}

	strTemp = "PASS ";
	strTemp += _logonInfo.strPassWord + "\r\n";

	strR = "";
	if(!SendCommand(strTemp, strR))
         return false;

	rp.Set(strR);
    if(!rp.Code().IsPositiveCompletionReply() && !rp.Code().IsPositiveIntermediateReply())
	{   
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::Login() 'PASS' is failed, response string is '%s'",strR.c_str());
   		return false;
	}

/*
	strTemp = "/";
	if(!ChangeWorkingDirectory(strTemp,strR))
		return false;

	rp.Set(strR);
	if(!rp.Code().IsPositiveCompletionReply() && !rp.Code().IsPositiveIntermediateReply())
		return false;
*/
	FTPCLILOG(ZQ::common::Log::L_INFO,"FtpClient::Login() login server '%s:%d' OK",_logonInfo.strHostName.c_str(),_logonInfo.nPort);
	return true;
}

void FtpClient::Logout()
{
	std::string strR;
	SendCommand("QUIT\r\n", strR);

	CloseControlChannel();

}

bool FtpClient::IsConnected()
{
	if(_ctrlSock.operator SOCKET() != 0)
		return true;
	else
		return false;
}

bool FtpClient::OpenControlChannel(const std::string& strServerHost, int nPort )
{
	CloseControlChannel();

	try
	{
		_ctrlSock.Create(SOCK_STREAM);
		sockaddr_in adr = _ctrlSock.GetHostByName(strServerHost.c_str(),nPort);
		_ctrlSock.Connect((LPCSOCKADDR)&adr);
	}
	catch(BlockingSocketException& blockingException)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR, "FtpClient::OpenControlChannel() failed,error string '%s'",blockingException.GetErrorMessage().c_str());
		_ctrlSock.Cleanup();
		return false;
	}

	return true;
}

void FtpClient::CloseControlChannel()
{
	try
	{
		_ctrlSock.Close();
	}
	catch(BlockingSocketException& blockingException)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::CloseControlChannel() catch exception,error string '%s'",blockingException.GetErrorMessage().c_str());
		_ctrlSock.Cleanup();
	}
}

bool FtpClient::SendCommand(const std::string& strCommand)
{
	if( !IsConnected() )
		return false;

	try
	{
		_ctrlSock.Write(strCommand.c_str(), static_cast<int>(strCommand.size()), _nTimeOut);
	}
	catch(BlockingSocketException& blockingException)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR, "FtpClient::SendCommand() send command %s failed,error string '%s'",strCommand.c_str(),blockingException.GetErrorMessage().c_str());
		const_cast<FtpClient*>(this)->_ctrlSock.Cleanup();
		return false;
	}
	

	FTPCLILOG(ZQ::common::Log::L_DEBUG,"FtpClient::SendCommand() send command '%s' OK",(strCommand.substr(0,strCommand.length()-2)).c_str());
   
	return true;
}

bool FtpClient::SendCommand(const std::string& strCommand, std::string& strResponse)
{
	if( !SendCommand(strCommand) || !GetResponse(strResponse) )
		return false;
	return true;
}

bool FtpClient::GetResponse(std::string& strResponse)
{
//	std::string strResponse;
	if(!GetSingleResponseLine(strResponse))
		return false;


	if( strResponse.length() > 3 && strResponse.at(3) == ('-') )
	{
		std::string strSingleLine(strResponse);
		char* pEnd = NULL;
		const int iRetCode = strtol(strResponse.c_str(),&pEnd,10);
		// handle multi-line server responses
		while( !(strSingleLine.length() > 3 && 
               strSingleLine.at(3)==(' ') &&
               strtol(strSingleLine.c_str(),&pEnd,10) == iRetCode) )
		{ 
			if( !GetSingleResponseLine(strSingleLine) )
				return false;
			strResponse += ("\r\n") + strSingleLine;
		}
	}
	return true;
}

bool FtpClient::GetSingleResponseLine(std::string& strResponse)
{
	if(!IsConnected())
		return false;

	try
	{
		if(_qResponseBuffer.empty())
		{
			// internal buffer is empty ==> get response from ftp-server
			int iNum=0;
			std::string strTemp;

			do
			{
				memset(_vBuf,0,sizeof(_vBuf));
				iNum=_ctrlSock.Receive(_vBuf, static_cast<int>(sizeof(_vBuf))-1, _nTimeOut);
				strTemp += _vBuf;
			}while( iNum==static_cast<int>(sizeof(_vBuf))-1 && _ctrlSock.CheckReadability() );


			// each line in response is a separate entry in the internal buffer
			while( strTemp.length() )
			{
				size_t iCRLF=strTemp.find('\n');
				if( iCRLF != std::string::npos )
				{
					_qResponseBuffer.push(strTemp.substr(0, iCRLF+1));
					strTemp.erase(0, iCRLF+1);
				}
				else
				{
					// this is not rfc standard; normally each command must end with CRLF
					// in this case it doesn't
					_qResponseBuffer.push(strTemp);
					strTemp.clear();
				}
			}

			if( _qResponseBuffer.empty() )
				return false;
		}

		// get first response-line from buffer
		strResponse = _qResponseBuffer.front();
		_qResponseBuffer.pop();

		// remove CrLf if exists
		if( strResponse.length() > 1 && strResponse.substr(strResponse.length()-2)==("\r\n") )
			strResponse.erase(strResponse.length()-2, 2);
	}
	catch(BlockingSocketException& blockingException)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::GetSingleResponseLine() catch a exception,error string '%s'",blockingException.GetErrorMessage().c_str());
		const_cast<FtpClient*>(this)->_ctrlSock.Cleanup();
		return false;
	}

	return true;
}

bool FtpClient::ChangeWorkingDirectory(std::string& strDir, std::string& strResponse)
{
	std::string strTemp = "CWD ";
	strTemp += strDir + "\r\n";
	if(!SendCommand(strTemp, strResponse))
		return false;
	
	return true;
}

bool FtpClient::PrintWorkingDirectory(std::string& strWorkDir)
{
	std::string strTemp = "PWD ";
	strTemp += "\r\n";
	
	std::string strR;
	if(!SendCommand(strTemp, strR))
		return false;
	
	Reply rp(strR);
	if(!rp.Code().IsPositiveCompletionReply())
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::PrintWorkingDirectory() print working director failed, response string '%s'",strR.c_str());
		return false;
	}
	
	size_t sbe = strR.find("\"",3);
	strWorkDir = strR.substr(sbe+1,strR.find("\"",sbe+1)-sbe-1);

	return true;
}

bool FtpClient::System(std::string& strSys)
{
	std::string strTemp = "SYST \r\n";
	std::string strR;
	if(!SendCommand(strTemp, strR))
		return false;
	
	Reply rp(strR);
	if(!rp.Code().IsPositiveCompletionReply())
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::System() get system info failed, response string '%s'",strR.c_str());
		return false;
	}
	strSys = strR.substr(strR.find(" ",1));
	return true;
}

bool FtpClient::OpenActiveDataConnection(BlockingSocket& sckDataConnection, const DatachannelCmd& crDatachannelCmd, const std::string& strPath)
{
	std::auto_ptr<BlockingSocket> apSckServer(_ctrlSock.CreateInstance());

	int nPort = 0;
	try
	{
		// INADDR_ANY = ip address of localhost
		// second parameter "0" means that the WINSOCKAPI ask for a port
		sockaddr_in saddr;
		saddr.sin_family = AF_INET;
		saddr.sin_port = 0;
		saddr.sin_addr.s_addr = inet_addr(_strLocalIP.c_str());

		apSckServer->Create(SOCK_STREAM);
		apSckServer->Bind((LPCSOCKADDR)&saddr);
		apSckServer->GetSockAddr((LPSOCKADDR)&saddr);
		nPort = ntohs(saddr.sin_port);
		apSckServer->Listen();
	}
	catch(BlockingSocketException& blockingException)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::OpenActiveDataConnection() catch a exception,error string '%s'",blockingException.GetErrorMessage().c_str());
	    apSckServer->Cleanup();
		return false;
	}

	// transmit the socket (ip address + port) to the server
	// the ftp server establishes then the data connection
	std::string strR;
	if(!DataPort(_strLocalIP, nPort, strR))
		return false;

	Reply rp(strR);
	if(!rp.Code().IsPositiveCompletionReply())
		return false;


	// send FTP command RETR/STOR/NLST/LIST to the server

	std::string strTemp;
	switch(crDatachannelCmd)
	{
	case FTP_LIST:
		strTemp = "LIST ";
		break;
	case FTP_STOR:
		strTemp = "STOR ";
		break;
	default:
		break;

	}
	if(strTemp.length() < 1)
		return false;

	strTemp += strPath + "\r\n";
	if( !SendCommand(strTemp, strR))
		return false;
   
	rp.Set(strR);
    if(!rp.Code().IsPositivePreliminaryReply() )
		return false;

	// accept the data connection
	sockaddr_in conAddr;
	if(!apSckServer->Accept(sckDataConnection, (LPSOCKADDR)&conAddr))
		return false;
   
	return true;
}

bool FtpClient::DataPort(const std::string& strHostIP, int& nPort, std::string& strR)
{
	std::string strTemp = "PORT " + strHostIP;
	size_t index = 0;
	while((index = strTemp.find(".",index)) != std::string::npos)
		strTemp[index] = ',';

	strTemp += ",";
	char chP[5] = {0};
	sprintf(chP,"%d",nPort/256);
	strTemp += chP;
	strTemp += ",";
	memset(chP,0,sizeof(chP));
	sprintf(chP,"%d",nPort%256);
	strTemp += chP;
	strTemp += "\r\n";

	if(!SendCommand(strTemp, strR))
		return false;

	return true;
}

bool FtpClient::TransferData(const DatachannelCmd& crDatachannelCmd, std::string& strData, BlockingSocket& sckDataConnection)
{
	bool bR = false;
	switch(crDatachannelCmd)
	{
	case FTP_LIST:
		bR = ReceiveData(strData,sckDataConnection);
		break;
	case FTP_STOR:
		bR = SendData(strData,sckDataConnection);
		break;
	default:
		break;
	}
	return bR;
}

bool FtpClient::ReceiveData(std::string& strData, BlockingSocket& sckDataConnection)
{
	try
	{
		memset(_vBuf,0,sizeof(_vBuf));
		int iNumRead = sckDataConnection.Receive(_vBuf, static_cast<int>(sizeof(_vBuf)), _nTimeOut);
		
		if(iNumRead == 0)
			return false;

		long lTotalBytes = iNumRead;
		strData = _vBuf; 
		while(iNumRead != 0)
		{
			memset(_vBuf,0,sizeof(_vBuf));
			iNumRead=sckDataConnection.Receive(_vBuf, static_cast<int>(sizeof(_vBuf)), _nTimeOut);
	         
			lTotalBytes += iNumRead;
			strData += _vBuf;
		 }
	}
	catch(BlockingSocketException& sockEx)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::ReceiveData() catch a exception,error string '%s'",sockEx.GetErrorMessage().c_str());
		sckDataConnection.Cleanup();
		return false;
	}
   return true;
}

bool FtpClient::SendData(std::string& strData, BlockingSocket& sckDataConnection)
{
	int iNumSend = 0;
	try
	{
		iNumSend = sckDataConnection.Write(strData.c_str(),static_cast<int>(strData.size()),_nTimeOut);
	}
	catch(BlockingSocketException& sockEx)
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::SendData() catch a exception,error string '%s'",sockEx.GetErrorMessage().c_str());
		sckDataConnection.Cleanup();
		return false;
	}

	if(iNumSend != strData.size())
		return false;
   
	return true;
}

bool FtpClient::ExecuteDatachannelCommand(const std::string& strPath, std::string& strData, const DatachannelCmd& crDatachannelCmd, const DataType& dataType)
{

	if(!IsConnected() )
		return false;

	// check representation
	if(dataType != _dataType)
	{
		// transmit representation to server
		if(!RepresentationType(dataType))
			return false;
		_dataType = dataType;
	}

	std::auto_ptr<BlockingSocket> apSckDataConnection(_ctrlSock.CreateInstance());

	if(!OpenActiveDataConnection(*apSckDataConnection, crDatachannelCmd, strPath))
		return false;


	bool fTransferOK = TransferData(crDatachannelCmd, strData, *apSckDataConnection);
   
	apSckDataConnection->Close();

	// get response from ftp server
	if(!fTransferOK)
		return false;

	std::string strR;
	if(! GetResponse(strR))
		return false;
	Reply rp(strR);
	if(!rp.Code().IsPositiveCompletionReply())
		return false;


	return true;
}

bool FtpClient::RepresentationType(const DataType& dataType)
{
	std::string strCmd;
	switch(dataType)
	{
	case FTP_ASCII:
		strCmd = ("TYPE A");
		break;
	case FTP_IMAGE:     
		strCmd = ("TYPE I");
		break;
	default:
		return false;
	}
   
	strCmd += "\r\n";

	std::string strR;
	if(!SendCommand(strCmd, strR))
		return false;
   ;
	Reply rp(strR);
	
	if(!rp.Code().IsPositiveCompletionReply())
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::RepresentationType() 'TYPE' response string is '%s'",strR.c_str());
		return false;
	}
	return true;
}

bool FtpClient::Delete(const std::string& strFile,std::string& strR)
{
	if(strFile.empty())
		return false;
	std::string strTemp = "DELE " + strFile + "\r\n";

	if(!SendCommand(strTemp, strR))
		return false;

	Reply rp(strR);
	if(!rp.Code().IsPositiveCompletionReply())
	{
		FTPCLILOG(ZQ::common::Log::L_ERROR,"FtpClient::Delete() delete '%s' response is '%s'",strFile.c_str(),strR.c_str());
		return false;
	}
	
	FTPCLILOG(ZQ::common::Log::L_INFO,"FtpClient::Delete() delete '%s' OK",strFile.c_str());
	return true;
}


