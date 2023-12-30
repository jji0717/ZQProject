#ifndef _VSTRM_INDEX_FILE_CLONE_H__
#define _VSTRM_INDEX_FILE_CLONE_H__

#include <IndexFileParser.h>
#include <map>

namespace ZQ
{
namespace IdxParser
{

class IndexFileClone
{
public:
	IndexFileClone(ZQ::common::Log& fileLog, const std::string oriIndexFile, const std::string newIndexFile);
	~IndexFileClone();
public:
	bool replaceSubFile(const std::string oriSubFile, const std::string newSubFile);

	/// @return, false only if fail to open original index file to read  or open new index file to write
	bool clone();

protected:

	void parseCsicoINDEX11(FileReader* reader, std::ofstream& fileOut);

	bool parseSubFiles(FileReader* reader, std::ofstream& fileOut, uint16 sectionLength);

	void readRestFile(FileReader* reader, std::ofstream& fileOut);

	void parseSection(FileReader* reader, std::ofstream& fileOut, uint16 sectionLength);

	bool parseOtherSection(FileReader* reader, std::ofstream& fileOut, uint16 sectionLength);

	bool getTag(FileReader* reader, std::ofstream& fileOut, CsicoIndexFileParser::CsicoIndexFileTagAndLength& tal);

	void replaceSubFileName(FileReader* reader, std::ofstream& fileOut, uint16 sectionLength);

	std::string readString(FileReader* reader, size_t size);

	void cloneCiscoIndex(FileReader* reader, std::ofstream& fileOut);

private:
	IndexFileClone(const IndexFileClone& oriClone);
	IndexFileClone& operator=(const IndexFileClone& oriClone);

private:
	typedef std::map<std::string, std::string> StringMap;
	StringMap _subFiles;
	std::string _oriIndexFile;
	std::string _newIndexFile;
	ZQ::common::Log& _fileLog;
};

} // end for IdxParser
} // end for ZQ

#endif // end for _VSTRM_INDEX_FILE_CLONE_H__