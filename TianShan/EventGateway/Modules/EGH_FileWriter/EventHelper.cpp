#include "EventHelper.h"
#include "EventDispatcher.h"
#include "FileWriterConfig.h"
#include "Text.h"

extern ZQ::common::Config::Loader<EventGateway::FileWriter::FileWriterConfig> fwConfig;

__BEGIN_FILE_WRITER

EventHelper::EventHelper(ZQ::common::Log& logger):_log(logger) {
}

EventHelper::~EventHelper() {
    WorkerMap::const_iterator iter = _workers.begin();
    for(; iter != _workers.end(); ++iter) {
        iter->second->stop();
    }
}

bool EventHelper::init() {
    /* populate worker map */
    OutputFiles::const_iterator iter = fwConfig.files.begin();
    for(; iter != fwConfig.files.end(); ++iter) {
        WorkerThread* worker = new WorkerThread(iter->path, _log);

        Events::const_iterator iter2 = iter->events.begin();
        for(; iter2 != iter->events.end(); ++iter2) {
            _workers.insert(std::make_pair(*iter2, worker));
        }
        worker->start();
    }
    return true;
}

void EventHelper::onEvent(
              const std::string& category,
              Ice::Int eventId,
              const std::string& eventName,
              const std::string& stampUTC,
              const std::string& sourceNetId,
              const Properties& params) {

    Event event;
    event.name = eventName;
    event.category = category;

    WorkerMap::size_type entries = _workers.count(event);
    if(!entries) {
        return;
    }

    _log(ZQ::common::Log::L_INFO, CLOGFMT(EventHelper, "EVENT [%s - %s] received"), category.c_str(), eventName.c_str()); 

    std::map<std::string, std::string> ctx = params;
    ctx["Category"] = category;
    ctx["EventName"] = eventName;
    ctx["StampUTC"] = stampUTC;
    ctx["SourceNetId"] = sourceNetId;

    std::ostringstream oss;
    oss << eventId;
    ctx["EventId"] = oss.str();

    WorkerMap::const_iterator iter1 = _workers.find(event);
    for(WorkerMap::size_type cnt = 0; cnt < entries; ++cnt, ++iter1) {
        Lines formated;

        /* format every line of event */
        Lines::const_iterator iter2 = iter1->first.lines.begin();
        for(; iter2 != iter1->first.lines.end(); ++iter2) {
            Line plain;
            try {
                plain.text = ZQ::common::Text::format(iter2->text, ctx);
            } catch(const ZQ::common::Text::FormattingException& e) {
                _log(ZQ::common::Log::L_ERROR, "onEvent() FormmatingException caught during generating the messsage: %s", e.getString());
                return;
            }
            formated.push_back(plain);
        }
        
        /* send lines to worker */
        iter1->second->addLines(formated);
    }


}

__END_FILE_WRITER
