#include "CloneIndexFile.h"
#include <fstream>
#include "SimpleXMLParser.h"

using ZQ::IdxParser::CsicoIndexFileParser;

namespace ZQ
{
namespace IdxParser
{

IndexFileClone::IndexFileClone(ZQ::common::Log& fileLog, const std::string oriIndexFile, const std::string newIndexFile)
:_oriIndexFile(oriIndexFile), _newIndexFile(newIndexFile), _fileLog(fileLog)
{

}

IndexFileClone::~IndexFileClone()
{

}

bool IndexFileClone::replaceSubFile(const std::string oriSubFile, const std::string newSubFile)
{
	if (oriSubFile.size() != newSubFile.size())
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileClone, "replaceSubFile() : New index file[%s]'s length is unequal to Original index file[%s]'s length"), newSubFile.c_str(), oriSubFile.c_str());
		return false;
	}
	StringMap::iterator strIter = _subFiles.find(oriSubFile);
	if (strIter == _subFiles.end())
	{
		_subFiles.insert(std::make_pair(oriSubFile, newSubFile)); // insert 
	}
	else
	{
		strIter->second = newSubFile; // replace
	}
	return true;
}

bool IndexFileClone::clone()
{
	// open new index file 
	std::ofstream fileOut;
	fileOut.open(_newIndexFile.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!fileOut)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileClone, "clone() : Fail to open new index file [%s] to write"), _newIndexFile.c_str());
		return false;
	}
	// open original index file 
	IdxParserEnv env;
	FileReaderNative reader(env);
	if (!reader.open(_oriIndexFile))
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileClone, "clone() : Fail to open original index file [%s] to read"), _oriIndexFile.c_str());
		fileOut.close();
		return false;
	}
	if (reader.getFileSize(_oriIndexFile, true) <= 0)
	{
		fileOut.close();
		return true;
	}
	if (_subFiles.empty())
	{
		readRestFile(&reader, fileOut);
	}
	else
	{
		cloneCiscoIndex(&reader, fileOut);
	}

	// close original and new index file
	fileOut.flush();
	fileOut.close();
	reader.close();
	_subFiles.clear();
	return true;
}

void IndexFileClone::cloneCiscoIndex(FileReader* reader, std::ofstream& fileOut)
{
	// copy file header section
	CsicoIndexFileParser::CsicoIndexFileHeader fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));
	int32 nRead = reader->read(&fileHeader, sizeof(fileHeader));
	fileOut.write(reinterpret_cast<char*>(&fileHeader), nRead);
	if(nRead != (int32)sizeof(fileHeader))
	{
		// file is end
		_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"Failed to read CsicoIndexFileHeader for file[%s]"), _oriIndexFile.c_str());
	}
	else
	{
		parseCsicoINDEX11(reader, fileOut);
	}
}

#define	CSICOINDEX_TAG_SECTIONHEADERTAG								0x11000001
#define CSICOINDEX_TAG_SUBFILESECTION								0x110005FF
#define CSICOINDEX_TAG_SUBFILENAME									0x15000502

bool IndexFileClone::parseOtherSection(ZQ::IdxParser::FileReader* reader, std::ofstream& fileOut, uint16 sectionLength)
{
	char* buf = new char[sectionLength];
	int32 nRead = reader->read(buf, sectionLength);
	fileOut.write(buf, nRead);
	delete [] buf;
	if (nRead != sectionLength)
	{
		return false;
	}
	return true;
}

void IndexFileClone::parseCsicoINDEX11(ZQ::IdxParser::FileReader *reader, std::ofstream &fileOut)
{
	CsicoIndexFileParser::CsicoIndexFileTagAndLength tal;
	bool bQuit = false;
	while (!bQuit)
	{
		if (getTag(reader, fileOut, tal))
		{
			try
			{
				switch (tal.tagId)
				{
				case CSICOINDEX_TAG_SECTIONHEADERTAG: // index header section
					parseSection(reader, fileOut, tal.length);
					readRestFile(reader, fileOut);
					bQuit = true;
					break;
				default: // other sections between file header section and index header section 
					if (!parseOtherSection(reader, fileOut, tal.length))
					{
						bQuit = true;
					}
					break;
				}
			}
			catch (const char* reason)
			{
				_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse[%s] because [%s]"), _oriIndexFile.c_str(), reason);
				bQuit = true;
			}
		}
		else
		{
			_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"), _oriIndexFile.c_str());
			bQuit = true;
		}
	}
}

