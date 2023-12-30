// File Name: FTPMSClient.cpp
// Description:
// Date: 2008-12

#include <string>
#include <vector>
#include <fstream>
#include "Log.h"
#include "FTPMSClient.h"
#include "SystemUtils.h"
#include "SelectPort.h"
#define MOLOG (*_log)
#define FTPClientI "FTPClientI"

using namespace ZQ::common;
using namespace SocketAPI;

namespace ZQTianShan
{
	namespace ContentProvision
	{

FTPMSClient::FTPMSClient()
	:_nFTPReturnCode(0), _lpMsgBuf(NULL), _log(NULL), _dwError(0), 
	_transmitMode(binary), 
	_strLocalAddress(""), _strLastError(""),
	_bConnect(false), _bLogOn(false), _bOpenFile(false), _dwTimeoutInMs(4000)
{
	_bReadDone = false;
}

FTPMSClient::~FTPMSClient()
{
	closeFile();
	close();
}

void FTPMSClient::close()
{
	if (_bLogOn) // log off from ftp server
	{
		sendCommand(221, "QUIT");
		_bLogOn = false;
		MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to log off from ftp server"));
	}
	if (_bConnect) // disable control connection
	{
		_controlConnection.closeSocket();
		_bConnect = false;
		MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to close control connection"));
	}
}

bool FTPMSClient::open(const std::string& strServer, int nPort, 
	const std::string& strUserName, const std::string& strPasswd, 
	const std::string& strLocalNetworkInterface)
{
	close(); // close if connect to some FTP server
	saveUserInfo(strServer, nPort, strUserName, strPasswd,strLocalNetworkInterface);		//save the user's info
	if (!establishCtrlConnection(strLocalNetworkInterface, strServer, nPort))
	{
		close();
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to connect server.Address:[%s], port:[%d], error code[%d]"), strServer.c_str(), nPort, _dwError);
		return false;
	}
	_bConnect = true;
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to connect server. Address:[%s], port:[%d]"), strServer.c_str(), nPort);

	if (!logOn(strUserName, strPasswd))
	{
		close();
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to log on server. user:[%s], password:[%s]"), strUserName.c_str(), strPasswd.c_str());
		return false;
	}
	_bLogOn = true;
	_strLocalAddress = strLocalNetworkInterface; // save bind address
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to log on ftp server.user:[%s], password:[%s]"), strUserName.c_str(), strPasswd.c_str());
	return true;
}

bool FTPMSClient::establishCtrlConnection(const std::string& strLocalAddress, const std::string& strServer, const short nPort)
{

	if (strServer.empty()) // server address is empty
	{
		_strLastError = "Server address is empty.";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Server address is empty"));
		return false;
	}
	if (!_controlConnection.createSocket(_dwError, strLocalAddress)) // 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to create control connection. Error code [%d]"), _dwError);
		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to bind client socket with [%s]."), strLocalAddress.c_str());
	if (!_controlConnection.connectServer(strServer, nPort, _dwError))
	{
		_controlConnection.closeSocket();
		return false;
	}
	if (!getResponse()) // can't get response from server
	{
		_controlConnection.closeSocket();
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Failed to read response after connecting to ftp server."));
		return false;
	}
	if (_nFTPReturnCode != 220)
	{
		_controlConnection.closeSocket();
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "FTP server isn't ready to service "));
		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to connect server. Address:[%s], port:[%d]."), strServer.c_str(), nPort);
	return true;
}

bool FTPMSClient::logOn(const std::string& strUserName, const std::string& strPasswd)
{
	if (strUserName.empty() || strPasswd.empty())
	{
		_strLastError = "User name or password is empty";
		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Start to log on ftp server. user:[%s], password:[%s]"), strUserName.c_str(), strPasswd.c_str());
	if (!sendCommand(331, "USER", strUserName)) // 331 password is required
	{
		_strLastError = "User name is error";
		return false;
	}
	if (!sendCommand(230, "PASS", strPasswd)) // 230 success log on 
	{
		_strLastError = "Password is error";
		return false;
	}
	return true;
}

//@Function	  send command to ftp server, then get response from ftp server
//@Param      int nRequireCode, FTP return code for this command if success
//@Param	  const char* strCmdName, command name
//@Param	  const std::string strCmdOpt, command option
//@Return	  return true if success, else return false
//@Remark   
bool FTPMSClient::sendCommand(int nRequireCode, const char* strCmdName, const std::string strCmdOpt)
{
	char strBuffer[320];
	if (strCmdOpt.empty())
		sprintf(strBuffer, "%s\r\n", strCmdName);
	else
		sprintf(strBuffer, "%s %s\r\n", strCmdName, strCmdOpt.c_str());
	std::string strSend = ridOfEnterFlag(strBuffer);
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI,"send : (%s)"), strSend.c_str()); 
	if (!sendRequest(strBuffer, static_cast<int>(strlen(strBuffer))))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Failed to send  %s cmd when loging on to ftp server. Error code[%d]"), strSend.c_str(), _dwError);
		return false;
	}
	if (!getResponse())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI,"Failed to read %s response after connecting to ftp server. Error code [%d]"), strBuffer, _dwError);	
		return false;
	}
	//FileZilla ftp server ( While QUIT(221), will get code 425 before, so shoubld be getResponse again )
	if(nRequireCode == 221 && _nFTPReturnCode == 425)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPClientI,"While QUIT(221), get code 425 before"));	
		if (!getResponse())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI,"While QUIT(221), get code 425 before ,then Failed to read %s response after connecting to ftp server. Error code [%d]"), strBuffer, _dwError);	
			return false;
		}
	}
	if(nRequireCode  && (_nFTPReturnCode != nRequireCode) && (_nFTPReturnCode != 200) && (_nFTPReturnCode != 125) )
	{
		MOLOG(Log::L_WARNING, CLOGFMT(FTPDownload,"Return code is %d , not %d when get %s cmd response from ftp server."), _nFTPReturnCode, nRequireCode, strBuffer); 
		return false;
	}
	return true;
}

