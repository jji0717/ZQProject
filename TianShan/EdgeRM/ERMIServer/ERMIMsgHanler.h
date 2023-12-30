#ifndef __ZQTianShan_ERMIMSGHANDLER_H__
#define __ZQTianShan_ERMIMSGHANDLER_H__
#include "FileLog.h"
#include "RtspInterface.h"
#include "..\definition.h"
#include <list>
#include <vector>
namespace ZQTianShan {
	namespace EdgeRM {

		class ERMIMsgHanler : public ::ZQRtspCommon::IHandler
		{
		public:
			ERMIMsgHanler(ZQ::common::Log& log);
			~ERMIMsgHanler();

			virtual bool HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg);
			virtual void onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);

		private:

			bool doSetup(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
			bool doTeardown(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
			bool GetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
			bool SetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);
			bool Response(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response);

			void generateSessionID(std::string& sessionID);
		protected:
			ZQ::common::Mutex					_genIdCritSec;
			ZQ::common::Log&					_log;

		};

	}//namespace EdgeRM
}//namespace ZQTianShan

#endif __ZQTianShan_ERMIMSGHANDLER_H__
