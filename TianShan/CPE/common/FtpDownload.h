// File Name: FtpDownload.h
// Description: define interface for ftp client to download file 
// Date: 2008-12

#ifndef __FTP_INTERFACE_H__
#define __FTP_INTERFACE_H__

namespace ZQTianShan
{
	namespace ContentProvision
	{
		class FTPDownload
		{
		public:
			enum TransmitMode{ascii, binary, EBCDIC}; // data transmit format
			enum FTPMode{active, passive}; // data connection establish mode
			enum FileMode{toRead, toWrite}; // operate file mode

		public:
			virtual ~FTPDownload(){};

		public: 

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
				const std::string& strLocalNetworkInterface = "") = 0;

			//@Function	 log off and disconnect ftp server
			//@Return	 void 
			//@Remark    if open() success, close() must be call to release resource
			virtual void close() = 0;

			/// download the specified remote ftp server file to local disk file
			///@param[in] strRemoteFile		the file path name on the ftp server
			///@param[in] strLocalFile		the file path name on local disk
			///@return	true for success, false for failure, call getLastError for the error information if return false
			virtual bool downloadFile(const std::string& strRemoteFile, const std::string& strLocalFile) = 0;

			//@Function  get remote file size
			//@Param	 std::string strFileName: remote file name
			//@Return	 if success return file size, else return -1
			//@Remark
			virtual __int64 getFileSize(const std::string& strFileName) = 0;
       
			//@Function	 establish data connection for transmit file
			//@Param	 std::string strFileName: file name, to read or write
			//@Param	 FileMode fileMode: how to operate file , read or write
			//@Return	 return true if success, else return false
			//@Remark    if openFile() success, closeFile() must be call
			virtual bool openFile(const std::string& strFileName, enum FileMode fileMode = toRead) = 0;

			//@Function	 read file sequence
			//@Param	 char* buffer: buffer to receive datas
			//@Param	 int nSize: byte number to read
			//@Param     int& nRead: success to read byte number
			//@Return	 return true if success, else return false
			//@Remark    openFile() must be called before call this method
			virtual bool readFile(char* buffer, int nSize, int& nRead) = 0;

			//@Function	 close data connection
			//@Return	 void 
			//@Remark    if openFile() success, closeFile() must be call
			virtual void closeFile() = 0;

			//@Function	 get last error infos	
			//@Return	 last error infos
			//@Remark
			virtual std::string getLastError() = 0;

			//@Function	 set data transmit format, default is binary format
			//@Param     enum TransmitMode transmitMode: data transmit format
			//@Return	 return true if sucess, else return false
			//@Remark    call this method before establish data connection
			virtual bool setTransmitMode(enum TransmitMode transmitMode = binary) = 0;

			//@Function	 set data connection establish mode, default is active
			//@Param     enum FTPMode ftpMode: data connection establish mode
			//@Return	 return true if sucess, else return false
			//@Remark    call this method before establish data connection
			virtual bool setFTPMode(enum FTPMode ftpMode = active) = 0;

			//@Function	 set log object
			//@Param     ZQ::common::Log* pLog, point log object
			//@Return	 return true if sucess, else return false
			//@Remark    this method must be called first
			virtual void setLog(ZQ::common::Log* pLog) = 0;

			//@Function  set the read data timeout value, in mili-seconds
			//@param[in]	nMiliSeconds	the time out value, default is 4000ms
			virtual void setIoTimeout(int nMiliSeconds = 4000) = 0;
		};
	}
}
#endif
