// IndexData.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <IdxFileParserEnvironment.h>
#include <IndexFileParser.h>

using namespace ZQ::IdxParser;	



int _tmain(int argc, _TCHAR* argv[])
{
	
	if( argc < 2 )
	{
		printf("Usage: %s assetPathName\n",argv[0]);
		return -1;
	}

	IdxParserEnv env;
// 	env.InitVstrmEnv( );
// 	env.setUseVstrmIndexParseAPI(true);

	IndexFileParser parser(env);
	IndexData idxData;

	if( !parser.ParserIndexFileFromCommonFS( argv[1], idxData ) )
	{
		printf("failed to parse index file for [%s]\n",argv[1] );
		return -1;
	}

	printf("MAINFILE:  \t\t%s\n",argv[1]);
	printf("PLAYTIME:  \t\t%u\n",idxData.getPlayTime());
	printf("LENGTH:    \t\t%lld\n",idxData.getMainFileSize());
	printf("TYPE:      \t\t%s\n",idxData.getIndexTypeString(idxData.getIndexType()));
	printf("BITRATE:   \t\t%u\n",idxData.getMuxBitrate() );
	
	for( int i = 1 ; i < idxData.getSubFileCount(); i++ )
	{
		printf("\tSUBFILE:        \t\t\t%d\n",i);
		printf("\tSUBFILENAME:    \t\t\t%s\n",idxData.getSubFileName(i).c_str() );
		SPEED_IND speed = 	idxData.getSubFileSpeed(i);
		printf("\tSUBFILE speed:  \t\t\t%d/%d\n\n", speed.numerator , speed.denominator );
	}

	return 0;
}

