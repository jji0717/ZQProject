#ifndef _CPH_AQUALIB_SESSION_H
#define _CPH_AQUALIB_SESSION_H

#include <BaseCPH.h>
#include <BaseClass.h>
#include <FileIo.h>

#ifdef max
#undef max
#endif

#include <AquaUpdate.h>
#include <AquaFileIoFactory.h>

class PacedIndexFactory;

namespace ZQTianShan {
namespace ContentProvision{

class CPHAquaLibSess : public BaseCPHSession, public BaseGraph
{
public:
    CPHAquaLibSess(ZQ::common::Log* log, BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess);
    virtual ~CPHAquaLibSess();

public:
	virtual bool Start(); 
    virtual bool preLoad(); 
    virtual bool prime(); 
    virtual void terminate(bool bProvisionSuccess=true);
    virtual bool getProgress(::Ice::Long& offset, ::Ice::Long& total);

    virtual void OnProgress(int64& prcvBytes);
    virtual void OnStreamable(bool bStreamable);
    virtual void OnMediaInfoParsed(MediaInfo& mInfo);
protected:
    // impl of ThreadRequest
    virtual int run(void);
    virtual void final(int retcode =0, bool bCancelled =false);
    void cleanup();

    void updateMainMetadataOnStart(::TianShanIce::Properties& params);
    void updateMainMetadataOnStop(bool errorOccurred, ::TianShanIce::Properties& params);
    void updateMainMetadataOnProcess(::TianShanIce::Properties& params);
    void updateMainMetadataOnStreamable(::TianShanIce::Properties& params);

    bool updateMainFileMetadata();
    void OnPreloadFailed();

public:
    static std::auto_ptr<FileIoFactory> _pFileIoFac;
    static std::auto_ptr<FileIoFactory> _pCifsFileIoFac;
    static PacedIndexFactory*               _pPacedIndexFac;
    static void*                            _pPacedIndexDll;

protected:
    ZQ::common::Log*            _pCPELogger;
    bool                        _bPushTrigger;
    int                         _bitrate;
    bool                        _bStartEventSent;

    std::string                 _strMethod;
    int                         _nBandwidth;
    ::Ice::Long                 _filesize, _processed;
    bool                        _bQuit;
    bool                        _bCleaned;
    BaseTarget*		            _pMainTarget;	//main target that will send streamable event
    BaseProcess*		        _pRTFProc;		//the rtf process
    bool                        _bSuccessMount;
    std::string                 _sharePath;
    std::string                 _strLocalIp;
    std::string                 _strFileName;   
    std::string                 _paid;
    std::string                 _pid;

	std::string					_sourceType;
	bool					    _enableNoTrickSpeed;
    bool					    _bIndexVVC;

    CRM::A3Message::A3AquaContentMetadata::Ptr  _pA3AquaContentMetadata;

	Ice::Long	_nSchedulePlayTime;	//in milliseconds

#ifdef PROCESSED_BY_TIME
	Ice::Long   _nsessStartTime;
#endif

	int64					 _processedTime;
};
class CPHAquaLibAutoCheckSess : public CPHAquaLibSess
{
public:
	CPHAquaLibAutoCheckSess(ZQ::common::Log* log, BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess);
	virtual ~CPHAquaLibAutoCheckSess();

public:
	virtual bool Start(); 
	virtual bool preLoad(); 
protected:
	bool  setSpeedInfo();
	bool  checkFileFormat();
private:
	ZQTianShan::ContentProvision::BaseSource*       _pSource;
	std::string _protocol;
	std::string _sourceURL;

	std::string _strLocalIp;
	std::string _strFileName;

	std::list<float>		_trickspeed;	
	std::list<float> _trickspeedHD;
	uint16					_augmentationPids[ 4 ];
	int						_augmentationPidCount ;
	int						_nMaxBandwidth;
	std::string				_strMountPoint;
};
}} // namespace ZQTianShan::ContentProvision

#endif