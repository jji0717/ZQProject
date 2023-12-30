// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : FtpClient.h
// Branch: 
// Author: 
// Desc  : define interface for ftp client
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPE_FTPCLIENT_INTERFACE_H
#define ZQTS_CPE_FTPCLIENT_INTERFACE_H


#include "ZQ_common_conf.h"
#include <string>


namespace ZQ
{
	namespace common{
		class Log;
	}
}


namespace ZQTianShan
{
	namespace ContentProvision
	{
		class FTPClient
		{
		public:
			enum TransmitMode{ascii, binary, EBCDIC}; // data transmit format
			enum FTPMode{active, passive}; // data connection establish mode
			enum FileMode{toRead, toWrite}; // operate file mode

		public:
			virtual ~FTPClient(){};

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
			virtual int64 getFileSize(const std::string& strFileName) = 0;
       
			//@Function	 establish data connection for transmit file
			//@Param	 std::string strFileName: file name, to read or write
			//@Param	 FileMode fileMode: how to operate file , read or write
			//@Param	 llInitOffset, the initialize file offset when open file, only valid in fileMode "toRead"
			//@Return	 return true if success, else return false
			//@Remark    if openFile() success, closeFile() must be call
			virtual bool openFile(const std::string& strFileName, enum FileMode fileMode = toRead, int64 llInitOffset = 0) = 0;

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


		class FTPClientFactory
		{
		public:
			virtual ~FTPClientFactory(){};
			virtual ZQ::common::Log* getLog() = 0;
			virtual void setLog(ZQ::common::Log* pLog) = 0;
			
			virtual FTPClient* create() = 0;
			
			virtual bool initialize() = 0;
			virtual void uninitialize() = 0;					
		};
	}
}
#endif
