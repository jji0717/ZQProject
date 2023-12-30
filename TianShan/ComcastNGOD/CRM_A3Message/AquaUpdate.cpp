#include "AquaUpdate.h"

namespace CRM
{
	namespace A3Message
	{
#define JSON_HAS(OBJ, CHILDNAME) (OBJ.isObject() && OBJ.isMember(CHILDNAME))

		void JsonValue2StringMap(const Json::Value& jsonValue, TianShanIce::Properties& metadata)
		{
			metadata.clear();
			Json::Value::Members members = jsonValue.getMemberNames();
			Json::Value::Members::iterator itorMem= members.begin();
			try
			{
				for( ; itorMem != members.end(); ++itorMem)
				{
					std::string& key = *itorMem; 
					
					if(jsonValue[key].type() == Json::stringValue)
						MAPSET(TianShanIce::Properties, metadata, key, jsonValue[key].asString());
				}
			}
			catch (...)
			{
				
			}
		}
		void StringMap2JsonValue(const TianShanIce::Properties& metadata, Json::Value& jsonValue)
		{
			for(TianShanIce::Properties::const_iterator itorProp = metadata.begin(); itorProp != metadata.end(); itorProp++)
			{
				jsonValue[itorProp->first] = itorProp->second;
			}    
		}
		
		void contentMD2aquaMD(const TianShanIce::Properties& metadata, TianShanIce::Properties& jsonMD)
		{
			std::string oriStr , newStr;

			for(TianShanIce::Properties::const_iterator itorProp = metadata.begin(); itorProp != metadata.end(); itorProp++)
			{
				std::string strKey = itorProp->first;

				//判断key值中是否有sys.或者user.
				if( (strKey.find("sys.") != std::string::npos) ||  (strKey.find("user.") != std::string::npos))
				{
					std::replace(strKey.begin(), strKey.end(), '.', '_');
				}

				jsonMD[strKey] = itorProp->second;

				oriStr+= std::string("[") + itorProp->first + std::string(":") + itorProp->second + std::string("] ");
				newStr+= std::string("[") + strKey + std::string(":") + itorProp->second + std::string("] ");
			}  
			//printf("contentMD2aquaMD:  oriStr=%s\n", oriStr.c_str());
			//printf("contentMD2aquaMD:  newStr=%s\n", newStr.c_str());
		}
		void aquaMD2contentMD(const TianShanIce::Properties& jsonMD, TianShanIce::Properties& metadata)
		{
			std::string oriStr , newStr;
			for(TianShanIce::Properties::const_iterator itorProp = jsonMD.begin(); itorProp != jsonMD.end(); itorProp++)
			{
				std::string strKey = itorProp->first;

				//判断key值中是否有sys_或者user_
				if( (strKey.find("sys_") != std::string::npos) ||  (strKey.find("user_") != std::string::npos))
				{
					std::replace(strKey.begin(), strKey.end(), '_', '.');
				}

				metadata[strKey] = itorProp->second;

				oriStr+= std::string("[") + itorProp->first + std::string(":") + itorProp->second + std::string("] ");
				newStr+= std::string("[") + strKey + std::string(":") + itorProp->second + std::string("] ");
			}  
			//printf("aquaMD2contentMD:  oriStr=%s\n", oriStr.c_str());
			//printf("aquaMD2contentMD:   newStr=%s\n", newStr.c_str());
		}
		////////////////////////////////////////////////////////
		//////////////////////class A3AquaBase//////////////////
		////////////////////////////////////////////////////////
		A3AquaBase::A3AquaBase( ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps,const std::string& mainFileExtension , const std::string& contentNameFormat) :
		_log(log), _cdmiFuseOps(cdmiFuseOps), _mainFileExtension(mainFileExtension), _contentNameFormat(contentNameFormat)
		{

		}