bool FTPMSClient::sendRequest(char* pBuffer, int nLen)
{
	int nReturn = _controlConnection.completeSend(pBuffer, nLen, 500, _dwError);
	if ( nReturn != 0)
	{
		return false;
	}
	return true;
}

bool FTPMSClient::getResponse()
{
	int nCode = 0;
	int nLenRead = 0;
	if (!getLine(_buffer, 1024, nCode, nLenRead))
	{
		return false;
	}
	//if it is single reply.  
	_nFTPReturnCode = nCode;
	if((strlen(_buffer) < 6) || (*(_buffer+3) != '-') )
	{
		return true;
	}
	//if it is multiple replies.
	int nmsg = atoi( _buffer);
	while(1)
	{
		if (!getLine(_buffer, 1024, nCode, nLenRead))
		{
			return false;
		}
		if((strlen(_buffer) >= 6) && (*(_buffer+3)==' ') && (atoi(_buffer) == nmsg))
		{
			return true;
		}
	} // end while 
}

bool FTPMSClient::getLine(char* pBuffer, int nLen, int& nCode, int& nLenRead)
{
	int nPos = 0;
	int nRetRead = 0;
	nCode = 0;
	nLenRead = 0;
	unsigned long dwstart = SYS::getTickCount();
	while (1)
	{
		nRetRead = _controlConnection.receiveDatas(pBuffer + nPos, 1, _dwTimeoutInMs, _dwError);
		if (_dwError)
		{
			MOLOG(Log::L_INFO, CLOGFMT(FTPDownload,"error code:%d"), _dwError); 
		}
		
		switch (nRetRead)
		{
		case 1: // read one byte success
			nPos++;
			nLenRead++;
			if( *(pBuffer + nPos - 1) == '\n' || nPos >= 1024 )
			{
				if ( *(pBuffer + nPos - 1) == '\n')
				{
					*(pBuffer + nPos -1) = '\0';
				}
				else
				{
					*(pBuffer + nPos) = '\0';
				}
				std::string strResponse(pBuffer);
				size_t nposition = strResponse.find_first_of("123456789");
				nCode = atoi(pBuffer + nposition);
				MOLOG(Log::L_INFO, CLOGFMT(FTPClientI,"receive : (%s)"), pBuffer); 
				return true;
			}
			break;
		case -1: // we regard it is Socket error
		case -2: // peer socket has gracefully closed
			*(pBuffer + nPos) = '\0';
			MOLOG(Log::L_WARNING, CLOGFMT(FTPClientI,"receiveDatas return %d, read bytes %d"), nRetRead, nPos); 
			return false;
		case -3:
			if( SYS::getTickCount()-dwstart > _dwTimeoutInMs || SYS::getTickCount() < dwstart )
			{
				*(pBuffer + nPos) = '\0';
				MOLOG(Log::L_WARNING, CLOGFMT(FTPClientI,"receiveDatas timeout, read bytes %d"), nPos); 
				return false;
			}
			break;
		default: // time out
			//continue;
			*(pBuffer + nPos) = '\0';
			MOLOG(Log::L_WARNING, CLOGFMT(FTPClientI,"receiveDatas return %d, read bytes %d"), nRetRead, nPos); 
			return false;
		} // end for switch
	} // end for while
}

