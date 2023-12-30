#include <memory>
#include "utils.h"

namespace ZQ
{
	namespace common{
		class Log;
	}
}

namespace ZQTianShan 
{
	namespace ContentProvision
	{

		class FileIoFactory;
		class FileIo;


class IOInterface
{
public:
	IOInterface(FileIoFactory* pfilefac);
	virtual ~IOInterface();

	bool Open(const char* szFile, int nOpenFlag);
	int64 GetFileSize(const char* szFile);
	int Read(char* pPtr, int nReadLen);
	bool Write(char* pPtr, int nWriteLen);
	bool Seek(int64 lOffset, int nPosFlag);
	void Close();
	int GetRecommendedIOSize();

	bool ReserveBandwidth(int nbps);
	void ReleaseBandwidth();


protected:
	FileIoFactory*                          _pFileIoFactory;
	FileIo*                                 _pFileIo;
	bool                                    _bOpened;
	std::string                             _strFile;
	uint64								    _bwTicket;
	unsigned int						    _dwBandwidth;
};

}
}

