#ifndef __CONTROL_VLC_SERVICE_H__
#define __CONTROL_VLC_SERVICE_H__

namespace ZQTianShan{
namespace VSS{
namespace VLC{

class ControlVLCService : public ZQ::common::NativeThread
{
public:
	ControlVLCService(::ZQ::common::FileLog& filelog, 
		const std::string szServerName, 
		const std::string strServerIP,
		uint16 uServerPort,
		const std::string strServerPwd,
		int nInterval);
	~ControlVLCService();
protected:
	virtual bool init(void);
	virtual int run(void);
private:
	bool startVLC();
	bool stopVLC();
	bool isVLCRunning();
	void test();
private:
	::ZQ::common::FileLog& _filelog;
	std::string _szServiceName;
	std::string _strServerIP;
	uint16 _uServerPort;
	std::string _strServerPwd;
	std::string _strVLCCmd;
	int _nInterval;
	DWORD _pid;
	HANDLE _vlcHandle;
	HANDLE _quitHandle;
};




}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan

#endif