int64 FTPMSClient::getFileSize(const std::string& strFileName)
{
	if (!setTransmitModeToServer()) // set data transmit format
	{
		MOLOG(Log::L_WARNING, CLOGFMT(FTPClientI,"get[%s]'s size Fail to set transmit mode"), strFileName.c_str());
	}
	unsigned int timeOut = _dwTimeoutInMs;
	setIoTimeout(2);
	getResponse();
	setIoTimeout(timeOut);
	std::string filename = IntoDirectory(strFileName);
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Start to get [%s]'s size."), filename.c_str());
	if (!sendCommand(213, "SIZE", filename)) // 213 response file status
	{
		_strLastError = "Cann't get file size from ftp server.";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to get [%s]'s size. Error code[%d]"), filename.c_str(),_dwError);
		return -1;
	}
	int64 nSize = 0;
	if (strlen(_buffer) > 4)  // server side error 
	{
#ifdef ZQ_OS_MSWIN
		nSize = _atoi64(_buffer + 4);
#else
		nSize = atoll(_buffer + 4);
#endif
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "[%s] is size [%lld] bytes."), strFileName.c_str(), nSize);
	return nSize;
}

bool FTPMSClient::openFile(const std::string& strFileName, enum FileMode fileMode, int64 llInitOffset)
{
	closeFile(); // close old file before open new file 
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "opening file [%s] with offset [%lld]..."), strFileName.c_str(), llInitOffset);
	_strFileName = strFileName;
	_nFileOffset = llInitOffset;

	std::string filename = strFileName;
	char spec = '\\';
#ifdef ZQ_OS_LINUX
	spec = '/';
#endif

	int npos = strFileName.rfind(spec);
	if(npos > 0)
	{
		filename = strFileName.substr(npos +1);
	}

	if (0 == filename.size())
	{
		_strLastError = "File name is empty";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "File name is empty"));
		return false;
	}
	if (!setTransmitModeToServer()) // set data transmit format
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI,"Fail to set transmit mode"));
		return false;
	}
	if (active == _ftpMode) // set data connection establish mode
	{
		bool bRet = setActiveMode(filename, fileMode);
		if (!bRet)
		{
			_strLastError = "";
			MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Active -> Passive ,close all connect,then open again...")); 
			_dataConnection.shutdownSocket();
			_dataConnection.closeSocket();
			if (!open(_strServer, _nPort, _strUserName, _strPassword, _strLocalNetworkInterface)) // open fail
			{
				std::string strErr = "active -> Passive,failed to connect to " + _strServer + ", return with error: " + this->getLastError();
				MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "failed to connnet to %s\n"), strErr.c_str());
			}
			bRet = setPassiveMode(filename, fileMode);
		}
		return bRet;
	}
	else
	{
		bool bRet = setPassiveMode(filename, fileMode);
		if (!bRet)
		{
			_strLastError = "";
			MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Passive -> Active  ,close all connect,then open again...")); 
			_dataConnection.shutdownSocket();
			_dataConnection.closeSocket();
			if (!open(_strServer, _nPort, _strUserName, _strPassword, _strLocalNetworkInterface)) // open fail
			{
				std::string strErr = "Passive -> active ,failed to connect to " + _strServer + ", return with error: " + this->getLastError();
				MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "failed to connnet to %s\n"), strErr.c_str());
			}
			bRet = setActiveMode(filename, fileMode);
		}
		return bRet;
	}
}

