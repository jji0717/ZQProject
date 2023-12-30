#include "SenderManager.h"
#include "MessageSenderPump.h"
#include "LogPositionI.h"

SenderManager::SenderManager(ZQ::common::Log& log,const std::string& configPath,Ice::CommunicatorPtr comm,const std::string& posDbPath,int posDbEvictorSize)
				:_log(log)
				,_posDb(NULL)
				,_pSenderPump(NULL)
{
	_pSinkConf = new ZQ::common::Config::Loader< EventSinkConf >("");
	_pSinkConf->setLogger(&log);
	if(!_pSinkConf->load(configPath.c_str()))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(SenderManager, "Failed to load config file:%s"),configPath.c_str());
		throw ZQ::common::Exception(std::string("Failed to load config file: ") + configPath);
	}

	_pSenderPump = new MessageSenderPump(_log,this);
	if(_pSenderPump == NULL || !_pSenderPump->init(_pSinkConf)) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(SenderManager, "Failed to init MessageSenderPump."));
		throw ZQ::common::Exception("Failed to init MessageSenderPump.");
	}

	// init the position db
	_posDb = new LogPositionDb(_log);
	if((_posDb == NULL) || !_posDb->init(comm,posDbPath,posDbEvictorSize)) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(SenderManager, "Failed to init the PositionDb at %s with evictor"),posDbPath.c_str());
		 throw ZQ::common::Exception("Failed to init LogPositionDb.");
	}
	
}

SenderManager::~SenderManager(void)
{
	if(_pSenderPump != NULL)
	{
		delete _pSenderPump; 
		_pSenderPump = NULL; 
	}

	_posDb->uninit();
	if(_posDb != NULL)
	{
		delete _posDb; 
		_posDb = NULL; 
	}
}

void SenderManager::getPosition(const std::string& filePath,bool exist,MessageIdentity& mid)
{
	MessageSenderPump::vecMsgSender senders = _pSenderPump->query();
    for(size_t iSender = 0; iSender < senders.size(); ++iSender) {
        PositionRecordPtr record = _posDb->getPosition(filePath, senders[iSender].strType);
        if(!record) {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(SenderManager, "get the position of handler(%s) error, file(%s)"), senders[iSender].strType.c_str(), filePath.c_str());
            continue;
        }
        /*
        if(ZQ::common::now() - record->lastUpdatedAt() > DidcardLimit) {
            record->set(0, 0);
        }
        */
        if(!exist) { // need calculate the recover point from db records
            int64 position = 0;
            int64 stamp = 0;
            record->get(position, stamp);
            if((mid.stamp == -1 && mid.position == -1) ||
               (mid.stamp > stamp) ||
               (mid.stamp == stamp && mid.position > position)) {
                mid.stamp = stamp;
                mid.position = position;
            }
        }
    }
}

void SenderManager::getHandlers(const std::string& filepath,Handlers& TempHandlers)
{
	MessageSenderPump::vecMsgSender senders = _pSenderPump->query();
	for(size_t iSender = 0; iSender < senders.size(); ++iSender) {

		PositionRecordPtr record = _posDb->getPosition(filepath, senders[iSender].strType);
		if(!record) {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(SenderManager, "get the position of handler(%s) error, file(%s)"), senders[iSender].strType.c_str(), filepath.c_str());
			continue;
		}
		Handler handler;
		handler.type = senders[iSender].strType;
		handler.onMessage = senders[iSender].handle;
		handler.pos = record;
		TempHandlers.push_back(handler);
	}
}