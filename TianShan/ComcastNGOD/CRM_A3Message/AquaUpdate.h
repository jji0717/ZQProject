#ifndef __ZQ_CRM_A3MESSAGE_AQUAUpdate_H_
#define __ZQ_CRM_A3MESSAGE_AQUAUpdate_H_
#include "CdmiFuseOps.h"
#include "TianShanIce.h"
#include "TsStorage.h"

namespace CRM
{
	namespace A3Message
	{
		const std::string strMainFileExtension = ".0X0000" ;
		const std::string strMdataExtension = ".mdata";
		const std::string strSessionListFile = "A3SessionList.bin";
		const std::string strContentNameFormat="${PAID}_${PID}";

		///////////////////// class A3AquaBase////////////////////////////////////
		class A3AquaBase : virtual public ZQ::common::SharedObject
		{
		public:
			A3AquaBase(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps, const std::string& mainFileExtension = strMainFileExtension, const std::string& contentNameFormat = strContentNameFormat);

			virtual ~A3AquaBase();

			typedef ZQ::common::Pointer < A3AquaBase > Ptr;
		public: 
			///create file, write value to file
			bool CreateFile(const std::string& fileName, const char* value, uint len);

			///update file, write value to buf from startoffset=0, and filesize = 0
			bool updateFile(const std::string& fileName, const char* value, uint len);

			///read file from startoffset=0 and length=len to buf
			bool readFile(const std::string& fileName,  Json::Value& value, int64& filesize);

			///read file metadata, write to value
			bool readFileMetadata(const std::string& fileName, Json::Value& value);

			///delete file
			bool deleteFile(const std::string& fileName);
		protected:
			bool getFileSize(const Json::Value& metadata, int64& filesize);

		protected:
			CdmiFuseOps& _cdmiFuseOps;
			ZQ::common::Log& _log;
			std::string _mainFileExtension;
			std::string _contentNameFormat;

		public:

			CdmiFuseOps::CdmiRetCode checkFileStatus(const std::string& fileName, Json::Value& value);

			/// return main content filename which exist in Aqua Server, format: Paid + Pid + mainFileExtension
			std::string getMainFileName(const std::string& paid, const std::string& pid)
			{
				return fixupContentName(paid,pid) + _mainFileExtension;
			}

			/// return content MetaData filename which exist in Aqua Server,format:  Paid + Pid + MdataExtension
			std::string getAquaContentMDName(const std::string& paid, const std::string& pid)
			{
				return fixupContentName(paid,pid) + strMdataExtension;
			}

			/// return content filename which exist in CPE, format: Paid + Pid
			std::string getContentName(const std::string& paid, const std::string& pid)
			{
				return fixupContentName(paid, pid);
			}
			std::string getMainFileExtension()
			{
				return _mainFileExtension;
			}
			std::string getAquaMdataFileExtension()
			{
				return strMdataExtension;
			}
			///for CDNSS, read metadata from Auqa Server
			///@param[in]	paid	content Provider asset Id
			///@param[in]	pid		content Provider Id
			///@param[out]	props   content metadata
			virtual bool getMetadataInfo(const std::string& paid, const std::string& pid,  TianShanIce::Properties& metadata);

			///get MainFile Metadata
			bool  getMainFileMetadata(const std::string& fileName, Json::Value& vMetadata, int& cdmiCode);
			bool  getMainFileMetadata(const std::string& paid, const std::string& pid, Json::Value& vMetadata, int& cdmiCode);
			bool  getMainFileMetadata(const std::string& fileName, TianShanIce::Properties& metadata ,int& cdmiCode);
			bool  getMainFileMetadata(const std::string& paid, const std::string& pid, TianShanIce::Properties& metadata,int& cdmiCode);

			///get content.mdata Metadata
			bool  getMDFileMetadata(const std::string& fileName, Json::Value& vMetadata,  int& cdmiCode, int64 filesize = -1);
			bool  getMDFileMetadata(const std::string& paid, const std::string& pid, Json::Value& vMetadata,  int& cdmiCode, int64 filesize = -1);
			bool  getMDFileMetadata(const std::string& fileName, TianShanIce::Properties& metadata,  int& cdmiCode, int64 filesize = -1);
			bool  getMDFileMetadata(const std::string& paid, const std::string& pid, TianShanIce::Properties& metadata,  int& cdmiCode, int64 filesize = -1);

			///update MainFileMetadata
			bool  updateMainFileMetadata(const std::string& fileName, const TianShanIce::Properties& metadata);
			bool  updateMainFileMetadata(const std::string& paid, const std::string& pid, const TianShanIce::Properties& metadata);
		public:
			/// convert contentState to string
			static const char* stateStr(const ::TianShanIce::Storage::ContentState state);

			/// convert string to contentState
			static TianShanIce::Storage::ContentState stateId(const char* stateStr);
		private:
			std::string fixupContentName(const std::string& paid, const std::string& pid);

		};

		///////////////////// class A3AquaContentMetadata////////////////////////////////////
		class A3AquaContentMetadata: public A3AquaBase
		{
		public:
			A3AquaContentMetadata(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps, const std::string& mainFileExtension = strMainFileExtension,const std::string& contentNameFormat = strContentNameFormat);
			virtual ~A3AquaContentMetadata();

			typedef ZQ::common::Pointer < A3AquaContentMetadata > Ptr;
		public:
			bool updateMetadata(const std::string& fileName, const Json::Value& vMetadata);
			bool updateMetadata(const std::string& fileName, const TianShanIce::Properties& metadata);

			bool updateMetadata(const std::string& paid, const std::string& pid, const Json::Value& vMetadata);
			bool updateMetadata(const std::string& paid, const std::string& pid, const TianShanIce::Properties& metadata);
		public:
			void testMD();
		};


		///////////////////// class A3CPESessionMgr////////////////////////////////////
		class A3CPESessionMgr: public A3AquaBase
		{
		public:
			A3CPESessionMgr(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps);
			virtual ~A3CPESessionMgr();
			typedef ZQ::common::Pointer < A3CPESessionMgr > Ptr;
		public:
			bool addSession(const std::string& contentName, const std::string& cpeSessionProxy);
			bool removeSession(const std::string& contentName);
			bool getSessionList(TianShanIce::Properties& sessionLists);
			bool updateSessionList(TianShanIce::Properties& sessionLists);
		};

		///////////////////// class A3AquaFileStatus////////////////////////////////////
		class A3AquaContentStatus: public A3AquaBase
		{
		public:
			A3AquaContentStatus(ZQ::common::Log& log, CdmiFuseOps& cdmiFuseOps, int existTimeout= 30, int  notexistTimeout=30 , size_t lruSize= 500, const std::string& mainFileExtension = strMainFileExtension,const std::string& contentNameFormat = strContentNameFormat);
			virtual ~A3AquaContentStatus();

			typedef ZQ::common::Pointer < A3AquaContentStatus > Ptr;
		public:
			virtual bool getMetadataInfo(const std::string& paid, const std::string& pid,  TianShanIce::Properties& metadata);
		private:
			typedef struct 
			{
				int64 expiredTime;
				TianShanIce::Properties props;
			}ContentProps;
			ZQ::common::LRUMap<std::string, ContentProps> _contents; //map to filename with exist contents
			ZQ::common::LRUMap<std::string, ContentProps> _nocontents; //map to filename with not exist contents
			ZQ::common::Mutex _lockContents;

			int _existTimeout;
			int  _notexistTimeout;
		};

	}
}
#endif//__ZQ_CRM_A3MESSAGE_AQUAUpdate_H_