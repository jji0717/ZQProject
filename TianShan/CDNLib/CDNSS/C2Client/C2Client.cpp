#include "C2Client.h"
#include <TimeUtil.h>

namespace ZQ{
	namespace StreamService{
        ZQ::common::AtomicInt C2Client::_atomicID;
		C2Client::C2Client(ZQ::common::Log& log, IC2CallBack::Ptr cb, RequestParams params, bool bSaveFile)
            : _log(log), _cb(cb), _params(params), _currRetryCount(0), _bSaveFile(false), _startRange(0), _endRange(0)
		{
            _atomicID.inc();
            _id = _atomicID.get();

            // init statistic data
            _statisticData.filename = _params.filename;
            // #TODO
            //_statisticData.reqBitrate = _params.transferRate;
            _md5 = new ZQ::common::md5();

            _phase = ZQ::StreamService::phaseInit;
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] constructor entry, enter phase [%s]"), _id, getCurrPhaseStr().c_str());
		}

        C2Client::~C2Client()
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] destructor entry, release resources"), _id);

            if(_cb)
            {
                 _cb = NULL;
            }

            if (_requestHandle)
            {
                _requestHandle = NULL;
                //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] set request handle to NULL"), _id);
            }
        }

        std::string C2Client::getCurrPhaseStr()
        {
            switch(_phase)
            {
            case ZQ::StreamService::phaseInit:
                return "Init";
            case ZQ::StreamService::phaseLocate:
                return "Locate";
            case ZQ::StreamService::phaseGet:
                return "Get";
            case ZQ::StreamService::phaseTransferDelete:
                return "TransferDelete";
            case ZQ::StreamService::phaseDone:
                return "Done";
            }
        }

        int C2Client::getMaxRetry()
        {
            if ("index" == _params.subType)
            {
                return _params.indexRetryTimes;
            }
            return _params.mainfileRetryTimes;
        }

        bool C2Client::startRequest()
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2Client, "CID[%ld] startRequest() prepare for locate request"), _id);

            _phase = ZQ::StreamService::phaseLocate;
            // #TODO
            size_t mpos = _params.range.find_first_of('-');
            if (std::string::npos != mpos)
            {
                _startRange = atol(_params.range.substr(0, mpos).c_str());
                if (_params.range.length() != mpos + 1)
                {
                    _endRange = atol(_params.range.substr(mpos + 1).c_str());
                    _statisticData.requetSize = _endRange - _startRange;
                }
            }
            // _statisticData.requetSize = _params.range;

            _requestHandle = new LocateRequest(_log, this, _params, "C2Client", _id);

            //(new AsyncRequest(_requestHandle, &_requestHandle->getLoop()))->queueWork();
            return _requestHandle->process();
        }

        bool C2Client::retry()
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2Client, "CID[%ld] retry(), [%d]-th retry"), _id, ++_currRetryCount);
            _statisticData.currRetryNum = _currRetryCount;
            std::ostringstream oss;
            oss<<_startRange + _statisticData.receivedSize<<"-";
            if (_endRange != 0)
            {
                oss<<_endRange;
            }
            _params.range = oss.str();
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] retry(), next request range[%s]"), _id, _params.range.c_str());
            return startRequest();
        }

        void C2Client::onLocate(const LocateResponseData& resp)
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2Client, "CID[%ld] onLocate() locate request complete, prepare for get request"), _id);
            _params.transferID  = resp.transferId;
            _params.getAddr = resp.transferPort;

            if (resp.portNum.empty())
            {
                _log(ZQ::common::Log::L_INFO, CLOGFMT(C2Client, "CID[%ld] onLocate() can't find 'PortNum' from locate response, try to use default port[%d]"), _id, _params.defaultGetPort);
                _params.getPort = _params.defaultGetPort;
            }else{
                _params.getPort = atoi(resp.portNum.c_str());
            }

            _statisticData.locateSuccess = true;
            _cb->onStart(_id, _statisticData);
            _requestHandle = new GetRequest(_log, this, _params, "C2Client", _id);

            _phase = ZQ::StreamService::phaseGet;
            _requestHandle->process();
            if (_currRetryCount == 0)
            {
                _startTime = ZQ::common::TimeUtil::now();
            }
        }

        void C2Client::onData(const char* data, const size_t& size, bool error)
        {
            // check file data
            int64 offset = _statisticData.receivedSize;
            int64 end = offset + size;

            int64 pos = 188-offset % 188;
            for( ; pos < size; pos+=188 * 10) {
                char ch = *(data + pos);
                if(ch != 0x47){
                    //assert(false && "failed to get right data");
                    //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] onData() data error"), _id);
                }
            }

            _abadonLength = 0;
            int64 usefulLength = 0;
            if (error)
            {
                assert(_params.alignment > 0);
                usefulLength = ((size - _params.alignment * 1024) / (_params.alignment*1024)) * _params.alignment*1024;
                usefulLength = usefulLength < 0 ? 0 : usefulLength;
                _abadonLength = size - usefulLength;
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] onData() error occured, data[%ld] useful data[%ld], align abandon data[%ld]"), _id, size, usefulLength, _abadonLength);
            }

            //_md5->Update((unsigned char*)data, size - _abadonLength);
            _statisticData.receivedSize += size - _abadonLength;

            int64 currTime = ZQ::common::TimeUtil::now();
            _statisticData.recvBitrate = ((double)_statisticData.receivedSize) / (currTime - _startTime) * 1000 * 8;

            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] onData() received data[%ld/%ld] average bitrate[%ld]bps"), _id, usefulLength, _statisticData.receivedSize, _statisticData.recvBitrate);

            _cb->onProcess(_id, _statisticData);
            /*if (!_bSaveFile)
            {
                // do something
                return;
            }*/
            // save file to disk
			//std::string fn="/opt/TianShan/logs/thread/";
			//fn += _statisticData.filename;
			/*FILE* file = fopen(fn.c_str(),"ab");
			if (file == NULL)
			{
				fclose(file);
				assert(false && "open file failed");
				return;
			}
			int res = fwrite(data, sizeof(unsigned char), size, file);
			if ( res < size)
			{
				fclose(file);
				assert(false && "write failed.");
				return ;
			}

			fclose(file);*/
			return;
		}
        void C2Client::onRecvComplete()
        {
            int64 currTime = ZQ::common::TimeUtil::now();
            _statisticData.recvBitrate = ((double)_statisticData.receivedSize) / (currTime - _startTime) * 1000 * 8;
            _statisticData.usedtime = currTime - _startTime;

            _md5->Finalize();
            _statisticData.md5 = _md5->PrintMD5(_md5->Digest());
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] onRecvComplete() file[%s] md5[%s] received all data[%ld] average bitrate[%ld]bps"),
                _id, _params.filename.c_str(), _statisticData.md5.c_str(), _statisticData.receivedSize, _statisticData.recvBitrate);
            _cb->onSuccess(_id, _statisticData);

            if (_params.transferDelete)
            {
                _log(ZQ::common::Log::L_INFO, CLOGFMT(C2Client, "CID[%ld] onRecvComplete() transfer delete is enabled, prepare for send transfer delete request"), _id);

                _requestHandle = new TransferDelete(_log, this, _params, "C2Client", _id);
                _phase = ZQ::StreamService::phaseTransferDelete;
                _requestHandle->process();
                return;
            }

            _phase = ZQ::StreamService::phaseDone;

            _requestHandle = NULL;
        }

        void C2Client::onTransferDelete()
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2Client, "CID[%ld] onTransferDelete() transfer delete complete"), _id);

            _phase = ZQ::StreamService::phaseDone;

            _requestHandle = NULL;
        }

        void C2Client::onError(C2RequestErrorCategory category, const int& err, const std::string& msg)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2Client, "CID[%ld] onError() error[%d:%s] current phase[%s], retry times[%d/%d]"), _id, err, msg.c_str(), getCurrPhaseStr().c_str(), _currRetryCount, getMaxRetry());

            if (_currRetryCount < getMaxRetry() &&
                (ZQ::StreamService::crTimeout == category || ZQ::StreamService::crWaitBufferTimeout == category))
            {
                if (!retry())
                {
                    _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2Client, "CID[%ld] onError() locate request failed"), _id);
                    onError(ZQ::StreamService::crGeneric, -14, "locate request failed");
                }
                return;
            }

            // no need for retry, then add the abadon data
            _statisticData.requetSize += _abadonLength;
            _abadonLength = 0;

            int64 currTime = ZQ::common::TimeUtil::now();
            _statisticData.recvBitrate = ((double)_statisticData.receivedSize) / (currTime - _startTime) * 1000 * 8;
            _statisticData.usedtime = currTime - _startTime;

            _statisticData.errorCode = err;
            _statisticData.errMessage = msg;
            _cb->onError(_id, _statisticData);
            _phase = ZQ::StreamService::phaseDone;
        }
	}
}
