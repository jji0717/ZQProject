
#include "IdxFileParserEnvironment.h"
//#ifndef EXCLUDE_VSTRM_API
//	#include <StreamSmithConfig.h>
//#endif

#define IDXPARSERLOG	if( _idxParserEnv.GetLogger() ) (*_idxParserEnv.GetLogger())

namespace ZQ
{
namespace IdxParser
{


IdxParserEnv::IdxParserEnv( )
:mpVstrmLoadAssetInfoMethod(NULL),
mpVstrmLoadAssetInfoExMethod(NULL)
{
#ifndef EXCLUDE_VSTRM_API
	vstrmHandle		=	NULL;
	bVstrmInitialByCaller = false;
#endif//EXCLUDE_VSTRM_API
	logger			=	NULL;
	bLoggerIntialiByCaller=	false;
	mbCanUseVstrmAPIToParseIndexFile = false;
	mbSkipZeroByteFile	= false;
	mbUseVsOpen = false;
}

IdxParserEnv::~IdxParserEnv( )
{
#ifndef EXCLUDE_VSTRM_API
	UninitVstrmEnv( );
#endif//EXCLUDE_VSTRM_API

	DetachLogger( );

	if(mpVstrmLoadAssetInfoMethod )
	{
		delete mpVstrmLoadAssetInfoMethod ;
		mpVstrmLoadAssetInfoMethod = NULL;
	}
	if( mpVstrmLoadAssetInfoExMethod )
	{
		delete mpVstrmLoadAssetInfoExMethod;
		mpVstrmLoadAssetInfoExMethod = NULL;
	}
}

#ifndef EXCLUDE_VSTRM_API

void IdxParserEnv::setUseVstrmIndexParseAPI( bool bUse )
{
	mbCanUseVstrmAPIToParseIndexFile = bUse;
}

bool IdxParserEnv::canUseVstrmIndexParseAPI( ) const
{
	return mbCanUseVstrmAPIToParseIndexFile;
}


void IdxParserEnv::InitVstrmEnv( VHANDLE vstrmHandleValue /* = NULL */ )
{

	mbSkipZeroByteFile	= true;//( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.skipZeroByteFiles >= 1) ;
	mbUseVsOpen			= true;//( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.useVsOpenAPI >=  1 );

	UninitVstrmEnv();

	//load 
	if( mVstrmLoadAssetInfoObj.load("VstrmDllEx.dll"))
	{
		mpVstrmLoadAssetInfoMethod = new VstrmLoadAssetInfo(mVstrmLoadAssetInfoObj);
		if( !mpVstrmLoadAssetInfoMethod->isValid() )
		{
			delete mpVstrmLoadAssetInfoMethod;
			mpVstrmLoadAssetInfoMethod = NULL;
		}
		mpVstrmLoadAssetInfoExMethod = new VstrmLoadAssetInfoEx(mVstrmLoadAssetInfoObj);
		if(!mpVstrmLoadAssetInfoExMethod->isValid())
		{
			delete mpVstrmLoadAssetInfoExMethod;
			mpVstrmLoadAssetInfoExMethod = NULL;
		}		
	}
	
	if( vstrmHandleValue != NULL )
	{
		vstrmHandle = vstrmHandleValue;
		bVstrmInitialByCaller = true;
	}
	else
	{
		VstrmClassOpenEx (&vstrmHandle );
		assert( vstrmHandle != NULL );
		bVstrmInitialByCaller = false;
	}
}
void IdxParserEnv::UninitVstrmEnv( )
{
	if( !bVstrmInitialByCaller )
	{
		if( vstrmHandle != NULL )
			VstrmClassCloseEx( vstrmHandle );
	}
	vstrmHandle = NULL;
}
#endif//EXCLUDE_VSTRM_API

void IdxParserEnv::DetachLogger( )
{
	if( !bLoggerIntialiByCaller )
	{
		if( logger)
		{
			delete logger;
		}
	}	
	logger = NULL;
}

void IdxParserEnv::AttchLogger( ZQ::common::Log* logInstance )
{	
	DetachLogger();
	if( logInstance)
	{
		logger = logInstance;
		bLoggerIntialiByCaller = true;
	}
	else
	{
// 		logger = &ZQ::common::NullLogger;
// 		bLoggerIntialiByCaller = false;
	}
}


}}//namespace ZQ::IdxParser
