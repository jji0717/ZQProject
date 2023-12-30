#include "ParseIndexFile.h"
#include "VvxParser.h"
#include "VV2Parser.h"
#define _VSTRM_DATA_TYPE_DEFINED
#include "IndexFileParser.h"

bool ParseIndexFileInfo::getIdxSubFileInfo(const char* szIdxFile, std::string  strIndexExt, std::vector<SubFileInfo>& subFiles, ZQTianShan::ContentProvision::MediaInfo& info)
{
	int64 SparseFileSize = 20000000000;
	if (stricmp(strIndexExt.c_str(), "VVX") == 0)
	{
		VvxParser vvx;
		if(!vvx.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.videoResolutionH = vvx.getVideoHorizontalSize(); 
		info.videoResolutionV = vvx.getVideoVerticalSize();
		info.bitrate = vvx.GetBitRate();
		info.framerate = atof(vvx.getFrameRateString(vvx.getFrameRateCode()));
		info.playTime = vvx.GetPlayTime();

		//		subFiles.resize(vvx.getSubFileCount());
		for(int i=0;i<vvx.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			subinfo.firstOffset = 0;
			subinfo.finalOffset = 0;
			subinfo.totalFilesize = 0;
			subinfo.bIsSparseFile = false;

			vvx.getSubFileExtension(i,subinfo.ext, sizeof(subinfo.ext));
			vvx.getSubFileSpeed((uint32)i,subinfo.numerator,*(uint32*)&subinfo.denominator);
			if (subinfo.numerator<0)
			{
				subinfo.numerator = 0 - subinfo.numerator;
				subinfo.direction = -1;
			}
			else
			{
				subinfo.direction = 1;
			}
			subFiles.push_back(subinfo);
		}		
	}
	else if(stricmp(strIndexExt.c_str(), "VV2") == 0)
	{
		VV2Parser vv2;
		if(!vv2.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.bitrate = vv2.getBitrate();
		info.playTime = vv2.getPlayTime();

		//		subFiles.resize(vv2.getSubFileCount());
		for(unsigned int i=0;i<vv2.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			subinfo.firstOffset = 0;
			subinfo.finalOffset = 0;
			subinfo.totalFilesize = 0;
			subinfo.bIsSparseFile = false;
			vv2.getSubFileExtension(i,subinfo.ext, sizeof(subinfo.ext));
			vv2.getSubFileSpeed(i, subinfo.numerator, subinfo.denominator, subinfo.direction);			
			subFiles.push_back(subinfo);
		}		
	}
	else if(stricmp(strIndexExt.c_str(), "index") == 0)
	{
		ZQ::IdxParser::IdxParserEnv  env;
		env.AttchLogger(&glog);
		ZQ::IdxParser::IndexFileParser idxParser(env);
		ZQ::IdxParser::IndexData	idxData;
		if(!idxParser.ParserIndexFileFromCommonFS("",idxData, true, szIdxFile)) {
			return false;
		}

		info.videoResolutionH = idxData.getVideoHorizontalSize(); 
		info.videoResolutionV = idxData.getVideoVerticalSize();
		info.bitrate = idxData.getMuxBitrate();
		info.framerate = atof(idxData.getFrameRateString());
		info.playTime = idxData.getPlayTime();

		//		subFiles.resize(idxData.getSubFileCount());
		for(int i=0;i<idxData.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			ZQ::IdxParser::IndexData::SubFileInformation subfileinfo;
			idxData.getSubFileInfo(i, subfileinfo);
			subinfo.firstOffset = subfileinfo.startingByte;
			subinfo.finalOffset = subfileinfo.endingByte;

			if(subinfo.firstOffset > 0 && subinfo.finalOffset > 0 && subinfo.firstOffset > SparseFileSize)
				subinfo.bIsSparseFile = true;
			else
				subinfo.bIsSparseFile = false;
			subinfo.totalFilesize = subfileinfo.fileSize;
			const std::string& strExtension =  idxData.getSubFileName( i ) ;
			memset(subinfo.ext, 0 , sizeof(subinfo.ext));
			strncpy(subinfo.ext, strExtension.c_str(), sizeof(subinfo.ext));

			SPEED_IND speed = idxData.getSubFileSpeed((uint32)i);
			subinfo.numerator = speed.numerator;
			subinfo.denominator = speed.denominator;

			if (subinfo.numerator<0)
			{
				subinfo.numerator = 0 - subinfo.numerator;
				subinfo.direction = -1;
			}
			else
			{
				subinfo.direction = 1;
			}
			subFiles.push_back(subinfo);
		}		
	}
	else
		return false;
	return true;
}

