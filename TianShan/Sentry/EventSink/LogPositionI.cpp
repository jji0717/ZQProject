#include "LogPositionI.h"
#include <TimeUtil.h>

LogPositionFactory::LogPositionFactory()
{
}

Ice::ObjectPtr LogPositionFactory::create(const std::string& type)
{
	if( type == EventSink::LogPosition::ice_staticId() )
		return new LogPositionI();
	else
		return NULL;
}

void LogPositionFactory::destroy()
{
}


/////// class LogPositionI ////////
LogPositionI::LogPositionI()
{
    data.position = 0;
    data.stamp = 0;
    data.lastUpdated = 0;
}
std::string LogPositionI::getSourceName(const Ice::Current&) const {
	IceUtil::Mutex::Lock lock(*this);
    return data.source;
}
std::string LogPositionI::getHandlerName(const Ice::Current&) const {
    IceUtil::Mutex::Lock lock(*this);
    return data.handler;
}
Ice::Long LogPositionI::getLastUpdatedTime(const Ice::Current&) const {
    IceUtil::Mutex::Lock lock(*this);
    return data.lastUpdated;
}
void LogPositionI::getData(Ice::Long& position, Ice::Long& stamp, const Ice::Current&) const {
    IceUtil::Mutex::Lock lock(*this);
    position = data.position;
    stamp = data.stamp;
}
void LogPositionI::updateData(Ice::Long position, Ice::Long stamp, const Ice::Current&) {
    IceUtil::Mutex::Lock lock(*this);
    data.position = position;
    data.stamp = stamp;
    data.lastUpdated = ZQ::common::now();
}

#define EVENTSINK_RECORD_CATEGORY 	"LogPosition"
#define EVENTSINK_DEFAULT_ENDPOINT "tcp -h 127.0.0.1"

LogPositionDb::LogPositionDb(ZQ::common::Log& log)
    :log_(log) {
}

