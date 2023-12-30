#ifndef __NATIVE_SERVICE__
#define __NATIVE_SERVICE__


#include "ZQDaemon.h"
#include "FileLog.h"

namespace ZQTianShan {
namespace ContentStore {


class NativeService: public ZQ::common::ZQDaemon {

public:

    NativeService();
	virtual ~NativeService();

public:

    virtual bool OnInit(void);
	virtual bool OnStart(void);
	virtual void OnStop(void);
	virtual void OnUnInit(void);

	ZQ::common::FileLog* _iceLogFile;

	std::string	_rootPath;
};

}}

#endif