bool FTPMSClient::readFile(char* buffer, int nSize, int &nRead)
{
	nRead = _dataConnection.receiveDatas(buffer, nSize, _dwTimeoutInMs, _dwError);
	if (_dwError != 0)
	{
		_bReadDone = true;
		return false;
	}
	if(nRead <= 0)
		_bReadDone = true;
	return true;
}

void FTPMSClient::closeFile()
{
	if (_bOpenFile)
	{
		if (!_bReadDone)
			sendCommand(0, "ABOR");

		MOLOG(Log::L_DEBUG, CLOGFMT(FTPDownload, "[%s] closing the data socket"), _strFileName.c_str());
		_dataConnection.shutdownSocket();
		_dataConnection.closeSocket();
		_serverSocket.closeSocket();
		getResponse(); // 226 transfer complete or 426 connection closed, transfer aborted
		_bOpenFile = false;
		MOLOG(Log::L_INFO, CLOGFMT(FTPDownload, "success to close remote file [%s]"), _strFileName.c_str());
	}
}

std::string FTPMSClient::getLastError()
{
	if (_dwError != 0) // prior to use this to get error message
	{
#ifdef ZQ_OS_MSWIN
		SocketInterface::getLastErrorDes(&_lpMsgBuf, _dwError);
		_strLastError = _lpMsgBuf;
		SocketInterface::releaseMessage(_lpMsgBuf);
#else
		_strLastError = SocketInterface::getLastErrorDes(_dwError);
#endif
	}
	return _strLastError;
}

bool FTPMSClient::setTransmitModeToServer()
{
	char strBuffer[1025];
	switch(_transmitMode)
	{
	case ascii:
		strcpy(strBuffer, "TYPE A");
		break;
	case binary:
		strcpy(strBuffer, "TYPE I");
		break;
	case EBCDIC:
		strcpy(strBuffer, "TYPE E");
		break;
	default:
		break;
	}
	if (!sendCommand(200, strBuffer)) // 200 execute command success
	{
		_strLastError = "Fail to set transmit mode";
		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to set transmit mode")); 
	return true;
}
bool FTPMSClient::setTransmitMode(enum TransmitMode mode)
{
	_transmitMode = mode;
	return true;
}

bool FTPMSClient::setFTPMode(enum FTPMode ftpMode)
{
	_ftpMode = ftpMode;
	return true;
}

bool FTPMSClient::setActiveMode(const std::string& strFileName, enum FileMode fileMode)
{
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Start to open file in active mode............")); 

	SelectPort aa;
	if (aa.getQueueSize())
	{
		for (int i = 1; i<= aa.getQueueSize();i++)
		{
			int pp = aa.getPort();
			if (!_serverSocket.createSocket(_dwError, _strLocalAddress,pp))
			{
				if (i == aa.getQueueSize())
				{
					MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to create client's server socket for data connection,try[%d] times,lastPort[%d], Error code [%d}"),i,pp, _dwError);
					return false;
				}
			}
			else
			{
				MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "create client's server socket for data connection,port[%d]try[%d]times"),pp,i);
				break;
			}
			
		}
	}
	else
	{
		if (!_serverSocket.createSocket(_dwError, _strLocalAddress))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to create client's server socket for data connection. Error code [%d}"), _dwError);
			return false;
		}
	}
	if (!_serverSocket.listenRequest(5, _dwError))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Client's server socket fail to listen for data connection. Error code [%d}"), _dwError);
		_serverSocket.shutdownSocket();
		_serverSocket.closeSocket();
		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Client's Server socket is listening for data connection")); 
	if (!sendPortCmd())
	{
		_serverSocket.shutdownSocket();
		_serverSocket.closeSocket();
		return false;
	}

	if (fileMode == toRead && _nFileOffset)
	{
		if (!sendDownloadOffset())
			return false;
	}

	const char* opMode = (toRead == fileMode) ? "RETR" : "STOR";
	if (!sendCommand(150, opMode, strFileName)) // 150 open connection
	{
		_strLastError = "Fail to set operate file mode";
		_serverSocket.shutdownSocket();
		_serverSocket.closeSocket();
		MOLOG(Log::L_INFO, CLOGFMT(FTPClientI,"Fail to set operate file mode, close data connection"));
		return false;
	}
	if (!_serverSocket.acceptSocket(_dataConnection, _dwError))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI,"Fail to accept data connection from ftp server. Error code :[%d]"), _dwError);
		_dataConnection.shutdownSocket();
		_dataConnection.closeSocket();
		_serverSocket.shutdownSocket();
		_serverSocket.closeSocket();
		MOLOG(Log::L_INFO, CLOGFMT(FTPClientI,"setActiveMode() close server Socket and data connection"));
		return false;
	}
	_bOpenFile = true;
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to open file in active mode")); 
	return true;
}

