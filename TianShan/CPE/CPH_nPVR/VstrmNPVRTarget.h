

#ifndef _VSTRMFILESET_TARGETEX_FILTER_
#define _VSTRMFILESET_TARGETEX_FILTER_

#include "FilesetTarget.h"
#include "WriteNotificationI.h"

#define	TARGET_TYPE_VSTRMNPVR	"VSMNPVR"
#define VSTRM_IO_SIZE		64*1024


namespace ZQTianShan {
	namespace ContentProvision {


class VstrmNPVRTarget : public FilesetTarget
{
public:
	VstrmNPVRTarget(FileIoFactory* pFileIoFac);
	virtual ~VstrmNPVRTarget(){};

	void setWriteNotification(WriteNotificationI* pNotify)
	{
		_pNotify = pNotify;
	}

protected:
    virtual bool writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen);

	virtual bool reserveVstrmBW(std::string& errmsg)
	{
		return true;
	}

	virtual bool releaseVstrmBW()
	{
		return true;
	}

	virtual int decideOpenFileFlag(bool bIndexFile);
	virtual void processOutPut();

	WriteNotificationI*		_pNotify;
};

}}


#endif