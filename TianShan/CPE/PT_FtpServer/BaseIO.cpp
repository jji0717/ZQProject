
#include "Log.h"
#include "BaseIO.h"
#include "NTFSIO.h"

#ifdef ZQ_OS_MSWIN
#include "VstrmIO.h"

BaseIOI::IOINFO BaseIOI::_Infos[] = {
	{VStrmIO::Init, VStrmIO::Uninit,VStrmIO::Create, "VstrmIO", BaseIOI::IO_VSTRM, 0},
	{NTFSIO::Init, NTFSIO::Uninit, NTFSIO::Create, "NtfsIO", BaseIOI::IO_NTFS, 0},
};
#else//linux now no vstream
BaseIOI::IOINFO BaseIOI::_Infos[] = {
	{NTFSIO::Init, NTFSIO::Uninit, NTFSIO::Create, "NtfsIO", BaseIOI::IO_NTFS, 0},
};
#endif

int BaseIOI::_nInfo = sizeof(BaseIOI::_Infos)/sizeof(BaseIOI::IOINFO);
ZQ::common::Mutex BaseIOI::_lock;
int BaseIOI::_nDefaultIOType = BaseIOI::IO_NTFS;

bool BaseIOI::setDefaultIO(int nType)
{
	for(int i=0;i<_nInfo;i++)
	{
		if (nType == _Infos[i].nIOType)
		{
			_nDefaultIOType = nType;
			return true;
		}
	}
	
	return false;
}

BaseIOI* BaseIOI::CreateInstance()
{
	return CreateInstance(_nDefaultIOType);
}

BaseIOI* BaseIOI::CreateInstance(int nType)
{
	ZQ::common::Guard<ZQ::common::Mutex> op(_lock);

	for(int i=0;i<_nInfo;i++)
	{
		if (nType == _Infos[i].nIOType)
		{
			if (!_Infos[i].bInited)
			{
				if(!_Infos[i].pfInit())
				{
					_Infos[i].pfUninit();
					return NULL;
				}

				_Infos[i].bInited = true;
			}

			return _Infos[i].pfCreate();
		}
	}

	return NULL;
}

bool BaseIOI::Init()
{	
	return true;
}

void BaseIOI::Uninit()
{
	ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
	
	for(int i=0;i<_nInfo;i++)
	{
		if (_Infos[i].bInited)
		{
			_Infos[i].pfUninit();
			_Infos[i].bInited = false;
		}
	}
}

void BaseIOI::Release()
{
	delete this;
}

BaseIOI::BaseIOI()
{
	_bOpened = false;
}

int BaseIOI::GetRecommendedIOSize()
{
	return DEF_IORW_SIZE;
}

BaseIOI::~BaseIOI()
{
}
