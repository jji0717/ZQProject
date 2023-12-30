#include "ConcurrentRequest.h"
#include <algorithm>

namespace ZQ{
    namespace StreamService{
        ConcurrentRequest::ConcurrentRequest(ZQ::common::Config::Loader<C2ClientConf>& conf,
            ZQ::common::Log& log, ILoopRequest::Ptr cb, int loopIndex)
            : _conf(conf), _log(log), _resultCollectionPtr(NULL), _cb(cb), _currLoop(loopIndex), _client(0), _fileNumber(0)
        {
            _resultCollectionPtr = new ZQ::StreamService::C2ResultCollection(_log, _cb, _conf.client);
        }

        ConcurrentRequest::~ConcurrentRequest()
        {
            if(_cb)
            {
                 _cb = NULL;
            }

            if (_resultCollectionPtr)
            {
                _resultCollectionPtr = NULL;
            }
        }

        void ConcurrentRequest::printStatisticalData()
        {
            if (_resultCollectionPtr)
            {
                _resultCollectionPtr->printStatisticalData();
            }
        }

        int ConcurrentRequest::run(void)
        {
            if(!_resultCollectionPtr)
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(ConcurrentRequest, "start() _resultCollectionPtr is NULL"));
                return 0;
            }

            int fileIndex = 0;
            int client = _conf.client;
            _fileNumber = MIN(_conf.file, _conf._filesHolder.files.size());

            while(client--)
            {
                ZQ::StreamService::RequestParams params;
                params.upstreamIP = _conf._requestParamsHolder.upstreamIP;
                params.locateIP = _conf._requestParamsHolder.locateIP;
                params.locatePort = _conf._requestParamsHolder.locatePort;
                params.defaultGetPort = _conf._requestParamsHolder.defaultGetPort;
                params.url = _conf._requestParamsHolder.url;
                params.clientTransfer = _conf._requestParamsHolder.clientTransfer;
                params.transferRate = _conf._requestParamsHolder.transferRate;
                params.ingressCapacity = _conf._requestParamsHolder.ingressCapacity;
                params.exclusionList = _conf._requestParamsHolder.exclusionList;
                params.transferDelay = _conf._requestParamsHolder.transferDelay;
                params.indexTimeout = _conf._requestParamsHolder.indexTimeout;
                params.indexRetryTimes = _conf._requestParamsHolder.indexRetry;
                params.mainfileTimeout      = _conf._requestParamsHolder.mainfileTimeout;
                params.mainfileRetryTimes   = _conf._requestParamsHolder.mainfileRetry;
                params.range = _conf._requestParamsHolder.range;
                params.alignment = _conf._requestParamsHolder.alignment;

                // select file to request
                params.filename = _conf._filesHolder.files[fileIndex].name;
                fileIndex = (++fileIndex)%_fileNumber;

                ZQ::StreamService::C2Client::Ptr c2clientPtr = new ZQ::StreamService::C2Client(_log, _resultCollectionPtr, params);
                c2clientPtr->startRequest();

                SYS::sleep(_conf.interval);
            }
        }
    }
}
