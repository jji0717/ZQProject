// File Name: FTPMSClient.h
// Description:
// Date: 2008-12

#ifndef __FTPMSCLIENT_H__
#define __FTPMSCLIENT_H__

#include "FtpClient.h"
#include "SocketAPI.h"
#include <vector>

namespace ZQTianShan
{
	namespace ContentProvision
	{
		class FTPMSClient : public FTPClient
		{
		public:
			FTPMSClient();
			~FTPMSClient();

		public: // derive from FTPInterface
			//@Function	 connect and log on ftp server
			//@Param     std::string strServer: ftp server address
			//@Param	 short nPort: ftp server listen port
			//@Param     std::string strUserName: user name to log on server
			//@Param	 std::string strPasswd: user password to log on server
			//@Param     std::string strLocalNetworkInterface: which local ip address to use for connection
			//@Return    return true if success, else return false
			//@Remark    if open() success, close() must be call to release resource
			virtual bool open(const std::string& strServer,  
				int nPort = 21,
				const std::string& strUserName = "anonymous", 
				const std::string& strPasswd = "xx@126.com",
				const std::string& strLocalNetworkInterface = "");

			//@Function	 log off and disconnect ftp server
			//@Return	 void 
			//@Remark    if open() success, close() must be call to release resource
			virtual void close();

			/// download the specified remote ftp server file to local disk file
			///@param[in] strRemoteFile		the file path name on the ftp server
			///@param[in] strLocalFile		the file path name on local disk
			///@return	true for success, false for failure, call getLastError for the error information if return false
			virtual bool downloadFile(const std::string& strRemoteFile, const std::string& strLocalFile);

			//@Function  get remote file size
			//@Param	 std::string strFileName: remote file name
			//@Return	 if success return file size, else return -1
			//@Remark
			virtual int64 getFileSize(const std::string& strFileName);
       
			//@Function	 establish data connection for transmit file
			//@Param	 std::string strFileName: file name, to read or write
			//@Param	 FileMode fileMode: how to operate file , read or write
			//@Param	 llInitOffset, the initialize file offset when open file, only valid in fileMode "toRead"
			//@Return	 return true if success, else return false
			//@Remark    if openFile() success, closeFile() must be call
			virtual bool openFile(const std::string& strFileName, enum FileMode fileMode = toRead, int64 llInitOffset = 0);

			//@Function	 read file sequence
			//@Param	 char* buffer: buffer to receive datas
			//@Param	 int nSize: byte number to read
			//@Param     int& nRead: success to read byte number
			//@Return	 return true if success, else return false
			//@Remark    openFile() must be called before call this method
			virtual bool readFile(char* buffer, int nSize, int& nRead);

			//@Function	 close data connection
			//@Return	 void 
			//@Remark    if openFile() success, closeFile() must be call
			virtual void closeFile();

			//@Function	 get last error infos	
			//@Return	 last error infos
			//@Remark
			virtual std::string getLastError();

			//@Function	 set data transmit format, default is binary format
			//@Param     enum TransmitMode transmitMode: data transmit format
			//@Return	 return true if sucess, else return false
			//@Remark    it only save transmit mode to class data member(_transmitMode)
			//           it will only be set until you call openFile()
			//           it should be call before you call openFile()
			virtual bool setTransmitMode(enum TransmitMode transmitMode = binary);

			//@Function	 set data connection establish mode, default is active
			//@Param     enum FTPMode ftpMode: data connection establish mode
			//@Return	 return true if sucess, else return false
			//@Remark    it only save FTP mode to class data member(_ftpMode)
			//           it will only be set until you call openFile()
			//           it should be call before you call openFile()
			virtual bool setFTPMode(enum FTPMode ftpMode = active);

			//@Function	 set log object
			//@Param     ZQ::common::Log* pLog, point log object
			//@Return	 return true if sucess, else return false
			//@Remark    this method must be called first
			virtual void setLog(ZQ::common::Log* pLog);

			//@Function	  split a string into words according delimiter
			//@Param	  std::vector<std::string>& result, storage words
			//@Param	  std::string& str, string to split
			//@Param      const std::string& delimiter,  delimiter
			//@Return	  return true if success, else return false
			//@Remark   
			void splitString(std::vector<std::string> &result, const std::string &str, const std::string &delimiter);
			void saveUserInfo(const std::string& strServer, int& nPort ,const std::string& strUserName, const std::string& strPasswd,const std::string& strLocalNetworkInterface = "");
		protected:
			//@Function	 connect to ftp server
			//@Param	 std::string& strLocalAdress: which local ip address to use for connection
			//@Param	 std::string& strServer: ftp server address
			//@Param     const short nPort: ftp server listen port
			//@Return	 return true if success, else return false
			//@Remark    _controlConnection.closeSocket() must be call if sucess
			bool establishCtrlConnection(const std::string& strLocalAddress, 
				const std::string& strServer, 
				const short nPort);
			
