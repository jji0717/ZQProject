#ifndef __FW_EVENT_HELPER__
#define __FW_EVENT_HELPER__

#include "EventGwHelper.h"
#include "FileWriterConfig.h"
#include "EventDispatcher.h"

__BEGIN_FILE_WRITER

class EventHelper : public EventGateway::IGenericEventHelper {

public:
    typedef std::multimap<Event, WorkerThread*> WorkerMap;
    
    EventHelper(ZQ::common::Log&);
    virtual ~EventHelper(); 

    bool init();

    virtual void onEvent(
         const std::string& category,
         Ice::Int eventId,
         const std::string& eventName,
         const std::string& stampUTC,
         const std::string& sourceNetId,
         const Properties& params
    );

private:
    ZQ::common::Log& _log;
    WorkerMap _workers;

};

__END_FILE_WRITER

#endif //__FW_ EVENT_HELPER__

