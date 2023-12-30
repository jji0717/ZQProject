
#include "rpcwpreporter.h"
#include "xmlrpc_plus/src/xmlrpc.h"
#include "xmlrpc_plus/src/xmlrpcutil.h"
#include <iostream>
#include <assert.h>

namespace ZQ
{
	namespace rpc
	{
		
		char* sfstrncpy(char* dest, const char* src, size_t destbufsize)
		{
			if (NULL == dest)
			{
				assert(dest);
				return NULL;
			}
			
			if (NULL == src)
			{
				assert(src);
				return NULL;
			}

			size_t i = 0;
			for (; i < destbufsize-1; ++i)
			{
				if ('\0' == src[i])
					break;
				dest[i] = src[i];
			}
			dest[i] = '\0';

			return dest;
		}

		// Default log verbosity: 0 for no messages through 5 (writes everything)
		int RpcLogHandler::_verbosity = 0;

		static class DefaultLogHandler : public RpcLogHandler
		{
public:
	
			void log(int level, const char* msg) { 
#ifdef USE_WINDOWS_DEBUG
			if (level <= _verbosity) { OutputDebugString(msg); OutputDebugString("\n"); }
#else
			if (level <= _verbosity) std::cout << msg << std::endl; 
#endif  
		}
	
		} defaultLogHandler;
		
		// Message log singleton
		RpcLogHandler* RpcLogHandler::_logHandler = &defaultLogHandler;
		
		
		// Default error handler
		static class DefaultErrorHandler : public RpcErrorHandler
		{
public:
	
			void error(const char* msg)
			{
#ifdef USE_WINDOWS_DEBUG
				OutputDebugString(msg); OutputDebugString("\n");
#else
				std::cerr << msg << std::endl; 
#endif  
			}
		} defaultErrorHandler;
		
		
		// Error handler singleton
		RpcErrorHandler* RpcErrorHandler::_errorHandler = &defaultErrorHandler;
		RpcErrorHandler* RpcErrorHandler::getErrorHandler()
		{
			return _errorHandler;
		}

		void RpcErrorHandler::setErrorHandler(RpcErrorHandler* eh)
		{
			_errorHandler = eh;
		}

		RpcLogHandler* RpcLogHandler::getLogHandler()
		{
			return _logHandler;
		}

		void RpcLogHandler::setLogHandler(RpcLogHandler* lh)
		{
			_logHandler = lh;
		}

		int RpcLogHandler::getVerbosity()
		{
			return _verbosity;
		}

		void RpcLogHandler::setVerbosity(int v)
		{
			_verbosity = v;
		}

		int getVerbosity()
		{
			return XmlRpc::getVerbosity();
		}

		void setVerbosity(int level)
		{
			XmlRpc::setVerbosity(level);
		}

	}
}
