#ifndef __FILE_WRITER_CONFIG__
#define __FILE_WRITER_CONFIG__

#include <vector>
#include <map>
#include <string>
#include "ConfigHelper.h"

#define __BEGIN_FILE_WRITER \
namespace EventGateway { \
namespace FileWriter {

#define __END_FILE_WRITER }}

using namespace ZQ::common::Config;

__BEGIN_FILE_WRITER

struct Line  {
    std::string text;
    static void structure(Holder<Line> &holder) {
        holder.addDetail("", "text", &Line::text, NULL, optReadOnly);
    }
};

typedef std::vector<struct Line> Lines;

struct Event {
    std::string category;
    std::string name;
    Lines lines;

    std::string path;
    static void structure(ZQ::common::Config::Holder<Event> &holder) {
        holder.addDetail("", "category", &Event::category, NULL, optReadOnly);
        holder.addDetail("", "name", &Event::name, NULL, optReadOnly);
        holder.addDetail("Line", &Event::readLines, &Event::registerNothing);
    }
    void readLines(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        ZQ::common::Config::Holder<Line> line;
        line.read(node, hPP);
        lines.push_back(line);
    }
    void registerNothing(const std::string&){
    }

    bool operator< (const Event& rhs) const {
        if(category == rhs.category) {
            return (name < rhs.name);
        }
        else {
            return (category < rhs.category);
        }
    }
};

typedef std::vector<struct Event> Events;

struct OutputFile {
    std::string path;
    Events events;

    static void structure(ZQ::common::Config::Holder<OutputFile> &holder) {
        holder.addDetail("", "path", &OutputFile::path, NULL, optReadOnly);
        holder.addDetail("Event", &OutputFile::readEvents, &OutputFile::registerNothing);
    }
    void readEvents(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        ZQ::common::Config::Holder<Event> event;
        event.read(node, hPP);
        event.path = path;
        events.push_back(event);
    }
    void registerNothing(const std::string&){
    }
};

typedef std::vector<OutputFile> OutputFiles;

struct FileWriterConfig {
    OutputFiles files; 
    int32 logLevel;
    int32 logSize;

    int32 rotateCount;
    int32 rotateSize;
    int32 rotateStart;

    static void structure(ZQ::common::Config::Holder<FileWriterConfig> &holder) {
        holder.addDetail("Log", "level", &FileWriterConfig::logLevel, NULL, optReadOnly);
        holder.addDetail("Log", "size", &FileWriterConfig::logSize, NULL, optReadOnly);
        holder.addDetail("OutputFile", &FileWriterConfig::readFiles, &FileWriterConfig::registerNothing);
        holder.addDetail("Rotate", "count", &FileWriterConfig::rotateCount, NULL, optReadOnly);
        holder.addDetail("Rotate", "size", &FileWriterConfig::rotateSize, NULL, optReadOnly);
        holder.addDetail("Rotate", "start", &FileWriterConfig::rotateStart, NULL, optReadOnly);
    }
    void readFiles(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        ZQ::common::Config::Holder<OutputFile> file;
        file.read(node, hPP);
        files.push_back(file);
    }
    void registerNothing(const std::string&){
    }
};


__END_FILE_WRITER

#endif // __FILE_WRITER_CONFIG__
