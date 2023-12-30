#ifndef __zq_dsmcccrg_client_configuration_header_file_h__
#define __zq_dsmcccrg_client_configuration_header_file_h__

#include <map>
#include <vector>
#include <ConfigHelper.h>

namespace ZQ { 
	namespace DsmccCRGClient
	{
		struct LogCfg
		{
			/*LogCfg()
			{
				level				= 7;
				size				= 20 * 1024 * 1024;
				count				= 10;
				hexdumpSentMsg		= 0;
				hexDumpRcvdMsg		= 0;
			}*/
			int32			level;
			std::string		filename;
			int32			size;
			int32			count;
			int32			hexdumpSentMsg;
			int32			hexDumpRcvdMsg;

			static void structure( ZQ::common::Config::Holder<LogCfg>& holder )
			{
				using namespace ZQ::common::Config;
				holder.addDetail("","level",&LogCfg::level,"",optReadOnly);
				holder.addDetail("","filename",&LogCfg::filename,"",optReadOnly);
				holder.addDetail("","size",&LogCfg::size,"",optReadOnly);
				holder.addDetail("","count",&LogCfg::count,"",optReadOnly);
				holder.addDetail("","hexdumpSentMsg",&LogCfg::hexdumpSentMsg,"",optReadOnly);
				holder.addDetail("","hexDumpRcvdMsg",&LogCfg::hexDumpRcvdMsg,"",optReadOnly);
			}

		};

		struct DsmccCRGServer
		{
			std::string	ip;
			std::string port;
			std::string type;
			std::string protocol;

			static void structure( ZQ::common::Config::Holder<DsmccCRGServer>& holder )
			{
				using namespace ZQ::common::Config;
				holder.addDetail("","type",&DsmccCRGServer::type,"",optReadOnly);
				holder.addDetail("","ip",&DsmccCRGServer::ip,"0.0.0.0",optReadOnly);
				holder.addDetail("","port",&DsmccCRGServer::port,"",optReadOnly);
				holder.addDetail("","protocol",&DsmccCRGServer::protocol,"",optReadOnly);
			}
		};

		struct DsmccCommonHeader
		{
			std::string protocolDiscriminator;
			std::string dsmccType;
			std::string transactionId;
			std::string  reserved;

			static void structure( ZQ::common::Config::Holder<DsmccCommonHeader>& holder )
			{
				using namespace ZQ::common::Config;
				holder.addDetail("","protocolDiscriminator",&DsmccCommonHeader::protocolDiscriminator,"17",optReadOnly);
				holder.addDetail("","dsmccType",&DsmccCommonHeader::dsmccType,"",optReadOnly);
				holder.addDetail("","transactionId",&DsmccCommonHeader::transactionId,"",optReadOnly);
				holder.addDetail("","reserved",&DsmccCommonHeader::reserved,"",optReadOnly);
			}
		};

		struct Request
		{
			std::string operation;
			int			skip;
			std::string protocol;
			std::string		messageId;
			int			waitTime;

			std::map<std::string,std::string> header;
			std::map<std::string,std::string> appdata;