bool LogPositionDb::init(Ice::CommunicatorPtr communicator, const std::string& path, int32 evictorSize)
{
	if(path.empty())
		return false;

	FS::createDirectory(path.c_str(), true);

	try
	{
		communicator->addObjectFactory(new LogPositionFactory(), EventSink::LogPosition::ice_staticId());

		adapter_ = communicator->createObjectAdapterWithEndpoints(EVENTSINK_RECORD_CATEGORY, EVENTSINK_DEFAULT_ENDPOINT);

#if ICE_INT_VERSION / 100 >= 303
		evictor_ = Freeze::createBackgroundSaveEvictor(adapter_, path, EVENTSINK_RECORD_CATEGORY);
#else
		evictor_ = Freeze::createEvictor(adapter_, path, EVENTSINK_RECORD_CATEGORY);
#endif
		evictor_->setSize(evictorSize);

		adapter_->addServantLocator(evictor_, EVENTSINK_RECORD_CATEGORY);
		adapter_->activate();
	}
	catch(Ice::Exception& ex)
	{
		log_(ZQ::common::Log::L_ERROR, CLOGFMT(LogPositionDb, "init() got [%s]"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		log_(ZQ::common::Log::L_ERROR, CLOGFMT(LogPositionDb, "init() got unknown exception"));
		return false;
	}
	log_(ZQ::common::Log::L_INFO, CLOGFMT(LogPositionDb, "init() Position Db inited at %s with evictor size %d"), path.c_str(), evictorSize);
	return true;
}

void LogPositionDb::uninit() {
    try {
        evictor_ = 0;
        adapter_->deactivate();
    }
    catch(...){}
    log_(ZQ::common::Log::L_INFO, CLOGFMT(LogPositionDb, "uninit() Position Db uninited."));
}
PositionRecordPtr LogPositionDb::getPosition(const std::string& sourceName, const std::string handlerName) {
    ZQ::common::MutexGuard guard(lockPosCache_);
    PositionRecordCache::key_type key = make_pair(sourceName, handlerName);
    PositionRecordCache::iterator it = posCache_.find(key);
    if(it != posCache_.end()) {
        return it->second;
    } else { // query & save the cache
        std::pair<std::pair<std::string, std::string>, PositionRecordPtr> val;
        val.first = key;
        val.second = new PositionRecord(query(sourceName, handlerName));
        posCache_.insert(val);
        return val.second;
    }
}
EventSink::LogPositionPrx LogPositionDb::query(const std::string& sourceName, const std::string handlerName) {
    Freeze::EvictorIteratorPtr it = evictor_->getIterator("", evictor_->getSize());
    while (it && it->hasNext())
    {
        ::Ice::Identity ident = it->next();
        try {
            EventSink::LogPositionPrx pos = EventSink::LogPositionPrx::uncheckedCast(adapter_->createProxy(ident));
            if(pos->getSourceName() == sourceName &&
               pos->getHandlerName() == handlerName) {
                   log_(ZQ::common::Log::L_DEBUG, CLOGFMT(LogPositionDb, "query() got position record %s by source(%s), handler(%s)"), ident.name.c_str(), sourceName.c_str(), handlerName.c_str());
                return pos;
            }
        }
        catch(::Ice::Exception& ex)
        {
            log_(ZQ::common::Log::L_WARNING, CLOGFMT(LogPositionDb, "query() got [%s] when access position record %s"), ex.ice_name().c_str(), ident.name.c_str());
            continue;
        }
        catch(...)
        {
            log_(ZQ::common::Log::L_WARNING, CLOGFMT(LogPositionDb, "query() got unknown exception when access position record %s"), ident.name.c_str());
            continue;
        }
    }
    // not found, create new one
    LogPositionIPtr pos = new LogPositionI();
    pos->data.source = sourceName;
    pos->data.handler = handlerName;
    Ice::Identity ident;
    ident.name = IceUtil::generateUUID();
    ident.category = EVENTSINK_RECORD_CATEGORY;
    log_(ZQ::common::Log::L_INFO, CLOGFMT(LogPositionDb, "query() add position record %s with source(%s), handler(%s)"), ident.name.c_str(), sourceName.c_str(), handlerName.c_str());
    return EventSink::LogPositionPrx::checkedCast(evictor_->add(pos, ident));
}
void LogPositionDb::xmlDump(std::ostream& out) {
    out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    out << "<PositionDb>\n";
    Freeze::EvictorIteratorPtr it = evictor_->getIterator("", evictor_->getSize());
    std::vector<Ice::Identity> badData;
    while (it && it->hasNext())
    {
        ::Ice::Identity ident = it->next();
        try {
            EventSink::LogPositionPrx pos = EventSink::LogPositionPrx::uncheckedCast(adapter_->createProxy(ident));
            std::string srcName = pos->getSourceName();
            std::string handlerName = pos->getHandlerName();
            Ice::Long position = 0;

            Ice::Long stamp = 0;
            pos->getData(position, stamp);
            Ice::Long lastUpdated = pos->getLastUpdatedTime();
            char buf[64];
            std::string stampUTC;
            if(ZQ::common::TimeUtil::TimeToUTC(stamp, buf, 64)) {
                stampUTC = buf;
            }
            std::string lastUpdatedUTC;
            if(ZQ::common::TimeUtil::TimeToUTC(lastUpdated, buf, 64)) {
                lastUpdatedUTC = buf;
            }

            out << "  <Record>\n";
            out << "    <Source>" << srcName << "</Source>\n";
            out << "    <Handler>" << handlerName << "</Handler>\n";
            out << "    <Position>" << position << "</Position>\n";
            out << "    <Stamp>";
            if(!stampUTC.empty()) {
                out << stampUTC;
            } else {
                out << "Invalid(" << stamp << ")";
            }
            out << "</Stamp>\n";

            out << "    <LastUpdatedAt>";
            if(!lastUpdatedUTC.empty()) {
                out << lastUpdatedUTC;
            } else {
                out << "Invalid(" << lastUpdated << ")";
            }
            out << "</LastUpdatedAt>\n"
                << "  </Record>\n";
        }
        catch(::Ice::Exception &)
        {
            badData.push_back(ident);
            continue;
        }
        catch(...)
        {
            badData.push_back(ident);
            continue;
        }
    }
    if(!badData.empty()) {
        out << "  <BadData>\n";
        for(size_t i = 0; i < badData.size(); ++i) {
            out << "    <Identity>" << badData[i].name << "</Identity\n";
        }
        out << "  </BadData>\n";
    }

    out << "</PositionDb>\n";
}
