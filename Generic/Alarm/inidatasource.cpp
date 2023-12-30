
#include "inidatasource.h"
#include "ini.h"
#include <exception>

TG_BEGIN

INIDataSource::INIDataSource(const char* inifile)
{
	if (NULL == inifile)
		throw std::exception("[INIDataSource::INIDataSource] ini file name is empty");

	m_source.inifile = inifile;

	IniFile ini(inifile);

	//get total count of triggered log file
	std::string logcount = ini.ReadKey(MAIN_SECTION, LOG_COUNT_KEY);
	int nLogCount = atoi(logcount.c_str());
	if (0 == nLogCount)
		throw std::exception("[INIDataSource::INIDataSource] ini file is empty");

	for (int i = 0; i < nLogCount; ++i)
	{
		LogBlock logblock;

		//get the group section name
		char groupsecname[MAX_INI_SECTION_LENGTH] = {0};
		_snprintf(groupsecname, MAX_INI_SECTION_LENGTH-1, "Group_%03d", i);

		std::string sectionname		= ini.ReadKey(groupsecname, SECTION_NAME_KEY);
		std::string loglocation		= ini.ReadKey(groupsecname, LOG_LOCATION_KEY);
		std::string triggernumber	= ini.ReadKey(groupsecname, TRIGGER_NUMBER_KEY);
		int nTriggerNumber = atoi(triggernumber.c_str());
		if (0 == nTriggerNumber)
			continue;

		logblock.filename = loglocation;

		for (int j = 0; j < nTriggerNumber; ++j)
		{
			//get the syntax section name
			char syntaxname[MAX_INI_SECTION_LENGTH] = {0};
			_snprintf(syntaxname, MAX_INI_SECTION_LENGTH-1,
				"%s_%03d", sectionname.c_str(), j);

			DataBlock datablock;

			datablock.syntax = ini.ReadKey(syntaxname, SYNTAX_KEY);

			std::vector<std::string> keylist;
			if (!ini.EnumKey(syntaxname, keylist))
			{
				//warning here
				continue;
			}

			for (size_t k = 0; k < keylist.size(); ++k)
			{
				size_t keyPos = keylist[k].find('=');
				std::string key = keylist[k].substr(0, keyPos);
				std::string value = keylist[k].substr(keyPos+1);
				datablock.kvpair[key] = value;
			}

			logblock.trigger.push_back(datablock);
		}

		m_source.log.push_back(logblock);
	}
}

size_t INIDataSource::countSource()
{
	return m_source.log.size();
}

std::string INIDataSource::listSource(size_t sourceid)
{
	return m_source.log[sourceid].filename;
}

size_t INIDataSource::countSyntax(size_t sourceid)
{
	return m_source.log[sourceid].trigger.size();
}

std::string INIDataSource::listSyntax(size_t sourceid, size_t syntaxid)
{
	return m_source.log[sourceid].trigger[syntaxid].syntax;
}

size_t INIDataSource::countData(size_t sourceid , size_t syntaxid)
{
	return m_source.log[sourceid].trigger[syntaxid].kvpair.size();
}

std::string INIDataSource::listDataKey(size_t sourceid , size_t syntaxid, size_t dataid)
{
	std::map<std::string, std::string>::iterator itor = 
		m_source.log[sourceid].trigger[syntaxid].kvpair.begin();

	for (size_t i = 0; i < dataid; ++i)
		++itor;

	return itor->first;
}

std::string INIDataSource::getDataValue(size_t sourceid , size_t syntaxid, const char* datakey)
{
	return m_source.log[sourceid].trigger[syntaxid].kvpair[datakey];
}

TG_END
