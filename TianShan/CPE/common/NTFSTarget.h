

#ifndef _NTFS_TARGET_FILTER_
#define _NTFS_TARGET_FILTER_

#include "BaseClass.h"
#include <fstream>

#define	TARGET_TYPE_NTFS	"NTFSIO"

namespace ZQTianShan {
	namespace ContentProvision {

class NTFSTarget : public BaseTarget
{
protected:
	friend class TargetFactory;
	NTFSTarget();
	
public:
	virtual ~NTFSTarget();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual void setFilename(const char* szFile){
		_strFilename = szFile;
	}
	
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}
	
	bool Receive(MediaSample* pSample, int nInputIndex=0);

protected:

	std::ofstream	_fstrm;
	std::string		_strFilename;
	unsigned int	_offset;
};

}}


#endif