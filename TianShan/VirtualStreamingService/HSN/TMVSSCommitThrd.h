#ifndef __ZQTianShan_TMVStreamImpl_H__
#define __ZQTianShan_TMVStreamImpl_H__

#include "TMVStreamImpl.h"
namespace ZQTianShan{
namespace VSS{
namespace TM{

class TMVSSCommitThrd : public ZQ::common::NativeThread
{
public:
	TMVSSCommitThrd(std::string cbNotification,
					std::string ctxNotification,
					::ZQTianShan::VSS::TM::TMVStreamImplPtr TMVStreamObj,
					std::string strPathTicket,
					const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream);

	~TMVSSCommitThrd();

	bool	initialize(void);

	int		run(void);
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

	void final(void)
	{
		delete this;
	}

private:
	::ZQTianShan::VSS::TM::TMVStreamImplPtr _TMVStreamObj;
	TMVSoapClientSession	*_soapClientSession;
	//TMVSSProxy 	*_soapClientSession;
	const ::TianShanIce::Streamer::AMD_Stream_commitPtr _amdStream;
	std::string	_strPathTicket;
	TMVSSEnv &_env;
	std::string _cbNotification;
	std::string _ctxNotification;

	TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap, const TianShanIce::SRM::ResourceType& type, const std::string& strkey);

};
static std::string VariantDefault = "";

}//namespace TM
}//namespace NSS
}//namespace ZQTianShan

#endif __ZQTianShan_TMVStreamImpl_H__