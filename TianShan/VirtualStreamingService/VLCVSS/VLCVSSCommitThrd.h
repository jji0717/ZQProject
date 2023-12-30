#ifndef __ZQTianShan_VLCStreamImpl_H__
#define __ZQTianShan_VLCStreamImpl_H__

#include "VLCStreamImpl.h"
namespace ZQTianShan{
namespace VSS{
namespace VLC{

class VLCVSSCommitThrd : public ZQ::common::NativeThread
{
public:
	VLCVSSCommitThrd(::ZQTianShan::VSS::VLC::VLCStreamImplPtr TMVStreamObj,
					std::string strPathTicket,
					const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream);

	~VLCVSSCommitThrd();

	bool	initialize(void);

	int		run(void);
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

	void final(void)
	{
		delete this;
	}

private:
	::ZQTianShan::VSS::VLC::VLCStreamImplPtr _VLCStreamObj;
	//VLCTelnetSession	*_vlcClientSession;
	//TMVSSProxy 	*_soapClientSession;
	const ::TianShanIce::Streamer::AMD_Stream_commitPtr _amdStream;
	std::string	_strPathTicket;
	VLCVSSEnv &_env;
	std::string _cbNotification;
	std::string _ctxNotification;

	TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap, const TianShanIce::SRM::ResourceType& type, const std::string& strkey);

};
static std::string VariantDefault = "";

}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan

#endif __ZQTianShan_VLCStreamImpl_H__