		A3AquaBase::~A3AquaBase(void)
		{

		}
		bool A3AquaBase::CreateFile(const std::string& fileName, const char* value, uint len)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaBase, "CreateFile()file[%s]filesize[%d]"), fileName.c_str(), len);

			int64 lStart =  ZQ::common::now(); 
			std::string contentType = "";
			std::string uri = _cdmiFuseOps.pathToUri(fileName);
            CdmiFuseOps::CdmiRetCode retCode = _cdmiFuseOps.nonCdmi_CreateDataObject(uri, contentType, value, len);

			if(!CdmiRet_SUCC(retCode))
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "CreateFile()failed to create file[%s] with errorCode[%d==>]"), fileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
				return false;
			}
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "CreateFile()create file[%s]filesize[%d] successfully took %dms"),
				fileName.c_str(),len, (int)(ZQ::common::now() - lStart));
			return true;
		}
		bool A3AquaBase::updateFile(const std::string& fileName, const char* value, uint len)
		{
            //nonCdmi_UpdateDataObject(const std::string& uri, out std::string& location, const std::string& contentType, uint64 startOffset, 
			// uint len, const char* buff, int64 objectSize=-1, bool partial=false, bool disableCache = false);
			std::string uri = _cdmiFuseOps.pathToUri(fileName);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaBase, "updateFile()file[%s]filesize[%d]"), fileName.c_str(), len);

			int64 lStart =  ZQ::common::now(); 
			std::string contentType = "", location;

			CdmiFuseOps::CdmiRetCode  retCode = _cdmiFuseOps.nonCdmi_UpdateDataObject(uri, location, contentType, 0, 0, NULL, 0, false, true);
			if(!CdmiRet_SUCC(retCode))
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "updateFile()failed to truncate file[%s] with errorCode[%d]"), fileName.c_str(), retCode);
				return false;
			}

			retCode = _cdmiFuseOps.nonCdmi_UpdateDataObject(uri, location, contentType, 0, len, value, len, false, true);

			if(!CdmiRet_SUCC(retCode))
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "updateFile()failed to update file[%s] with errorCode[%d]"), fileName.c_str(), retCode);
				return false;
			}

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "updateFile()update file[%s]filesize[%d] successfully took %dms"),
				fileName.c_str(),len, (int)(ZQ::common::now() - lStart));
			return true;
		}
		///read file from startoffset=0 and length=len to buf
		bool A3AquaBase::readFile(const std::string& fileName,  Json::Value& value, int64& filesize)
		{
			//CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType,
			//std::string& location,uint64 startOffset, in out uint& len, char* recvbuff, bool disableCache);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaBase, "readFile()file[%s]filesize[%lld]"), fileName.c_str(), filesize);

			int64 lStart =  ZQ::common::now(); 

			std::string contentType,location;
			std::string recvBuff;
			recvBuff.resize((uint)filesize);
            uint32 len = (int) filesize;
			std::string uri = _cdmiFuseOps.pathToUri(fileName);
			CdmiFuseOps::CdmiRetCode retCode = _cdmiFuseOps.nonCdmi_ReadDataObject(uri, contentType, location, 0, len, (char*)recvBuff.c_str(), true);

			if(!CdmiRet_SUCC(retCode))
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "readFile()failed to read file[%s] with errorCode[%d]"), fileName.c_str(), retCode);
				return false;
			}

			Json::Reader reader;
			if(!recvBuff.empty() && !reader.parse(recvBuff, value))
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "readFile()file[%s] failed to parse response len[%d] took %dms: %s"),
					fileName.c_str(), recvBuff.length(), (int)(ZQ::common::now() - lStart), reader.getFormatedErrorMessages().c_str());
				return false;
			}
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "readFile()read file[%s]filesize[%lld] successfully took %dms"),
				fileName.c_str(),filesize, (int)(ZQ::common::now() - lStart));
			return true;
		}
		bool A3AquaBase::deleteFile(const std::string& fileName)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaBase, "deleteFile()file[%s]"), fileName.c_str());

			int64 lStart =  ZQ::common::now(); 
			std::string contentType = "";
			std::string uri = _cdmiFuseOps.pathToUri(fileName);
			CdmiFuseOps::CdmiRetCode retCode = _cdmiFuseOps.nonCdmi_DeleteDataObject(uri);

			if(!CdmiRet_SUCC(retCode))
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "deleteFile()failed to delete file[%s] with errorCode[%d]"), fileName.c_str(), retCode);
				return false;
			}
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "deleteFile()delete file[%s]successfully took %dms"),
				fileName.c_str(), (int)(ZQ::common::now() - lStart));
			return true;
		}
		CdmiFuseOps::CdmiRetCode A3AquaBase::checkFileStatus(const std::string& fileName, Json::Value& value)
		{
			std::string uri = _cdmiFuseOps.pathToUri(fileName) + "?metadata";
			std::string location;
			return  _cdmiFuseOps.cdmi_ReadDataObject(value, uri , location);
		}
		bool A3AquaBase::getFileSize(const Json::Value& metadata, int64& filesize)
		{
			if (!metadata.isObject())
				return false;

#ifdef ZQ_OS_MSWIN
			//	COPY_METADATA_VAL(fileInfo.filestat.st_size, metadata, cdmi_size, Int);
			if (JSON_HAS(metadata, "cdmi_size"))
			{
				if (metadata["cdmi_size"].isIntegral())
					filesize = metadata["cdmi_size"].asInt64();
				else
					filesize = _atoi64(metadata["cdmi_size"].asString().c_str());
			}
#elif defined ZQ_OS_LINUX

			if( !JSON_HAS(metadata, "cdmi_size"))
				return 0;
			const Json::Value& v = metadata["cdmi_size"];
			if( !v.isString())
				return 0;
			long long retValue = 0;
			sscanf(v.asString().c_str(), "%lld", &retValue);
			filesize	= retValue;
#endif//ZQ_OS
			return true;
		}

		///step1. read Metadata from content metadada file, 判断sys.state状态
		///if sys.state == ::TianShanIce::Storage::csInService, 则去从读取MainContent的Metadata
		///else return content metadada file中的metadata
		bool A3AquaBase::getMetadataInfo(const std::string& paid, const std::string& pid,  TianShanIce::Properties& metadata)
		{
			Json::Value mdMdValue;
			Json::Value resultValue;
			std::string cntStat;
			int statId = -1;
			int cdmiCode = 0;
			bool bRet = false;

			///step1. read metadata from main content file metadata if mdfile not exist or content status is inService
			metadata.clear();
			std::string strMainContentName = getMainFileName(paid, pid);
			Json::Value mainFileMdValue;
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMetadataInfo()read metadata from mainfile[%s]"),strMainContentName.c_str());

			bRet = getMainFileMetadata(strMainContentName, mainFileMdValue, cdmiCode);
			if(bRet && JSON_HAS(mainFileMdValue, "metadata"))
			{
				Json::Value cntMdValue = mainFileMdValue["metadata"];
				metadata.clear();
				TianShanIce::Properties contentMD;
				JsonValue2StringMap(cntMdValue, contentMD);
				aquaMD2contentMD(contentMD, metadata);
				if(metadata.size() > 0 && metadata.find("sys.Name") != metadata.end() && metadata.find("sys.State") != metadata.end() && metadata.find("sys.memberFileExts") != metadata.end())
				{
					_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMetadataInfo()read metadata count[%d] from mainfile[%s] successful"), metadata.size(), strMainContentName.c_str());
				}
				else
					bRet = false;
			}
			else
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3AquaBase, "getMetadataInfo()failed to read metadata from mainfile[%s]"), strMainContentName.c_str());
			}

			///step2. read metadata from content Metadata file
			if(!bRet)
			{
				cdmiCode = 0;
				metadata.clear();

				std::string strAquaContentMDName = getAquaContentMDName(paid, pid);
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMetadataInfo()read metadata from content.mdata file[%s]"),strAquaContentMDName.c_str());

				bRet= getMDFileMetadata(strAquaContentMDName, mdMdValue, cdmiCode);
				if(bRet)
				{
					JsonValue2StringMap(mdMdValue, metadata);

					if (JSON_HAS(mdMdValue, "sys.State"))
					{
						cntStat = mdMdValue["sys.State"].asString();
						statId =  (int)stateId(cntStat.c_str());
					}
					_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMetadataInfo()content[%s]stat[%s]"), getContentName(paid,pid).c_str(), cntStat.c_str());
				}
			}

			if(cdmiCode == CdmiFuseOps::cdmirc_NotFound)
				metadata["sys.State"] = "NotFound";

			return bRet;  
		}

		///get MainFile Metadata
		bool  A3AquaBase::getMainFileMetadata(const std::string& fileName, Json::Value& vMetadata, int& cdmiCode)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaBase, "getMainFileMetadata()file[%s]"), fileName.c_str());

			///step1. read metadata from content Metadata file
			CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(fileName, vMetadata);
			if(!CdmiRet_SUCC(retCode))
			{
				cdmiCode = retCode;
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaBase, "getMainFileMetadata()failed to get file[%s]with error[%d==>%s]"), fileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
				return false;
			}

			Json::FastWriter write;
			std::string strMD = write.write(vMetadata);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMainFileMetadata()file[%s]MD[%s]"), fileName.c_str(), strMD.c_str());
 
			return true; 
		}
		bool A3AquaBase::getMainFileMetadata(const std::string& paid, const std::string& pid, Json::Value& vMetadata, int& cdmiCode)
		{
             return getMainFileMetadata(getMainFileName(paid, pid),vMetadata, cdmiCode);
		}

		bool A3AquaBase::getMainFileMetadata(const std::string& fileName, TianShanIce::Properties& metadata, int& cdmiCode)
		{
			Json::Value vMetadata;
			if(getMainFileMetadata(fileName,vMetadata, cdmiCode))
			{
				metadata.clear();
				TianShanIce::Properties contentMD;
				JsonValue2StringMap(vMetadata, contentMD);
				aquaMD2contentMD(contentMD, metadata);
				return true;
			}
			return false;
		}

		bool A3AquaBase::getMainFileMetadata(const std::string& paid, const std::string& pid, TianShanIce::Properties& metadata, int& cdmiCode)
		{
			return getMainFileMetadata(getMainFileName(paid, pid),metadata,cdmiCode);
		}

		///get content.mdata Metadata
		bool  A3AquaBase::getMDFileMetadata(const std::string& fileName, Json::Value& vMetadata, int& cdmiCode, int64 filesize)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaBase, "getMDFileMetadata()file[%s], filesize[%lld]"), fileName.c_str(), filesize);

			if(filesize < 0)
			{
				///step1. read metadata from content Metadata file
				Json::Value resultValue;
				CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(fileName, resultValue);
				if(!CdmiRet_SUCC(retCode))
				{
					cdmiCode = retCode;
					_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaBase, "getMDFileMetadata()failed to get file[%s]with error[%d==>%s]"), fileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
					return false;
				}
				else ///file exist
				{
					if (!JSON_HAS(resultValue, "metadata"))
					{
						_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaBase, "getMDFileMetadata()file[%s] no metadata received"), fileName.c_str());
						return false;
					}
					if (!getFileSize(resultValue["metadata"], filesize))
					{
						_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaBase, "getMDFileMetadata()file[%s] failed to get filesize"), fileName.c_str());
						return false;
					}
					_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMDFileMetadata()file[%s] get filesize[%lld]"), fileName.c_str(), filesize);
				}
			}

			bool bret = readFile(fileName, vMetadata, filesize);

			if(!bret)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaBase, "getMDFileMetadata()file[%s] failed to get content.mdata metadata"), fileName.c_str());
				return false;
			}

			Json::FastWriter write;
			std::string strMD = write.write(vMetadata);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "getMDFileMetadata()file[%s]MD[%s]"), fileName.c_str(), strMD.c_str());

			return true;
		}
		bool  A3AquaBase::getMDFileMetadata(const std::string& paid, const std::string& pid, Json::Value& vMetadata, int& cdmiCode, int64 filesize)
		{
            return getMDFileMetadata(getAquaContentMDName(paid, pid), vMetadata, cdmiCode, filesize);
		}
		bool  A3AquaBase::getMDFileMetadata(const std::string& fileName, TianShanIce::Properties& metadata,int& cdmiCode, int64 filesize)
		{
			Json::Value mValue;
            if(!getMDFileMetadata(fileName, mValue, cdmiCode, filesize))
				return false;

			metadata.clear();
			JsonValue2StringMap(mValue, metadata);
			return true;
		}
		bool  A3AquaBase::getMDFileMetadata(const std::string& paid, const std::string& pid, TianShanIce::Properties& metadata, int& cdmiCode, int64 filesize)
		{
            return getMDFileMetadata(getAquaContentMDName(paid, pid), metadata,cdmiCode, filesize);
		}

		///update MainFileMetadata
		bool  A3AquaBase::updateMainFileMetadata(const std::string& fileName, const TianShanIce::Properties& metadata)
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "updateMainFileMetadata()file[%s]"), fileName.c_str());

			TianShanIce::Properties aquaMetadata;
			contentMD2aquaMD(metadata, aquaMetadata);

			std::string location;
			std::string uri = _cdmiFuseOps.pathToUri(fileName) +"?metadata";
			CdmiFuseOps::CdmiRetCode retCode  = _cdmiFuseOps.cdmi_UpdateDataObject(location, uri, aquaMetadata, 0);
			if(!CdmiRet_SUCC(retCode))
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaBase, "updateMainFileMetadata()file[%s] failed to update metadata with error[%d==>%s]"), fileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
				return false;
			}
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaBase, "updateMainFileMetadata()file[%s] update metadata success"), fileName.c_str());
          return true;
		}
		bool  A3AquaBase::updateMainFileMetadata(const std::string& paid, const std::string& pid, const TianShanIce::Properties& metadata)
		{

			 return updateMainFileMetadata(getMainFileName(paid, pid), metadata);
		}

		const char*  A3AquaBase::stateStr(const ::TianShanIce::Storage::ContentState state)
		{
#define SWITCH_CASE_STATE(_ST)	case ::TianShanIce::Storage::cs##_ST: return #_ST
			switch(state)
			{
				SWITCH_CASE_STATE(NotProvisioned);
				SWITCH_CASE_STATE(Provisioning);
				SWITCH_CASE_STATE(ProvisioningStreamable);
				SWITCH_CASE_STATE(InService);
				SWITCH_CASE_STATE(OutService);
				SWITCH_CASE_STATE(Cleaning);
			default:
				return "<Unknown>";
			}
#undef SWITCH_CASE_STATE
		}

		::TianShanIce::Storage::ContentState  A3AquaBase::stateId(const char* stateStr)
		{
			if (NULL == stateStr)                                   return ::TianShanIce::Storage::csNotProvisioned;
			else if (0 == stricmp("NotProvisioned", stateStr))      return ::TianShanIce::Storage::csNotProvisioned;
			else if (0 == stricmp("Provisioning", stateStr))        return ::TianShanIce::Storage::csProvisioning;
			else if (0 == stricmp("ProvisioningStreamable", stateStr)) return ::TianShanIce::Storage::csProvisioningStreamable;
			else if (0 == stricmp("InService", stateStr))           return ::TianShanIce::Storage::csInService;
			else if (0 == stricmp("OutService", stateStr))          return ::TianShanIce::Storage::csOutService;
			else if (0 == stricmp("Cleaning", stateStr))            return ::TianShanIce::Storage::csCleaning;
			return ::TianShanIce::Storage::csNotProvisioned;
		}
		////////////////////////////////////////////////////////
		//////////////////////class A3AquaContentMetadata///////
		////////////////////////////////////////////////////////
		A3AquaContentMetadata::A3AquaContentMetadata(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps, const std::string& mainFileExtension,const std::string& contentNameFormat) :
								A3AquaBase(log, cdmiFuseOps,mainFileExtension,contentNameFormat)			
		{

		}

		A3AquaContentMetadata::~A3AquaContentMetadata(void)
		{

		}
		bool A3AquaContentMetadata::updateMetadata(const std::string& fileName, const TianShanIce::Properties& metadata)
		{
			Json::Value vMdata;
			StringMap2JsonValue(metadata, vMdata);
			return updateMetadata(fileName, vMdata);
		}
		bool A3AquaContentMetadata::updateMetadata(const std::string& paid, const std::string& pid, const TianShanIce::Properties& metadata)
		{
			Json::Value vMdata;
			StringMap2JsonValue(metadata, vMdata);
			return updateMetadata(getAquaContentMDName(paid, pid), vMdata);
		}
		bool A3AquaContentMetadata::updateMetadata(const std::string& paid, const std::string& pid, const Json::Value& vMetadata)
		{
			return updateMetadata(getAquaContentMDName(paid, pid), vMetadata);
		}

		bool A3AquaContentMetadata::updateMetadata(const std::string& fileName, const Json::Value& vMetadata)
		{
			Json::FastWriter writer;
			std::string strMetadata = writer.write(vMetadata);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3AquaContentMetadata, "updateMetadata()file[%s], metadata[%s]"), fileName.c_str(), strMetadata.c_str());

			///step1, read file content from aqua server
			///if file exist, update metadata to file
			///if file not exist, create file and update metadata to file.
			Json::Value resultValue;
			CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(fileName, resultValue);
			if(!CdmiRet_SUCC(retCode) && retCode != CdmiFuseOps::cdmirc_NotFound)
				return false;

			///file not exist
			if(retCode == CdmiFuseOps::cdmirc_NotFound)
			{	
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentMetadata, "updateMetadata()file[%s] not exist, create it"), fileName.c_str());
				Json::FastWriter write;
				std::string requestBody = write.write(vMetadata);

				return  CreateFile(fileName, (char*)requestBody.c_str(), (uint)requestBody.length());
			}
			else ///file exist
			{
				if (!JSON_HAS(resultValue, "metadata"))
				{
					_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaContentMetadata, "updateMetadata()ContentName[%s] no metadata received"), fileName.c_str());
					return false;
				}

				int64 filesize;
				if (!getFileSize(resultValue["metadata"], filesize))
				{
					_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3AquaContentMetadata, "updateMetadata()ContentName[%s] failed to get filesize"), fileName.c_str());
					return false;
				}

				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentMetadata, "updateMetadata()ContentName[%s] get filesize[%lld]"), fileName.c_str(), filesize);

				///MD文件中原有的数据 value, 用新的vMetadata更新原来的metadata
				Json::Value value;
				if(!readFile(fileName, value, filesize))
					return false;

				Json::Value::Members members = vMetadata.getMemberNames();
				Json::Value::Members::iterator itorMem= members.begin();
				for( ; itorMem != members.end(); ++itorMem)
				{
					std::string& key = *itorMem; 
					if (JSON_HAS(value, key))
						value.removeMember(key);

					value[key] = vMetadata[key].asString();
				}
				Json::FastWriter write;
				std::string buff = write.write(value);
				if(!updateFile(fileName, (char*)buff.c_str(), (uint)buff.size()))
					return false;
			}
			return true;
		}
		void A3AquaContentMetadata::testMD()
		{
			std::string pid = "xor";
			std::string paid = "1111";

			Json::Value value;
			value["sys.1"] = "11";
			value["sys.2"] = "22";
			value["sys.3.9"] = "33";
			value["user.e.9e"] = "33";
			value["key44554_eqfwejrj"] = "545";

			{
				TianShanIce::Properties metadata;
				JsonValue2StringMap(value, metadata);
                

				TianShanIce::Properties aquaMd;
			    contentMD2aquaMD(metadata,aquaMd);

				TianShanIce::Properties metadata1;
				aquaMD2contentMD(aquaMd, metadata1);
			}

			updateMetadata(getAquaContentMDName(paid, pid), value);
            
			
			Json::Value Value2;
			Value2["sys.4"] = "44";
			Value2["key.5"] = "55";
			Value2["key.6"] = "66";
			Value2["key.1"] = "1111";

			updateMetadata(getAquaContentMDName(paid, pid), Value2);
            

			Json::Value resultValue;
			int cdmiCode = 0;
			getMDFileMetadata(paid, pid, resultValue, cdmiCode);

			std::string buf = "test";
			CreateFile(getMainFileName(paid, pid),buf.c_str(), buf.size());

			TianShanIce::Properties metadata;
			JsonValue2StringMap(resultValue, metadata);

			{
				bool bret = updateMainFileMetadata(getMainFileName(paid, pid), metadata);
			}

			{
				Json::Value vMetadata;
				bool bret =getMainFileMetadata(getMainFileName(paid, pid), vMetadata, cdmiCode);
		    }

			value["key.6"] = "6666";
			value["key.7"] = "77";

			JsonValue2StringMap(value, metadata);

			{
				bool bret = updateMainFileMetadata(getMainFileName(paid, pid), metadata);
			}

			{
				Json::Value vMetadata;
				bool bret =getMainFileMetadata(getMainFileName(paid, pid), vMetadata, cdmiCode);
			}
		}
		////////////////////////////////////////////////////////
		//////////////////////class A3CPESessionMgr///////////////////
		////////////////////////////////////////////////////////
		A3CPESessionMgr::A3CPESessionMgr(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps) :
								A3AquaBase(log, cdmiFuseOps)
		{

		}

		A3CPESessionMgr::~A3CPESessionMgr(void)
		{

		}
		bool A3CPESessionMgr::addSession(const std::string& contentName, const std::string& cpeSessionProxy)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "addSession()ContentName[%s] CPESession[%s]"), contentName.c_str(), cpeSessionProxy.c_str());

			int64 lstart = ZQ::common::now();

			Json::Value resultValue;
			CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(strSessionListFile, resultValue);
			if(!CdmiRet_SUCC(retCode) && retCode != CdmiFuseOps::cdmirc_NotFound)
				return false;

			///file not exist
			if(retCode == CdmiFuseOps::cdmirc_NotFound)
			{	
				Json::Value value;
				value[contentName] = cpeSessionProxy;
				Json::FastWriter write;
				std::string requestBody = write.write(value);

				return  CreateFile(strSessionListFile, (char*)requestBody.c_str(),(uint)requestBody.length());
			}
			else ///file exist
			{
				if (!JSON_HAS(resultValue, "metadata"))
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "addSession()ContentName[%s] no metadata received"), contentName.c_str());
					return false;
				}

				int64 filesize;
				if (!getFileSize(resultValue["metadata"], filesize))
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "addSession()ContentName[%s] failed to get filesize"), contentName.c_str());
					return false;
				}

				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "addSession()ContentName[%s] get filesize[%lld]"), contentName.c_str(), filesize);

				Json::Value value;
				if(!readFile(strSessionListFile, value, filesize))
					return false;
                value[contentName] = cpeSessionProxy;

				Json::FastWriter write;
				std::string buff = write.write(value);
				if(!updateFile(strSessionListFile, (char*)buff.c_str(), (uint)buff.length()))
					return false;
			}

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3CPESessionMgr, "addSession()ContentName[%s] CPESession[%s] took %dms"), contentName.c_str(), cpeSessionProxy.c_str(), (int)(ZQ::common::now()  - lstart));

			return true;
		}
		bool A3CPESessionMgr::removeSession(const std::string& contentName)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "removeSession()ContentName[%s]"), contentName.c_str());

			int64 lstart = ZQ::common::now();

			Json::Value resultValue;
			CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(strSessionListFile, resultValue);
			if(!CdmiRet_SUCC(retCode) && retCode != CdmiFuseOps::cdmirc_NotFound)
				return false;

			if (!JSON_HAS(resultValue, "metadata"))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "removeSession()ContentName[%s] no metadata received"), contentName.c_str());
				return false;
			}

			int64 filesize;
			if (!getFileSize(resultValue["metadata"], filesize))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "removeSession()ContentName[%s] failed to get filesize"), contentName.c_str());
				return false;
			}

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "addSession()ContentName[%s] get filesize[%lld]"), contentName.c_str(), filesize);

			Json::Value value;
			if(!readFile(strSessionListFile, value, filesize))
				return false;
            
			///if has contentName key, remove it, and update cpesessionList on Aqua
			///else printf warning log
			if (!JSON_HAS(value, contentName))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "removeSession()ContentName[%s] no cpe provision session"), contentName.c_str());
			}
			else
			{
				std::string cpeSessionProxy = value[contentName].asString();;
				value.removeMember(contentName);
				Json::FastWriter write;
				std::string buff = write.write(value);

				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "removeSession()ContentName[%s] update sessionList[%s] to aqua server"), contentName.c_str(), buff.c_str());
				if(!updateFile(strSessionListFile, (char*)buff.c_str(), (uint)buff.length()))
					return false;
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3CPESessionMgr, "removeSession()ContentName[%s] CPESession[%s] took %dms"), contentName.c_str(), cpeSessionProxy.c_str(), (int)(ZQ::common::now()  - lstart));
			}
			return true;
		}
		bool A3CPESessionMgr::updateSessionList(TianShanIce::Properties& sessionLists)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "updateSessionList()FileName[%s]"), strSessionListFile.c_str());

			int64 lstart = ZQ::common::now();

			Json::Value resultValue;
			CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(strSessionListFile, resultValue);
			if(!CdmiRet_SUCC(retCode) && retCode != CdmiFuseOps::cdmirc_NotFound)
				return false;

		    Json::Value value;
			TianShanIce::Properties::iterator itor = sessionLists.begin();
			while(itor!= sessionLists.end())
			{
				value[itor->first] = itor->second;
				itor++;
			}

			Json::FastWriter write;
			std::string requestBody = write.write(value);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "updateSessionList()update sessionList[%s] to aqua server"), requestBody.c_str());

			///file not exist
			if(retCode == CdmiFuseOps::cdmirc_NotFound)
			{	
				CdmiFuseOps::CdmiRetCode retCode = CreateFile(strSessionListFile, (char*)requestBody.c_str(), (uint)requestBody.length());
				if(!CdmiRet_SUCC(retCode) && retCode != CdmiFuseOps::cdmirc_NotFound)
					return false;
			}
			else ///file exist
			{
				if(!updateFile(strSessionListFile, (char*)requestBody.c_str(), (uint)requestBody.length()))
					return false;
			}

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3CPESessionMgr, "updateSessionList()FileName[%s] took %dms"), strSessionListFile.c_str(), (int)(ZQ::common::now()  - lstart));
			return true;

		}
		bool A3CPESessionMgr::getSessionList(TianShanIce::Properties& sessionLists)
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "getSessionList()FileName[%s]"), strSessionListFile.c_str());

			sessionLists.clear();
			int64 lstart = ZQ::common::now();

			Json::Value resultValue;
			CdmiFuseOps::CdmiRetCode retCode = checkFileStatus(strSessionListFile, resultValue);
			if(!CdmiRet_SUCC(retCode))
				return false;

			if (!JSON_HAS(resultValue, "metadata"))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "getSessionList()FileName[%s]no metadata received"), strSessionListFile.c_str());
				return false;
			}

			int64 filesize;
			if (!getFileSize(resultValue["metadata"], filesize))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(A3CPESessionMgr, "getSessionList()FileName[%s]failed to get filesize"), strSessionListFile.c_str());
				return false;
			}

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CPESessionMgr, "getSessionList()FileName[%s]get filesize[%lld]"), strSessionListFile.c_str(), filesize);

			Json::Value value;
			if(!readFile(strSessionListFile, value, filesize))
				return false;

			Json::FastWriter write;
			std::string strSessionlist = write.write(value);

			_log(ZQ::common::Log::L_INFO, CLOGFMT(A3CPESessionMgr, "getSessionList() sessionlist[%s]"), strSessionlist.c_str());
			JsonValue2StringMap(value, sessionLists);
            return true;
		}

		std::string A3AquaBase::fixupContentName(const std::string& paid, const std::string& pid)
		{
			std::string contentName = _contentNameFormat;

			int npos = contentName.find("${PAID}");
			if(npos >= 0)
			{
				contentName.replace(npos, strlen("${PAID}"), paid);
			}
			npos = contentName.find("${PID}");
			if(npos >= 0)
			{
				contentName.replace(npos, strlen("${PID}"), pid);
			}
			return contentName;
		}
		////////////////////////////////////////////////////////
		//////////////////////class A3AquaContentStatus////////////
		////////////////////////////////////////////////////////
		A3AquaContentStatus::A3AquaContentStatus(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps, int existTimeout,  int  notexistTimeout ,size_t lruSize, const std::string& mainFileExtension,const std::string& contentNameFormat) :
		A3AquaBase(log, cdmiFuseOps,mainFileExtension,contentNameFormat), _existTimeout(existTimeout),_notexistTimeout(notexistTimeout)			
		{
			_contents.resize(lruSize);
			_nocontents.resize(lruSize);
		}

		A3AquaContentStatus::~A3AquaContentStatus(void)
		{

		}

		bool A3AquaContentStatus::getMetadataInfo(const std::string& paid, const std::string& pid,  TianShanIce::Properties& metadata)
		{
			ZQ::common::MutexGuard guard(_lockContents);
			std::string filename  = getMainFileName(paid, pid);

			ZQ::common::LRUMap<std::string, ContentProps>::iterator itornoContents = _nocontents.find(filename);
			if(itornoContents != _nocontents.end())
			{
				if(_nocontents[filename].expiredTime > ZQ::common::now())
				{
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentStatus, "getMetadataInfo()content[%s]paid[%s]pid[%s] not exist"), filename.c_str(), paid.c_str(), pid.c_str());
					metadata = _nocontents[filename].props;
					return false;
				}
				else
				{
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentStatus, "getMetadataInfo()remove content[%s]paid[%s]pid[%s] from nocontent map"), filename.c_str(), paid.c_str(), pid.c_str());
					_nocontents.erase(filename);
				}
			}

			ZQ::common::LRUMap<std::string, ContentProps>::iterator itorContents = _contents.find(filename);
			if(itorContents != _contents.end())
			{
				if(_contents[filename].expiredTime > ZQ::common::now())
				{
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentStatus, "getMetadataInfo()read content[%s]paid[%s]pid[%s] from content map"), filename.c_str(), paid.c_str(), pid.c_str());
					metadata = _contents[filename].props; 
					return true;
				}
				else
				{
					_contents.erase(filename);
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentStatus, "getMetadataInfo()remove content[%s]paid[%s]pid[%s] from content map"), filename.c_str(), paid.c_str(), pid.c_str());
				}
			}

			bool bRet = A3AquaBase::getMetadataInfo(paid, pid, metadata);
			if(bRet)
			{
				ContentProps cnt;
				cnt.expiredTime = ZQ::common::now() + _existTimeout * 1000;
				cnt.props = metadata;
				_contents[filename] = cnt;
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentStatus, "getMetadataInfo()add content[%s]paid[%s]pid[%s] to content map"), filename.c_str(), paid.c_str(), pid.c_str());
			}
			else
			{
				ContentProps cnt;
				cnt.expiredTime = ZQ::common::now() + _notexistTimeout* 1000;
				cnt.props = metadata;
				_nocontents[filename] = cnt;
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3AquaContentStatus, "getMetadataInfo()add content[%s]paid[%s]pid[%s] to nocontent map"), filename.c_str(), paid.c_str(), pid.c_str());
			}
			return bRet; 
		}

	}
}
