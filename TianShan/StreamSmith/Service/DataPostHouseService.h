// File Name: DataPostHouseService.h
// Date: 2009-03
// Description: Definition of DataPostHouse Service 

#ifndef __DATAPOSTHOUSE_SERVICE_H__
#define __DATAPOSTHOUSE_SERVICE_H__

#include "DataCommunicatorSSL.h"
#include "XMLPreferenceEx.h"

using namespace ZQ::common;

namespace ZQ
{
	namespace StreamSmith
	{
		class ServiceConfig;

		class DataPostHouseService
		{
		public:
			DataPostHouseService(bool includeIPv6 = false);
			virtual ~DataPostHouseService(void);
		public:
			/// new DataPostHouseEnv and DataPostDak
			//bool init(ZQ::common::Log* log, int32 postMen, int32 nReadBufferSize, int32 nEncryptBufferSize, ZQ::DataPostHouse::IDataDialogFactoryPtr dialogCreator = NULL);
			bool init(ServiceConfig& cfg);

			/// It is empty, so do nothing now 
			void uninit();

			/// start DataPost Dak
			bool begin();

			/// stop RTSP ,LSCP communicator if need and stop DataPost Dak
			/// this method can be called many times
			void end();

			/// new TCP RTSP communicator and start it
			bool bindRtsp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort);

			/// new TCP and UDP LSCP communicator and start it
			bool bindLscp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort);

			/// new SSL RTSP communicator and start it
			bool bindSSLRtsp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort);

			/// new SSL LSCP communicator and start it
			bool bindSSLLscp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort);

			/// set SSL certificate file and private key file
			void setCertAndKeyFile(const std::string strCertFile, const std::string strKeyFile, const std::string strCertPasswd);
			
		private:
			/*template<class X> 
			bool newCommunicator(ZQ::DataPostHouse::ObjectHandle<X>XPtr, bool bSuccess,
				std::string strErrorMsg = "No error message", std::string strSuccessMsg = "No sucess message")
			{
				XPtr = new X(*_postDak, *_postHouseEnv);
				if (NULL == XPtr)
				{
					return false;
				}
			}*/

			template<class X>
			void stopCommunicator(ZQ::DataPostHouse::ObjectHandle<X> XPtr, bool& bStart, const std::string strSuccess = "No success message")
			{
				if (XPtr && bStart)
				{
					XPtr->stop();
					bStart = false;
					glog(ZQ::common::Log::L_DEBUG, strSuccess.c_str());
				}
			}

			
		private:
			ZQ::DataPostHouse::DataPostHouseEnv* _postHouseEnv;
			ZQ::DataPostHouse::DataPostDak* _postDak;
			int32 _postMen;
	
		private:
			// RTSP TCP
			ZQ::DataPostHouse::AServerSocketTcpPtr _socketRtspTcpIPv4;
			ZQ::DataPostHouse::AServerSocketTcpPtr _socketRtspTcpIPv6;

			//RTSP SSL
			ZQ::DataPostHouse::SSLServerPtr _socketRtspSSLIPv4;
			ZQ::DataPostHouse::SSLServerPtr _socketRtspSSLIPv6;

			//LSCP TCP
			ZQ::DataPostHouse::AServerSocketTcpPtr _socketLscTcpIPv4;
			ZQ::DataPostHouse::AServerSocketTcpPtr _socketLscTcpIPv6;

			//LSCP SSL
			ZQ::DataPostHouse::SSLServerPtr _socketLscSSLIPv4;
			ZQ::DataPostHouse::SSLServerPtr _socketLscSSLIPv6;

			//LSCP UDP
			ZQ::DataPostHouse::AServerSocketUdpPtr _socketLscUdpIPv4;
			ZQ::DataPostHouse::AServerSocketUdpPtr _socketLscUdpIPv6;

		private:
			bool _bStartRtspTcpIPv4;
			bool _bStartRtspTcpIPv6;

			bool _bStartRtspSSLIPv4;
			bool _bStartRtspSSLIPv6;

			bool _bStartLscTcpIPv4;
			bool _bStartLscTcpIPv6;

			bool _bStartLscSSLIPv4;
			bool _bStartLscSSLIPv6;

			bool _bStartLscUdpIPv4;
			bool _bStartLscUdpIPv6;
		private:
			std::string _strCertificateFile;
			std::string _strPrivatekeyFile;
			std::string _strCertPasswd;

			bool _bIncludeIPv6;
		};

		//#define	MAX_CONN_PER_REQ			FD_SETSIZE
		//#define SF_IDLE_TIMEOUT				300000	/* 5 minute */
		//uint32 sf_idle_timeout = SF_IDLE_TIMEOUT;
		//static uint32 sf_max_polltime = 200;

		#define SVCDBG_LEVEL_MIN				SVCDBG_LEVEL_DISABLE
		#define SVCDBG_LEVEL_DISABLE			0
		#define SVCDBG_LEVEL_FAULT				1
		#define SVCDBG_LEVEL_ERROR				2
		#define SVCDBG_LEVEL_WARNING			3
		#define SVCDBG_LEVEL_NOTICE				4
		#define SVCDBG_LEVEL_DEBUG				5
		#define SVCDBG_LEVEL_MAX				SVCDBG_LEVEL_DEBUG

		/// 服务器的配置管理类
		class ServiceConfig 
		{
		/*public:
			/// 配置信息的数据类型
			struct _ConfigEntry 
			{
				const char* key;
				enum 
				{
					INVALID_TYPE, 
					STRING_VALUE, 
					SHORT_VALUE, 
					USHORT_VALUE, 
					LONG_VALUE,
					ULONG_VALUE,
					DISPATCH_ENTRY, 
				} type;
				void* value;
				int	maxValue;
			};*/
		public:
			ServiceConfig();
			virtual ~ServiceConfig();

		/*	/// 基类的配置表
			virtual const _ConfigEntry* getBaseConfigMap();

			/// 当前配置表
			virtual const _ConfigEntry* getConfigMap();

			/// 加载一个配置文件
			virtual bool load(const char* fileName,std::vector<std::string>& path=std::vector<std::string>());

		protected:
			/// 处理配置文件
			bool processEntry(XMLPreferenceEx* pref, const _ConfigEntry* cfgEntry);*/
		public:
			char _cfg_publicKeyFile[MAX_PATH];
			char _cfg_privateKeyFile[MAX_PATH];
			char _cfg_privateKeyFilePwd[64];
			char _cfg_dhParamFile[MAX_PATH];
			char _cfg_randFile[MAX_PATH];
			int32 _cfg_debugLevel;
			int32 _cfg_postMen;
			int32 _cfg_readBufferSize;
			int32 _cfg_encryptBufferSize;
			int32 _cfg_maxConn;
			ZQ::common::Log* _cfg_log;
			ZQ::DataPostHouse::IDataDialogFactoryPtr _cfg_dialogCreator;

		};

	}
}

#endif

