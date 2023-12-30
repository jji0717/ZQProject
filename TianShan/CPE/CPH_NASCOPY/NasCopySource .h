

#ifndef _NAS_COPY_SOURCE_FILTER_
#define _NAS_COPY_SOURCE_FILTER_


#include "BaseClass.h"
#include "NativeThread.h"

#include <fstream>

#include "VstrmUser.h"
#include "VodDriverApi.h"


#define	SOURCE_TYPE_NASCOPY		"NasCopySource"


#ifndef OBJECT_ID
#define OBJECT_ID ULONG 
#endif

namespace ZQTianShan{
	namespace ContentProvision{
		class BaseGraph;
	}};	

namespace ZQTianShan {
	namespace ContentProvision {

	   
class NasCopySource;
typedef struct
{
	VHANDLE				vstrmClassHandle;
	IOCTL_LOAD_PARAMS_LONG	loadParams;
	char 				UnprefixedInputObjectName[MAX_OBJECT_NAME_LONG];
	char 				InputObjectName[MAX_OBJECT_NAME_LONG];
	char				OutputObjectName[MAX_OBJECT_NAME_LONG];
	unsigned char		NodeName[32];
	ULONG				NumberOfPorts;
	long				sessionId;
	BOOLEAN				sessionIdSpecified;
	LARGE_INTEGER		fileSize;
	BOOLEAN				RaidLevelSet;
	ULONG				RaidLevel;
	ULONG64				bitRate;
	BOOLEAN				total;
	BOOLEAN				Elapsed;
	char				ErrorString[32];
	VSTATUS				status;
	BOOLEAN				StatusSet;
	UCHAR               Temp[MAX_OBJECT_NAME_STRING_LONG];
	FILE_ATTRIBUTES		FileAttributes;
	ULONG64				bwTicketFile;
	ULONG64				bwTicketHostNic;
	HANDLE				hThread;
	ULONG				activeCopyCount;
	NasCopySource*      pThis;
	int                 fileIndex;
} PX;

class NasCopySource : public BaseSource
{
protected:
	friend class SourceFactory;
public:
	NasCopySource();
	~NasCopySource();

public:
	virtual bool Init();
	virtual bool Start();

	virtual void Stop();

	virtual void Close();


	void setSourceFileName(const char* srcFilename);
	void setDestFileName(const char* desFilename);
	void setBandwith(int bitrate);
	void setVstrmBwClientId(unsigned int vstrmBwClientId);
	void setDisableBitrateLimit(bool bDisableBitrateLimit);

	virtual void endOfStream();

	virtual const char* GetName();
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

	virtual LONGLONG getProcessBytes();
	virtual bool Run();

	virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	virtual bool seek(int64 offset, int pos);

protected:
	VSTATUS AllocBw(PX *pxP,ULONG ClientId);
	VOID FreeBw(PX *pxP);
	VSTATUS VstrmCopy(PX* pxP);

	void setVstrmError(HANDLE hVstrmClass, VSTATUS status, const char* szFunc);

	VSTATUS sessionDoneCb(PIOCTL_STATUS_BUFFER_LONG sbP, PX *pxP);
	static VSTATUS SessionDoneV2Cb (HANDLE classHandle, PVOID cbParm, PVOID bufP, ULONG bufLen);

	void checkFinish();

private:
	PX                         gPx[12];
	int						   _nTotalFileCount;
	std::string                _srcFilename;
	std::string                _desFilename;
	int		                   _biterate;
	DWORD                      _bwmgrClientId;
	LONGLONG                    _lfsize;
	HANDLE						_hPartOne;
	HANDLE						_hPartTwo;
	bool						_bDisableBitrateLimit;
};

}}

#endif