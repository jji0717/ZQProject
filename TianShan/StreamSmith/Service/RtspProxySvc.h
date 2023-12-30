
#ifndef __RTSP_PROXY_SVC_H__
#define __RTSP_PROXY_SVC_H__

#include <ZQDaemon.h>
#include <FileLog.h>


class RtspProxyService : public ZQ::common::ZQDaemon
{
public:
	RtspProxyService();
	virtual ~RtspProxyService();
	
public:

	virtual bool OnInit(void);
	virtual bool OnStart(void);
	virtual void OnStop(void);
	virtual void OnUnInit(void);

private:
	ZQ::common::FileLog*		_svcLog;
	ZQ::common::FileLog*		_pluginLog;
	int							_usrtsPort;
	int 						_uslscPort;





};


#endif //__RTSP_PROXY_SVC_H__

