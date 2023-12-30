#ifndef __ADIXMLDOC_H__
#define __ADIXMLDOC_H__

#include "ZQ_common_conf.h"
#include "TianShanIce.h"
#include "FileLog.h"
#include <iostream>
#include <list>

#define ADILOG if (_pLog) (*_pLog)

static ::std::string AssetName = "Asset_Name=";
static ::std::string Asset_ID = "Asset_ID=";
static ::std::string Asset_Class = "Asset_Class=";
static ::std::string Provider = "Provider=";
static ::std::string Provider_ID = "Provider_ID=";
static ::std::string Product = "Product=";
static ::std::string Version_Minor = "Version_Minor=";
static ::std::string Version_Major = "Version_Major=";
static ::std::string Description = "Description=";
static ::std::string Creation_Date = "Creation_Date=";
static ::std::string Verb = "Verb=";

typedef struct AMS
{
	::std::string _strAsset_Name;
	::std::string _strAsset_ID;
	::std::string _strAsset_Class;
	::std::string _strProvider;
	::std::string _strProvider_ID;
	::std::string _strProduct;
	::std::string _strVersion_Minor;
	::std::string _strVersion_Major;
	::std::string _strDescription;
	::std::string _strCreation_Date;
	::std::string _strVerb;
}AMS;

typedef struct ADIAppData
{
	::std::string _strApp;
	::std::string _strName;
	::std::string _strValue;
}App_Data;
typedef ::std::list<ADIAppData> ADIAppDataList;

typedef struct ADIContent
{
	::std::string _strValue;
}ADIContent;
typedef ::std::list<ADIContent> ADIContentList;

typedef struct ADIMetaData
{
	AMS				_AMS;
	ADIAppDataList	_ADIAppDataList;	//0 or more
}ADIMetaData;

typedef struct ADIAsset
{
typedef ::std::list<ADIAsset> ADIAssetList;
	ADIMetaData		_MetaData;
	ADIAssetList	_Asset;		//0 or more
	ADIContentList	_Content;	//1 or more
}ADIAsset;

typedef struct ADI
{
	ADIMetaData				_MetaData;
	ADIAsset::ADIAssetList	_Asset;		//0 or more
}ADI;

static ::std::string XMLFirstLine = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n";
static ::std::string XMLDTDLine = "<!DOCTYPE ADI SYSTEM 'ADI.DTD'>\r\n";
static ::std::string BeginOfADI = "<ADI>\r\n";
static ::std::string EndOfADI = "</ADI>\r\n";
static ::std::string BeginOfMetaData = "<MetaData>\r\n";
static ::std::string EndOfMetaData = "</MetaData>\r\n";
static ::std::string EndSymbol = "/>\r\n";

class ADIXMLDoc
{
public:
	ADIXMLDoc(::ZQ::common::FileLog *logFile);
	ADIXMLDoc(::ZQ::common::FileLog *logFile, const ::std::string &strADITemplate);
	~ADIXMLDoc();

	inline void setADITemplate(const ::std::string &strADITemplate)
	{
		_strADITemplate = strADITemplate;
	}
	bool fixup(::TianShanIce::Properties &prop, ::std::string &strADI);
	size_t CreateADI(FILE *adiFile, ADI &adi);	//return file size
protected:
private:
	::ZQ::common::Log *_pLog;
	::std::string _strADITemplate;

	size_t setXMLFirstLine(::std::string &buf);
	size_t setXMLDTDLine(::std::string &buf);
	size_t writeLine(::std::string &buf, const char *line, size_t size);

	size_t setMetaData(::std::string &buf, ADIMetaData &metaData);

	size_t setAMS(::std::string &buf, AMS &ams);
	size_t setAppDataList(::std::string &bufe, ADIAppDataList &appData);
	size_t setAppData(::std::string &buf, ADIAppData &appData);

	size_t setAsset(::std::string &buf, ADIAsset &asset);
	size_t setContent(::std::string &buf, ADIContentList &content);
};
#endif __ADIXMLDOC_H__