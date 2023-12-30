#include "adiXMLDoc.h"

ADIXMLDoc::ADIXMLDoc(::ZQ::common::FileLog *logFile)
:_pLog(logFile)
{

}

ADIXMLDoc::ADIXMLDoc(::ZQ::common::FileLog *logFile, const ::std::string &strADITemplate)
:_pLog(logFile)
,_strADITemplate(strADITemplate)
{

}

ADIXMLDoc::~ADIXMLDoc()
{
	_pLog = NULL;
}

bool ADIXMLDoc::fixup(::TianShanIce::Properties &prop, ::std::string &strADI)
{
	strADI = _strADITemplate;
	for (::TianShanIce::Properties::iterator iter = prop.begin(); iter != prop.end(); iter++)
	{
		//fixup macro
		::std::string strMacro = "${" + iter->first + "}";
		::std::string::size_type pos_macro_begin = ::std::string::npos;
		while((pos_macro_begin = strADI.find_first_of(strMacro)) != ::std::string::npos)
		{
			 strADI.replace(pos_macro_begin, strMacro.size(), iter->second);
		}
	}
	if (strADI.find("${") != std::string::npos)
		return false;
	else
		return true;
}

size_t ADIXMLDoc::CreateADI(FILE *adiFile, ADI &adi)
{
	if (NULL == adiFile)
	{
		ADILOG(ZQ::common::Log::L_ERROR, CLOGFMT(ADIXMLDoc, "CreateADI: ADI file is null"));
		return false;
	}

	size_t iFileSize = 0;
	::std::string strADI;
	iFileSize += setXMLFirstLine(strADI);
	iFileSize += setXMLDTDLine(strADI);

	//set begin of ADI layer
	iFileSize += writeLine(strADI, BeginOfADI.c_str(), BeginOfADI.length());

	/////////////////////<MetaData>////////////////////
	iFileSize += setMetaData(strADI, adi._MetaData);
	///////////////////////////////////////////////////

	for (ADIAsset::ADIAssetList::iterator iter = adi._Asset.begin(); iter != adi._Asset.end(); iter++)
		iFileSize += setAsset(strADI, *iter);

	//set end of ADI layer
	iFileSize += writeLine(strADI, EndOfADI.c_str(), EndOfADI.length());
	fwrite(strADI.c_str(), strADI.length(), sizeof(char), adiFile);
	return iFileSize;
}

size_t ADIXMLDoc::setXMLFirstLine(::std::string &buf)
{
	buf += XMLFirstLine;
	return XMLFirstLine.length();
}


size_t ADIXMLDoc::setXMLDTDLine(::std::string &buf)
{
	buf += XMLDTDLine;
	return XMLDTDLine.length();
}

size_t ADIXMLDoc::writeLine(::std::string &buf, const char *line, size_t size)
{
	buf += ::std::string(line, size);
	return size;
}

size_t ADIXMLDoc::setMetaData(::std::string &buf, ADIMetaData &metaData)
{
	//set begin of MetaData layer
	size_t iFileSize = 0;
	iFileSize += writeLine(buf, BeginOfMetaData.c_str(), BeginOfMetaData.length());

	//set AMS property
	iFileSize += setAMS(buf, metaData._AMS);

	//set APP_Data(if has)
	iFileSize += setAppDataList(buf, metaData._ADIAppDataList);

	//set end of MetaData layer
	iFileSize += writeLine(buf, EndOfMetaData.c_str(), EndOfMetaData.length());

	return iFileSize;
}

size_t ADIXMLDoc::setAMS(::std::string &buf, AMS &ams)
{
	::std::string strAMS = ::std::string("<AMS ");
	strAMS += AssetName + "\"" + ams._strAsset_Name + "\" ";
	strAMS += Asset_ID + "\"" + ams._strAsset_ID + "\" ";
	strAMS += Asset_Class + "\"" + ams._strAsset_Class + "\" ";
	strAMS += Provider + "\"" + ams._strProvider + "\" ";
	strAMS += Provider_ID + "\"" + ams._strProvider_ID + "\" ";
	strAMS += Product + "\"" + ams._strProduct + "\" ";
	strAMS += Version_Minor + "\"" + ams._strVersion_Minor + "\" ";
	strAMS += Version_Major + "\"" + ams._strVersion_Major + "\" ";
	strAMS += Description + "\"" + ams._strDescription + "\" ";
	strAMS += Creation_Date + "\"" + ams._strCreation_Date + "\" ";
	if (!ams._strVerb.empty())
		strAMS += Verb + "\"" + ams._strVerb + "\" ";
	strAMS += EndSymbol;

	return writeLine(buf, strAMS.c_str(), strAMS.length());
}

size_t ADIXMLDoc::setAppDataList(::std::string &buf, ADIAppDataList &appDataList)
{
	if (appDataList.empty())
		return 0;
	else
	{
		size_t iFileSize = 0;
		for (ADIAppDataList::iterator iter = appDataList.begin(); iter != appDataList.end(); iter++)
			iFileSize += setAppData(buf, *iter);
		return iFileSize;
	}
}

size_t ADIXMLDoc::setAppData(::std::string &buf, ADIAppData &appData)
{
	::std::string strAppData = "<App_Data ";
	strAppData += "App=\"" + appData._strApp + "\" ";
	strAppData += "Name=\"" + appData._strName + "\" ";
	strAppData += "Value=\"" + appData._strValue+ "\" ";
	strAppData += EndSymbol;
	buf += strAppData;
	return strAppData.length();
}

size_t ADIXMLDoc::setAsset(::std::string &buf, ADIAsset &asset)
{
	size_t iFileSize = 0;
	::std::string strAsset = "<Asset>\r\n";

	iFileSize += setMetaData(strAsset, asset._MetaData);

	for (ADIAsset::ADIAssetList::iterator iter = asset._Asset.begin(); iter != asset._Asset.end(); iter++)
		iFileSize += setAsset(strAsset, *iter);

	iFileSize += setContent(strAsset, asset._Content);

	strAsset += "</Asset>\r\n";

	buf += strAsset;
	return strAsset.length();
}

size_t ADIXMLDoc::setContent(::std::string &buf, ADIContentList &content)
{
	size_t iFileSize = 0;
	if (content.empty())
		return iFileSize;

	::std::string strContent;
	for (ADIContentList::iterator iter = content.begin(); iter != content.end(); iter++)
		 strContent += "<Content value=\"" + iter->_strValue + "\"" + EndSymbol;
	
	buf += strContent;
	iFileSize = strContent.length();
	return iFileSize;
}