			void readHeader( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
			{
				using namespace ZQ::common::Config;
				Holder<NVPair> nvHolder;
				nvHolder.read(node, hPP);
				header[nvHolder.name] = nvHolder.value;
			}
			void registerHeader( const std::string& ){}

			void readAppdata( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
			{
				using namespace ZQ::common::Config;
				Holder<NVPair> nvHolder;
				nvHolder.read(node, hPP);
				appdata[nvHolder.name] = nvHolder.value;
			}
			void registerAppdata( const std::string&  ){}


			static void structure( ZQ::common::Config::Holder<Request>& holder )
			{
				using namespace ZQ::common::Config;
				holder.addDetail("","operation",&Request::operation,"",optReadOnly);
				holder.addDetail("","skip",&Request::skip,"",optReadOnly);
				holder.addDetail("","protocol",&Request::protocol,"",optReadOnly);
				holder.addDetail("","messageId",&Request::messageId,"",optReadOnly);
				holder.addDetail("Sleep","wait",&Request::waitTime,"",optReadOnly);
				holder.addDetail("Header",&Request::readHeader,&Request::registerHeader);
				holder.addDetail("AppData",&Request::readAppdata,&Request::registerAppdata);
			}
		};


	struct DsmccCRGClientCfg
	{
		DsmccCRGClientCfg()
		{
			protocolType		= 0;
			client				= 1;
			iterationPerClient	= 1;
			loop				= 1;
			interval			= 50;
			messageTimeout		= 5000;
			threadpoolSize		= 3;

		}
		int protocolType;
		int client;
		int iterationPerClient;
		int loop;
		int interval;
		int messageTimeout;
		int threadpoolSize;
		

		typedef ZQ::common::Config::Holder<LogCfg> LogCfgHolder;
		LogCfgHolder	logcfg;
		void readLogCfg( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
		{
			logcfg.read(node,hPP);
		}
		void registerLogCfg(const std::string&){}



		typedef ZQ::common::Config::Holder<DsmccCRGServer> DsmccCRGServerHolder;
		std::vector<DsmccCRGServerHolder> dsmccserver;
		void readDsmccCRGServer( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
		{
			DsmccCRGServerHolder holder;
			holder.read( node , hPP );
			dsmccserver.push_back(holder);
		}
		void registerDsmccCRGServer(const std::string&){}


		typedef ZQ::common::Config::Holder<DsmccCommonHeader> DsmccCommonHeaderHolder;
		DsmccCommonHeaderHolder commonheader;
		void readDsmccCommonHeader( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
		{
			commonheader.read(node,hPP);
		}
		void registerDsmccCommonHeader(const std::string&){}


		typedef ZQ::common::Config::Holder<Request> RequestHolder;
		std::vector<RequestHolder>	request;
		void readRequest( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
		{
			RequestHolder holder;
			holder.read( node , hPP );
			request.push_back(holder);
		}
		void registerRequest(const std::string&){}


		static void structure( ZQ::common::Config::Holder<DsmccCRGClientCfg>& holder )
		{
			using namespace ZQ::common::Config;
			holder.addDetail("","protocolType",&DsmccCRGClientCfg::protocolType,"0",optReadOnly);
			holder.addDetail("","client",&DsmccCRGClientCfg::client,"1",optReadOnly);
			holder.addDetail("","iterationPerClient",&DsmccCRGClientCfg::iterationPerClient,"1",optReadOnly);
			holder.addDetail("","loop",&DsmccCRGClientCfg::loop,"1",optReadOnly);
			holder.addDetail("","interval",&DsmccCRGClientCfg::interval,"50",optReadOnly);
			holder.addDetail("","messageTimeout",&DsmccCRGClientCfg::messageTimeout,"5000",optReadOnly);
			holder.addDetail("","threadpoolSize",&DsmccCRGClientCfg::threadpoolSize,"3",optReadOnly);


			holder.addDetail("Log",&DsmccCRGClientCfg::readLogCfg,&DsmccCRGClientCfg::registerLogCfg);
			holder.addDetail("DsmccCRG/server",&DsmccCRGClientCfg::readDsmccCRGServer,&DsmccCRGClientCfg::registerDsmccCRGServer);

			holder.addDetail("Session/DsmccCommonHeader",&DsmccCRGClientCfg::readDsmccCommonHeader,&DsmccCRGClientCfg::registerDsmccCommonHeader);
			holder.addDetail("Session/Request",&DsmccCRGClientCfg::readRequest,&DsmccCRGClientCfg::registerRequest);
		}
	};

	typedef ZQ::common::Config::Loader<DsmccCRGClientCfg> DsmccCRGClientConfig;

}}//namespace ZQ::DsmccCRGClient

#endif//__zq_dsmcccrg_client_configuration_header_file_h__
