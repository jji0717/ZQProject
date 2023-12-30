#include "BasePage.h"
//#include "atlstr.h"

#define LOG_MODULE_NAME         "BasePage"

namespace ErmWebPage
{

BasePage::BasePage(IHttpRequestCtx* pHttpRequestCtx) : 
_reqCtx(pHttpRequestCtx), 
_ic(NULL), 
_ERM(NULL)
{
	_lastError.clear();
	memset(szBuf, 0, sizeof(szBuf));
}

BasePage::~BasePage()
{
	if (_ic != NULL)
		try {_ic->destroy();} catch (...){}
	_ic = NULL;
}

const char* BasePage::getLastError() const
{
	return _lastError.c_str();
}

void BasePage::setLastError(const char* error)
{
	if (NULL != error)
		_lastError = error;
}

void BasePage::addToLastError(const char* error)
{
	if (NULL != error)
	{
		// 累加错误信息
		_lastError += error;
	}
}

bool BasePage::process()
{
	bool bRet = false;

	try
	{
		if (NULL == _reqCtx)
		{
			setLastError("Http request context pointer is null");
			glog(EmergLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			return false;
		}
		IHttpResponse& responser = _reqCtx->Response();
		_varMap = _reqCtx->GetRequestVars();

		if (_varMap[PublisherKey].empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "endpoint not configured");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		std::string endpoint = _varMap[PublisherKey];
		snprintf(szBuf, sizeof(szBuf) - 1, "<p>%s</p>", endpoint.c_str());
		responser<<szBuf;

		try
		{
			int i=0;
			_ic = Ice::initialize(i, NULL);
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "init ice environment caught %s", ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		try
		{
			_ERM = TianShanIce::EdgeResource::EdgeResouceManagerPrx::checkedCast(_ic->stringToProxy(endpoint));
		}
		catch (const Ice::ObjectNotExistException&)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "EdgeResourceManager not started or endpoint is wrong");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Edge Resource Manager proxy caught %s", ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(BasePage, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		if (isPostBack())
			bRet = post();
		else 
			bRet = get();
	}
	catch (...)
	{
		setLastError("caught global unexpect exception");
		glog(EmergLog, CLOGFMT(LOG_MODULE_NAME, "%s"), getLastError());
		IHttpResponse& responser = _reqCtx->Response();
		responser.SetLastError(getLastError());
	}

	return bRet;
}

bool BasePage::isPostBack() const
{
	return (_reqCtx->GetMethodType() == M_POST) ? true : false;
}

void BasePage::splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter)
{
	using namespace std;
	result.clear();
	string::size_type pos_from = 0;
	while((pos_from = str.find_first_not_of(delimiter, pos_from)) != string::npos)
	{
		string::size_type pos_to = str.find_first_of(delimiter, pos_from);
		if(pos_to != string::npos)
		{
			result.push_back(str.substr(pos_from, pos_to - pos_from));
		}
		else
		{
			result.push_back(str.substr(pos_from));
			break;
		}
		pos_from = pos_to;
	}
}

std::string BasePage::neighborLayout(const std::string& currentTemplate, const std::string& neighborLayoutId)
{
	std::string::size_type pos = currentTemplate.rfind(FNSEPC);
	return (pos == std::string::npos ? neighborLayoutId : currentTemplate.substr(0, pos + 1) + neighborLayoutId);
}
/*
bool BasePage::readChannels(ChannelInfos& channelInfos, string path)
{
	CString sectionName = "";
	CString keyName;
	CString defaultStr = "None";
	CString filename;
	filename.Format("%s", path.c_str());
	TCHAR      chSectionNames[2048]={0};      //所有节名组成的字符数组
	char * pSectionName; //保存找到的某个节名字符串的首地址
	int i;      //i指向数组chSectionNames的某个位置，从0开始，顺序后移
	int j=0;     //j用来保存下一个节名字符串的首地址相对于当前i的位置偏移量
	int count=0;     //统计节的个数

	//CString name;
	//char id[20];
	::GetPrivateProfileSectionNames(chSectionNames, 2048, filename);   
	for(i=0;i<2048;i++,j++)
	{
		if(chSectionNames[0]=='\0')
			break;      //如果第一个字符就是0，则说明ini中一个节也没有
		if(chSectionNames[i]=='\0')
		{
			pSectionName=&chSectionNames[i-j]; //找到一个0，则说明从这个字符往前，减掉j个偏移量，
			//就是一个节名的首地址

			j=-1;        //找到一个节名后，j的值要还原，以统计下一个节名地址的偏移量
			//赋成-1是因为节名字符串的最后一个字符0是终止符，不能作为节名的一部分

			char buf[255];
			ChannelInfo channelInfo;
			channelInfo.name = pSectionName;
			GetPrivateProfileString(pSectionName, "adminState", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.adminState = buf;

			GetPrivateProfileString(pSectionName, "RF", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.RF = atof(buf);

			GetPrivateProfileString(pSectionName, "powerLevel", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.powerLevel = atoi(buf);

			GetPrivateProfileString(pSectionName, "modulation", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.modulation = buf;

			GetPrivateProfileString(pSectionName, "level", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.level = buf;

			GetPrivateProfileString(pSectionName, "mode", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.mode = buf;

			channelInfo.TSID = GetPrivateProfileInt(pSectionName, "TSID", 0, filename);

			channelInfo.PAT = GetPrivateProfileInt(pSectionName, "PAT", 0, filename);

			channelInfo.PMT = GetPrivateProfileInt(pSectionName, "PMT", 0, filename);

			channelInfos.push_back(channelInfo);

//			AfxMessageBox(pSectionName);     //把找到的显示出来

			if(chSectionNames[i+1]==0)
			{
				break;     //当两个相邻的字符都是0时，则所有的节名都已找到，循环终止
			}
		}   

	}

	return count;
}

bool BasePage::writeChannels(ChannelInfos& channelInfos, string path)
{
	CString sectionName = "";
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// see if the file exists, if not then call the Write() method
	HANDLE fileHdl;
	fileHdl = CreateFile( CompleteFilename,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (fileHdl == INVALID_HANDLE_VALUE)
	{
	}
	else
	{
		CloseHandle(fileHdl);
	}

	for(ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
	{
		ChannelInfo channelInfo = *it;

		sectionName.Format("%s", channelInfo.name.c_str());

		// write out adminState
		keyName = "adminState";
		value.Format("%s", channelInfo.adminState.c_str());
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out RF
		keyName = "RF";
		value.Format("%.1f", channelInfo.RF);
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out powerLevel
		keyName = "powerLevel";
		value.Format("%d", channelInfo.powerLevel);
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out modulation
		keyName = "modulation";
		value.Format("%s", channelInfo.modulation.c_str());
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out level 
		keyName = "level";
		value.Format("%s", channelInfo.level.c_str());
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out ProvisionErrorMsg 
		keyName = "mode";
		value.Format("%s", channelInfo.mode.c_str());
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out ProvisionErrorMsg 
		keyName = "TSID";
		value.Format("%d", channelInfo.TSID);
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out ProvisionErrorMsg 
		keyName = "PAT";
		value.Format("%d", channelInfo.PAT);
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);

		// write out ProvisionErrorMsg 
		keyName = "PMT";
		value.Format("%d", channelInfo.PMT);
		WritePrivateProfileString(sectionName,
			keyName,
			value,
			CompleteFilename);
	}

	return true;
}

bool BasePage::appendChannel(ChannelInfo& channelInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", channelInfo.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out adminState
	keyName = "adminState";
	value.Format("%s", channelInfo.adminState.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out RF
	keyName = "RF";
	value.Format("%.1f", channelInfo.RF);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out powerLevel
	keyName = "powerLevel";
	value.Format("%d", channelInfo.powerLevel);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out modulation
	keyName = "modulation";
	value.Format("%s", channelInfo.modulation.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out level 
	keyName = "level";
	value.Format("%s", channelInfo.level.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "mode";
	value.Format("%s", channelInfo.mode.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "TSID";
	value.Format("%d", channelInfo.TSID);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PAT";
	value.Format("%d", channelInfo.PAT);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PMT";
	value.Format("%d", channelInfo.PMT);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::modifyChannel(ChannelInfo& channelInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", channelInfo.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out adminState
	keyName = "adminState";
	value.Format("%s", channelInfo.adminState.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out RF
	keyName = "RF";
	value.Format("%.1f", channelInfo.RF);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out powerLevel
	keyName = "powerLevel";
	value.Format("%d", channelInfo.powerLevel);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out modulation
	keyName = "modulation";
	value.Format("%s", channelInfo.modulation.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out level 
	keyName = "level";
	value.Format("%s", channelInfo.level.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "mode";
	value.Format("%s", channelInfo.mode.c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "TSID";
	value.Format("%d", channelInfo.TSID);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PAT";
	value.Format("%d", channelInfo.PAT);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PMT";
	value.Format("%d", channelInfo.PMT);
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}
*/
/*
bool BasePage::readChannels(TianShanIce::EdgeResource::EdgeChannelInfos& channelInfos, string path)
{
	CString sectionName = "";
	CString keyName;
	CString defaultStr = "None";
	CString filename;
	filename.Format("%s", path.c_str());
	TCHAR      chSectionNames[2048]={0};      //所有节名组成的字符数组
	char * pSectionName; //保存找到的某个节名字符串的首地址
	int i;      //i指向数组chSectionNames的某个位置，从0开始，顺序后移
	int j=0;     //j用来保存下一个节名字符串的首地址相对于当前i的位置偏移量
	int count=0;     //统计节的个数

	//CString name;
	//char id[20];
	::GetPrivateProfileSectionNames(chSectionNames, 2048, filename);   
	for(i=0;i<2048;i++,j++)
	{
		if(chSectionNames[0]=='\0')
			break;      //如果第一个字符就是0，则说明ini中一个节也没有
		if(chSectionNames[i]=='\0')
		{
			pSectionName=&chSectionNames[i-j]; //找到一个0，则说明从这个字符往前，减掉j个偏移量，
			//就是一个节名的首地址

			j=-1;        //找到一个节名后，j的值要还原，以统计下一个节名地址的偏移量
			//赋成-1是因为节名字符串的最后一个字符0是终止符，不能作为节名的一部分

			char buf[255];
			EdgeChannelInfo channelInfo;
			channelInfo.ident.name = pSectionName;

			GetPrivateProfileString(pSectionName, "qam", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Qam] = buf;

			GetPrivateProfileString(pSectionName, "adminState", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Admin] = buf;

			GetPrivateProfileString(pSectionName, "RF", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Freq] = buf;

			GetPrivateProfileString(pSectionName, "powerLevel", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Power] = buf;

			GetPrivateProfileString(pSectionName, "modulation", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Modulation] = buf;

			GetPrivateProfileString(pSectionName, "level", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Level] = buf;

			GetPrivateProfileString(pSectionName, "mode", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[Mode] = buf;

			GetPrivateProfileString(pSectionName, "TSID", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[TSID] = buf;

			GetPrivateProfileString(pSectionName, "PAT", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[PAT_Interval] = buf;

			GetPrivateProfileString(pSectionName, "PMT", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			channelInfo.props[PMT_Interval] = buf;

			channelInfos.push_back(channelInfo);

			if(chSectionNames[i+1]==0)
			{
				break;     //当两个相邻的字符都是0时，则所有的节名都已找到，循环终止
			}
		}   

	}

	return count;
}

bool BasePage::appendChannel(EdgeChannelInfo& channelInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", channelInfo.ident.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());


	// write out QAM
	keyName = "qam";
	value.Format("%s", channelInfo.props[Qam].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out adminState
	keyName = "adminState";
	value.Format("%s", channelInfo.props[Admin].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out RF
	keyName = "RF";
	value.Format("%s", channelInfo.props[Freq].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out powerLevel
	keyName = "powerLevel";
	value.Format("%s", channelInfo.props[Power].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out modulation
	keyName = "modulation";
	value.Format("%s", channelInfo.props[Modulation].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out level 
	keyName = "level";
	value.Format("%s", channelInfo.props[Level].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "mode";
	value.Format("%s", channelInfo.props[Mode].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "TSID";
	value.Format("%s", channelInfo.props[TSID].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PAT";
	value.Format("%s", channelInfo.props[PAT_Interval].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PMT";
	value.Format("%s", channelInfo.props[PMT_Interval].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::modifyChannel(EdgeChannelInfo& channelInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", channelInfo.ident.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out QAM
	keyName = "qam";
	value.Format("%s", channelInfo.props[Qam].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out adminState
	keyName = "adminState";
	value.Format("%s", channelInfo.props[Admin].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out RF
	keyName = "RF";
	value.Format("%s", channelInfo.props[Freq].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out powerLevel
	keyName = "powerLevel";
	value.Format("%s", channelInfo.props[Power].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out modulation
	keyName = "modulation";
	value.Format("%s", channelInfo.props[Modulation].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out level 
	keyName = "level";
	value.Format("%s", channelInfo.props[Level].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "mode";
	value.Format("%s", channelInfo.props[Mode].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "TSID";
	value.Format("%s", channelInfo.props[TSID].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PAT";
	value.Format("%s", channelInfo.props[PAT_Interval].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out ProvisionErrorMsg 
	keyName = "PMT";
	value.Format("%s", channelInfo.props[PMT_Interval].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::removeChannel(string channelName, string path)
{
	CString sectionName;
	sectionName.Format("%s", channelName.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	WritePrivateProfileString(sectionName, NULL, NULL, CompleteFilename);

	return true;
}

bool BasePage::readDevices(TianShanIce::EdgeResource::EdgeDeviceInfos& deviceInfos, string path)
{
	CString sectionName = "";
	CString keyName;
	CString defaultStr = "None";
	CString filename;
	filename.Format("%s", path.c_str());
	TCHAR      chSectionNames[2048]={0};      //所有节名组成的字符数组
	char * pSectionName; //保存找到的某个节名字符串的首地址
	int i;      //i指向数组chSectionNames的某个位置，从0开始，顺序后移
	int j=0;     //j用来保存下一个节名字符串的首地址相对于当前i的位置偏移量
	int count=0;     //统计节的个数

	//CString name;
	//char id[20];
	::GetPrivateProfileSectionNames(chSectionNames, 2048, filename);   
	for(i=0;i<2048;i++,j++)
	{
		if(chSectionNames[0]=='\0')
			break;      //如果第一个字符就是0，则说明ini中一个节也没有
		if(chSectionNames[i]=='\0')
		{
			pSectionName=&chSectionNames[i-j]; //找到一个0，则说明从这个字符往前，减掉j个偏移量，
			//就是一个节名的首地址

			j=-1;        //找到一个节名后，j的值要还原，以统计下一个节名地址的偏移量
			//赋成-1是因为节名字符串的最后一个字符0是终止符，不能作为节名的一部分

			char buf[255];
			EdgeDeviceInfo deviceInfo;
			deviceInfo.ident.name = pSectionName;
			GetPrivateProfileString(pSectionName, "Zone", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			deviceInfo.props[Zone] = buf;

			GetPrivateProfileString(pSectionName, "Vendor", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			deviceInfo.props[Vendor] = buf;

			GetPrivateProfileString(pSectionName, "Model", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			deviceInfo.props[Model] = buf;

			GetPrivateProfileString(pSectionName, "Description", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			deviceInfo.props[Description] = buf;

			GetPrivateProfileString(pSectionName, "TFTP", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			deviceInfo.props[TFTP] = buf;

			GetPrivateProfileString(pSectionName, "Telnet", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			deviceInfo.props[AdminUrl] = buf;

			deviceInfos.push_back(deviceInfo);

			if(chSectionNames[i+1]==0)
			{
				break;     //当两个相邻的字符都是0时，则所有的节名都已找到，循环终止
			}
		}   

	}

	return count;
}

bool BasePage::appendDevice(EdgeDeviceInfo& deviceInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", deviceInfo.ident.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out Zone
	keyName = "Zone";
	value.Format("%s", deviceInfo.props[Zone].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Vendor
	keyName = "Vendor";
	value.Format("%s", deviceInfo.props[Vendor].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Model
	keyName = "Model";
	value.Format("%s", deviceInfo.props[Model].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Description 
	keyName = "Description";
	value.Format("%s", deviceInfo.props[Description].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out TFTP 
	keyName = "TFTP";
	value.Format("%s", deviceInfo.props[TFTP].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Telnet 
	keyName = "Telnet";
	value.Format("%s", deviceInfo.props[AdminUrl].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::modifyDevice(EdgeDeviceInfo& deviceInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", deviceInfo.ident.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out Zone
	keyName = "Zone";
	value.Format("%s", deviceInfo.props[Zone].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Vendor
	keyName = "Vendor";
	value.Format("%s", deviceInfo.props[Vendor].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Model
	keyName = "Model";
	value.Format("%s", deviceInfo.props[Model].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Description 
	keyName = "Description";
	value.Format("%s", deviceInfo.props[Description].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out TFTP 
	keyName = "TFTP";
	value.Format("%s", deviceInfo.props[TFTP].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Telnet 
	keyName = "Telnet";
	value.Format("%s", deviceInfo.props[AdminUrl].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::removeDevice(string deviceName, string path)
{
	CString sectionName;
	sectionName.Format("%s", deviceName.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	WritePrivateProfileString(sectionName, NULL, NULL, CompleteFilename);

	return true;
}

bool BasePage::readAllocations(TianShanIce::EdgeResource::AllocationInfos& allocationInfos, string path)
{
	CString sectionName = "";
	CString keyName;
	CString defaultStr = "None";
	CString filename;
	filename.Format("%s", path.c_str());
	TCHAR      chSectionNames[2048]={0};      //所有节名组成的字符数组
	char * pSectionName; //保存找到的某个节名字符串的首地址
	int i;      //i指向数组chSectionNames的某个位置，从0开始，顺序后移
	int j=0;     //j用来保存下一个节名字符串的首地址相对于当前i的位置偏移量
	int count=0;     //统计节的个数

	//CString name;
	//char id[20];
	::GetPrivateProfileSectionNames(chSectionNames, 2048, filename);   
	for(i=0;i<2048;i++,j++)
	{
		if(chSectionNames[0]=='\0')
			break;      //如果第一个字符就是0，则说明ini中一个节也没有
		if(chSectionNames[i]=='\0')
		{
			pSectionName=&chSectionNames[i-j]; //找到一个0，则说明从这个字符往前，减掉j个偏移量，
			//就是一个节名的首地址

			j=-1;        //找到一个节名后，j的值要还原，以统计下一个节名地址的偏移量
			//赋成-1是因为节名字符串的最后一个字符0是终止符，不能作为节名的一部分

			char buf[255];
			AllocationInfo allocationInfo;
			allocationInfo.ident.name = pSectionName;
			GetPrivateProfileString(pSectionName, "Address", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[Address] = buf;

			GetPrivateProfileString(pSectionName, "UDP", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[UDP] = buf;

			GetPrivateProfileString(pSectionName, "Qam", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[Qam] = buf;

			GetPrivateProfileString(pSectionName, "SourceIP", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[SourceIP] = buf;

			GetPrivateProfileString(pSectionName, "BandWidth", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[BandWidth] = buf;

			GetPrivateProfileString(pSectionName, "Status", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[Status] = buf;

			GetPrivateProfileString(pSectionName, "StartTime", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[StartTime] = buf;

			GetPrivateProfileString(pSectionName, "MaximumJitter", defaultStr, buf, 255, filename);
			if (!strcmp(buf, defaultStr))
			{
				continue;
			}
			allocationInfo.props[MaximumJitter] = buf;

			allocationInfos.push_back(allocationInfo);

			if(chSectionNames[i+1]==0)
			{
				break;     //当两个相邻的字符都是0时，则所有的节名都已找到，循环终止
			}
		}   

	}

	return count;
}

bool BasePage::appendAllocation(AllocationInfo& allocationInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", allocationInfo.ident.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out Video Module Address
	keyName = "Address";
	value.Format("%s", allocationInfo.props[Address].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out UDP port
	keyName = "UDP";
	value.Format("%s", allocationInfo.props[UDP].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out output QAM
	keyName = "Qam";
	value.Format("%s", allocationInfo.props[Qam].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out SourceIP
	keyName = "SourceIP";
	value.Format("%s", allocationInfo.props[SourceIP].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out bandwidth 
	keyName = "BandWidth";
	value.Format("%s", allocationInfo.props[BandWidth].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Status 
	keyName = "Status";
	value.Format("%s", allocationInfo.props[Status].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out StartTime 
	keyName = "StartTime";
	value.Format("%s", allocationInfo.props[StartTime].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Maximum Jitter 
	keyName = "MaximumJitter";
	value.Format("%s", allocationInfo.props[MaximumJitter].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::modifyAllocation(AllocationInfo& allocationInfo, string path)
{
	CString sectionName;
	sectionName.Format("%s", allocationInfo.ident.name.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	// write out Video Module Address
	keyName = "Address";
	value.Format("%s", allocationInfo.props[Address].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out UDP port
	keyName = "UDP";
	value.Format("%s", allocationInfo.props[UDP].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out output QAM
	keyName = "Qam";
	value.Format("%s", allocationInfo.props[Qam].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out SourceIP
	keyName = "SourceIP";
	value.Format("%s", allocationInfo.props[SourceIP].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out bandwidth 
	keyName = "BandWidth";
	value.Format("%s", allocationInfo.props[BandWidth].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Status 
	keyName = "Status";
	value.Format("%s", allocationInfo.props[Status].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out StartTime 
	keyName = "StartTime";
	value.Format("%s", allocationInfo.props[StartTime].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	// write out Maximum Jitter 
	keyName = "MaximumJitter";
	value.Format("%s", allocationInfo.props[MaximumJitter].c_str());
	WritePrivateProfileString(sectionName,
		keyName,
		value,
		CompleteFilename);

	return true;
}

bool BasePage::removeAllocation(string allocationName, string path)
{
	CString sectionName;
	sectionName.Format("%s", allocationName.c_str());
	CString keyName;
	CString value;
	CString CompleteFilename;
	CompleteFilename.Format("%s", path.c_str());

	WritePrivateProfileString(sectionName, NULL, NULL, CompleteFilename);

	return true;
}
*/
}