#ifndef __CVSSCOMMITTHRD_H__
#define __CVSSCOMMITTHRD_H__

#include "common_structure.h"
#include "CVSSImpl.h"
#include "CVSSEnv.h"

class CVSSCommitThrd : public ZQ::common::NativeThread
{
public:
	CVSSCommitThrd(::ZQTianShan::CVSS::CiscoVirtualStreamImplPtr CVSStreamObj,
				  const ::TianShanIce::Streamer::AMD_Stream_commitPtr amdStream);

	~CVSSCommitThrd();

	bool	initialize(void);

	int		run(void);
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

	void final(void)
	{
		delete this;
	}

private:
	::ZQTianShan::CVSS::CiscoVirtualStreamImplPtr _CVSStreamObj;
	const ::TianShanIce::Streamer::AMD_Stream_commitPtr _amdStream;
};
#endif __CVSSCOMMITTHRD_H__