bool FTPMSClient::sendPortCmd()
{
	char strAddress[32];
	int  nPort = 0;
	uint8 a,b,c,d;
	if (!_controlConnection.getSocketName(strAddress, nPort, _dwError))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to get control connection's ip and port. Error code [%d}"), _dwError);
		return false;
	}
	if( !SocketInterface::IP2B4(strAddress, &a, &b, &c, &d) )
	{
		_strLastError =  "Fail to change ip address to 4 BYTES";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to change ip address to 4 BYTES"));
		return false;
	}
	if (!_serverSocket.getSocketName(strAddress, nPort, _dwError))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to get server socket's ip and port. Error code [%d}"), _dwError);
		return false;
	}
	sprintf(_buffer, "PORT %d,%d,%d,%d,%d,%d", a, b, c, d, nPort/256, nPort%256 );
	if (!sendCommand(200, _buffer))
	{
		_strLastError = "Fail to set PORT command";
		return false;
	}
	return true;
}

bool FTPMSClient::setPassiveMode(const std::string& strFileName, enum FileMode fileMode)
{
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Start to open file in passive mode.............")); 
	if (!sendCommand(227, "PASV"))
	{
		_strLastError = "Fail to set passive mode.";
		return false;
	}

	std::string strServer;
	short nDataPort = 0;
	if (!getServerAddress(strServer, nDataPort))
	{
		_strLastError = "Fail to get server address and port for data connenction.";
		return false;
	}

	if (fileMode == toRead && _nFileOffset)
	{
		if (!sendDownloadOffset())
			return false;
	}
	if (!_dataConnection.createSocket(_dwError, _strLocalAddress))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to create data connection. Error code [%d}"), _dwError);
		return false;
	}
	if (!_dataConnection.connectServer(strServer, nDataPort, _dwError))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to establish data connection with server. Error code [%d],close data connection"), _dwError);
		_dataConnection.closeSocket();
		return false;
	}
	const char* opMode = (toRead == fileMode) ? "RETR" : "STOR";
	if (!sendCommand(150, opMode, strFileName))
	{
		return false;
	}
	_bOpenFile = true;
	MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Success to open file in passive mode")); 
	return true;
}
 
bool FTPMSClient::getServerAddress(std::string& strServer, short& nDataPort)
{
	std::string strBuffer(_buffer + 4);
	size_t nBegin = strBuffer.find_first_of("123456789");
	if (std::string::npos == nBegin)
	{
		return false;
	}
	size_t nEnd = strBuffer.find_last_of("0123456789");
	if (std::string::npos == nEnd)
	{
		return false;
	}
	std::string strAddressAndPort = strBuffer.substr(nBegin, nEnd - nBegin + 1);
	std::vector<std::string> result;
	splitString(result, strAddressAndPort, ",");

	if(result.size() < 6)
		return false;

	strServer = result[0] + "." +result[1] + "." + result[2] + "." + result[3];
    nDataPort = (short)(atoi(result[4].c_str()) * 256 + atoi(result[5].c_str()));
	return true;
}
  
void FTPMSClient::splitString(std::vector<std :: string> &result, const std::string &str, const std::string &delimiter)
{
    using namespace std;
    result.clear();
    string::size_type pos_from = 0;
    while((pos_from = str.find_first_not_of(delimiter, pos_from)) != string::npos)
    {
        string::size_type pos_to = str.find_first_of(delimiter, pos_from);
        if(pos_to != string::npos)
        {
            result.push_back(str.substr(pos_from, pos_to - pos_from));
        }
        else
        {
            result.push_back(str.substr(pos_from));
            break;
        }
        pos_from = pos_to;
    }
}