			//@Function	 log on ftp server
			//@Param     std::string strUserName: user name to log on server
			//@Param	 std::string strPasswd: user password to log on server
			//@Return	 return true if success, else return false
			//@Remark    sendCommand("QUIT") must be call if sucess
			bool logOn(const std::string& strUserName, const std::string& strPasswd);

			//@Function	 get one reply from ftp server
			//@Param	 char* buffer : buffer address
			//@Param	 int nLen: buffer size, nLen >= 1024 is must
			//@Param     int& nLenRead: success to read byte number
			//@Return	 return true if success, else return false
			//@Remark    this method is call by getResponse()
            bool getLine(char* buffer, int nLen, int& nCode, int& nLenRead);

			//@Function	 set data connection establish mode as active
			//@Param	 std::string& strFileName: file name, to read or write
			//@Param	 FileMode fileMode: how to operate file , read or write
			//@Return	 retun true if success, else return false
			//@Remark
			bool setActiveMode(const std::string& strFileName, enum FileMode fileMode);

			//@Function	 set data connection establish mode as passive
			//@Param	 std::string& strFileName: file name, to read or write
			//@Param	 FileMode fileMode: how to operate file , read or write
			//@Return	 return true if success, else return false
			//@Remark
			bool setPassiveMode(const std::string& strFileName, enum FileMode fileMode);

			//@Function	 get server socket's ip address and port, then send them to ftp server	
			//@Return	 return true if success, else return false
			//@Remark
			bool sendPortCmd(); 

			//@Function	 send ftp command to ftp server
			//@Param	 char* buffer, command to send 
			//@Param	 int nLen, buffer size
			//@Return    return true if success, else return false	
			//@Remark
			bool sendRequest(char* buffer, int nLen);

			//@Function	 get replys from ftp server
			//@Return	 return true if success, else return false
			//@Remark
			bool getResponse();

			//@Function	 set transmit mode
			//@Return	 return true if success, else return false
			//@Remark
			bool setTransmitModeToServer();

			//@Function	  send command to ftp server, then get response from ftp server
			//@Param      FTP return code for this command if success
			//@Param	  const char* strCmdName, command name
			//@Param	  const std::string strCmdOpt, command option
			//@Return	  return true if success, else return false
			//@Remark   
			bool sendCommand(int nRequireCode, const char* strCmdName, 
				const std::string strCmdOpt = "");

			//@Function	  get server's address and port for data connection
			//@Param	  std::string& strServer, server address for data connection
			//@Param	  short& nDataPort, server port for data connection
			//@Return	  return true if success, else return false
			//@Remark   
			bool getServerAddress(std::string& strServer, short& nDataPort);

			virtual void setIoTimeout(int nMiliSeconds = 4000);

		    bool sendDownloadOffset();
			std::string ridOfEnterFlag(const char* strBuf);
			std::string IntoDirectory(const std::string strFilePath);			
		private:
			SocketAPI::SocketInterface _controlConnection; // control connection
			SocketAPI::SocketInterface _dataConnection; // data connection
			SocketAPI::SocketInterface _serverSocket; // server connection to establish data connetion
		private:
			int _nFTPReturnCode;  // response code of ftp command from server
			char _buffer[1025];   // buffer to receive or send datas
			char* _lpMsgBuf;      // point to error description  
			ZQ::common::Log* _log; // log pointer
			uint32 _dwError;       // error code 
        private:
			FTPMode _ftpMode;     // data connection establish mode
			TransmitMode _transmitMode; // data transmit format
			int64		_nFileOffset;	// the start file offset when download file
		private:
			std::string _strLocalAddress; // local ip address
			std::string _strLastError; // error message
			std::string _strServer;		//server's ip
			int _nPort;					//port
			std::string _strUserName;	//user's name
			std::string _strPassword;	//user's password
			std::string _strLocalNetworkInterface;
		private:
			
			bool _bConnect; // if success to connect ftp server
			bool _bLogOn; // if success to log on ftp server
			bool _bOpenFile;

			unsigned int		_dwTimeoutInMs;		//<io timeout in mili-seconds
			std::string			_strFileName;
			bool _bReadDone;
		}; // end for FTPSocket
	} // end for ContentProvision
} // end for ZQTianShan

#endif 