bool IndexFileClone::getTag(FileReader* reader, std::ofstream& fileOut,
							CsicoIndexFileParser::CsicoIndexFileTagAndLength& tal)
{
	int32 nRead = reader->read(&tal, sizeof(tal));
	fileOut.write(reinterpret_cast<char*>(&tal), nRead);
	if (nRead != sizeof(tal))
	{
		// file is end;
		_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),_oriIndexFile.c_str());
		return false;
	}
	while (*((char*)&tal) == 0x00)
	{
		CsicoIndexFileParser::CsicoIndexFileTagAndLength tmpTal;
		unsigned char c;
		nRead = reader->read(&c, sizeof(c));
		fileOut.write(reinterpret_cast<char*>(&c), nRead);
		if (nRead != sizeof(c))
		{
			// file is end
			_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),_oriIndexFile.c_str());
			return false;
		}
		char* pTemp = (char*)&tal;
		pTemp ++;
		memcpy(&tmpTal, pTemp, sizeof(tal) -1);
		pTemp = (char*)&tmpTal;
		pTemp += sizeof(tmpTal) - 1;
		memcpy(pTemp, &c, sizeof(c));
		memcpy(&tal, &tmpTal, sizeof(tal));
	}
	return true;
}

void IndexFileClone::parseSection(ZQ::IdxParser::FileReader* reader, std::ofstream& fileOut, uint16 sectionLength)
{
	CsicoIndexFileParser::CsicoIndexFileTagAndLength tal;
	bool bQuit = false;
	uint16 curLength = 0;
	while (!bQuit && !_subFiles.empty() && curLength < sectionLength)
	{
		if (getTag(reader, fileOut, tal))
		{
			switch (tal.tagId)
			{
			case CSICOINDEX_TAG_SUBFILESECTION:
				if (!parseSubFiles(reader, fileOut, tal.length))
				{
					bQuit = true;
				}
				break;
			default:
				if (!parseOtherSection(reader, fileOut, tal.length))
				{
					bQuit = true;
				}
				break;
			} // end for switch
		}
		else
		{
			bQuit = true;
			_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),_oriIndexFile.c_str());
		}
		curLength += tal.length;
	} // end for while
}

void IndexFileClone::readRestFile(ZQ::IdxParser::FileReader* reader, std::ofstream& fileOut)
{
	char buffer[2048];
	int32 nRead;
	do 
	{
		nRead = reader->read(buffer, sizeof(buffer));
		fileOut.write(buffer, nRead);
	} while (nRead == sizeof(buffer));
}

bool IndexFileClone::parseSubFiles(ZQ::IdxParser::FileReader* reader, std::ofstream& fileOut, uint16 sectionLength)
{
	CsicoIndexFileParser::CsicoIndexFileTagAndLength tal;
	bool bQuit = false;
	uint16 curLength = 0;
	int i = 0;
	while(!bQuit && curLength < sectionLength)
	{
		if (getTag(reader, fileOut, tal))
		{
			switch (tal.tagId)
			{
			case CSICOINDEX_TAG_SUBFILENAME:
				replaceSubFileName(reader, fileOut, tal.length);
				bQuit = true;
				break;
			default:
				if (!parseOtherSection(reader, fileOut, tal.length))
				{
					bQuit = true;
				}
				break;
			}
		}
		else
		{
			bQuit = true;
			_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),_oriIndexFile.c_str());
		}
		curLength += tal.length;
	}
	return true;
}

void IndexFileClone::replaceSubFileName(ZQ::IdxParser::FileReader* reader, std::ofstream& fileOut, uint16 sectionLength)
{
	std::string strXmlContent;
	strXmlContent = readString(reader, sectionLength);
	SimpleXMLParser parser;
	std::string xmlContent = "<root>" ;
	xmlContent = xmlContent + strXmlContent;
	xmlContent = xmlContent + "</root>";
	try
	{
		parser.parse( xmlContent.c_str() ,static_cast<int>( xmlContent.length() ), 1 );
		const SimpleXMLParser::Node& root = parser.document();
		const SimpleXMLParser::Node* pSubType = findNode(&root,"root/SubType");
		if(pSubType)
		{
			StringMap::iterator strIter = _subFiles.find(pSubType->content);
			if (strIter != _subFiles.end())
			{
				size_t iterStart = strXmlContent.find(pSubType->content);
				strXmlContent.replace(iterStart, pSubType->content.size(), strIter->second);
				_subFiles.erase(strIter);
			}
		}
		else
		{
			_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse subfile extension for [%s] with [%s] , can't get subtype"), _oriIndexFile.c_str() ,xmlContent.c_str() );
		}
	}
	catch( ZQ::common::ExpatException& )
	{
		_fileLog(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse subfile extension for [%s] with [%s]"), _oriIndexFile.c_str() ,xmlContent.c_str() );
	}
	fileOut.write(strXmlContent.c_str(), strXmlContent.length() + 1);
}

std::string	IndexFileClone::readString( FileReader* reader , size_t size )
{
	char* buf = new char[size];
	int32 nRead = reader->read(buf, size);
	std::string strContent = buf;
	delete [] buf;
	return strContent;
}


} // end for IndexClone
} // end for ZQ