void FTPMSClient::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

void FTPMSClient::setIoTimeout( int nMiliSeconds /*= 4000*/ )
{
	_dwTimeoutInMs = nMiliSeconds;
}

bool FTPMSClient::downloadFile( const std::string& strRemoteFile, const std::string& strLocalFile )
{
	if (strRemoteFile.empty() || strLocalFile.empty())
	{
		_strLastError = "Remote file name or Local file name is empty";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "File name is empty.Remote File[%s],Local File[%s]"), strRemoteFile.c_str(), strLocalFile.c_str());
		return false;
	}
	std::ofstream out;
	out.open(strLocalFile.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
	if (!out)
	{
		_strLastError = "Fail to open local file";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to open local file %s"), strLocalFile.c_str());
		return false;
	}
	if (!openFile(strRemoteFile))
	{
		_strLastError = "Fail to open remote file";
		out.close();
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Fail to open remote file %s"), strRemoteFile.c_str());
		return false;
	}
	char buffer[10241];
	int nRead = 0;
	int64 nAlreadyRead = 0;
	while (readFile(buffer, 10240, nRead) && (!_bReadDone))
	{
		nAlreadyRead += nRead;
		out.write(buffer, nRead);
	}
	nAlreadyRead += nRead;
	out.close();

	closeFile(); // close remote file

	if (nAlreadyRead != getFileSize(strRemoteFile))
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPClientI, "Current read bytes:"FMT64""), nAlreadyRead);
		_strLastError = "Fail to get complete file";
		MOLOG(Log::L_ERROR, CLOGFMT(FTPClientI, "Download file fail.Error reason %s"), getLastError().c_str());
		return false;
	}
	return true;
}

bool FTPMSClient::sendDownloadOffset()
{
	if (!_nFileOffset)
		return true;

	char szNum[64];
	sprintf(szNum, FMT64, _nFileOffset);

	if (!sendCommand(350, "REST", szNum)) // 350 success log on 
	{
		_strLastError = "failed to sendDownloadOffset() on file " + _strFileName + " with offset " + szNum;
		return false;
	}

	return true;
}

std::string FTPMSClient::ridOfEnterFlag(const char* strBuf)
{
	char line[2048];
	memset(line,0,sizeof(line));
	memcpy(line,strBuf,strlen(strBuf));

	char *ptr;

	if ((ptr = strchr(line,'\r')) != NULL)    //if '\r' is found set it to '\0'
		*ptr = '\0';
	if ((ptr = strchr(line,'\n')) != NULL)    //if '\n' is found set it to '\0'
		*ptr = '\0';
	return std::string(line);
}
void FTPMSClient::saveUserInfo(const std::string& strServer, int& nPort,const std::string& strUserName, const std::string& strPasswd,const std::string& strLocalNetworkInterface)
{
	_strServer = strServer;
	_nPort = nPort;
	_strUserName = strUserName;
	_strPassword = strPasswd;
	_strLocalNetworkInterface = strLocalNetworkInterface;
}
std::string FTPMSClient::IntoDirectory(const std::string strFilePath)
{
    char spec = '\\';
#ifdef ZQ_OS_LINUX
	 spec = '/';
#endif
	std::string strDir, strFileName;
	int npos = strFilePath.rfind(spec);
	if(npos  <= 0)
	{
		strFileName = strFilePath;
	}
	else
	{
		strDir = strFilePath.substr(0, npos  + 1);
		sendCommand(250, "CWD", strDir);
		strFileName = strFilePath.substr(npos +1);
	}

/*(	std::vector<std::string>strDirectory;
	bool bRet = ZQ::common::stringHelper::SplitString(strFileName ,strDirectory, "/\\");	
	if(!bRet || strDirectory.size() <= 1)
	{
		return strFileName;
	}
    for(uint i = 0; i < strDirectory.size() - 1; i++)
	{
		sendCommand(250, "CWD", strDirectory[i]);
	}
	return  strDirectory[strDirectory.size() -1];
*/
	return strFileName;
}
} // end for ContentProvision
} // end for ZQTianShan


