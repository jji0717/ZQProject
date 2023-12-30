#include "./soapAssetGearServiceSoapBindingProxy.h"	// get proxy
#include "./AssetGearServiceSoapBinding.nsmap"		// get namespace bindings

class MyAssetSB:  public AssetGearServiceSoapBinding
{
public:
	MyAssetSB(const char*IP, int port)
	{
		static char szEndPoint[256];
		sprintf(szEndPoint, "http://%s:%d/services/AssetGearService", IP, port);

		endpoint = szEndPoint;
		if (soap && !soap->namespaces)
		{ 
			static const struct Namespace namespaces[] = 
			{
				{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
				{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
				{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
				{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
				{"AssetGear2", szEndPoint, NULL, NULL},
				{NULL, NULL, NULL, NULL}
			};
		}
	};
};

void usage()
{
    cout << "\nUsage: AGTest -s -p -a -t -e -r"
         << "\n-s  server address, default: 127.0.0.1"
		<< "\n-p  server port, default: 1200"
		<< "\n-a  asset name, default: hellopig"
		<< "\n-r  retry flag, not specified will be not retry"
		<< "\n-t  starttime, time format: 2005-10-10T12:00:00, default: 0"
		<< "\n-e  endtime, time format: 2005-10-10T12:00:00, default: 0\n";
}

int main(int argc, char* argv[])
{
	std::string programId = "1";
	std::string assetName = "hellopig";
	int scheduleID = 101;
	std::string startTime = "0", endTime = "0";
	std::string pushUrl;
	std::string assetId;
	int bitrate =3750000;
	int priority =69;
	std::string provider="SeaChange";
	int nRetry = 0;
	std::string strIP = "127.0.0.1";
	int nsPort = 1200;
	std::string errorMessage;

	int nSet = 0;

	int idx = 1;
    while (idx < argc)
    {
        if ( (strcmp(argv[idx], "-s") == 0 ||strcmp(argv[idx], "-S") == 0) &&  ++idx < argc )
        {
            if (argv[idx])
				strIP = argv[idx];
        }
        else if (( strcmp(argv[idx], "-p") == 0 || strcmp(argv[idx], "-P") == 0) &&  ++idx < argc )
        {
            if (argv[idx])
				nsPort = atoi(argv[idx]);			
        }
        else if ( (strcmp(argv[idx], "-a") == 0 ||strcmp(argv[idx], "-A") == 0) &&  ++idx < argc )
        {
            if (argv[idx])
			{
				assetName = argv[idx];
				nSet = 1;
			}
        }
        else if ( (strcmp(argv[idx], "-t") == 0 ||strcmp(argv[idx], "-T") == 0) &&  ++idx < argc )
        {
            if (argv[idx])
				startTime = argv[idx];
        }
        else if ( (strcmp(argv[idx], "-e") == 0 ||strcmp(argv[idx], "-E") == 0) &&  ++idx < argc )
        {
            if (argv[idx])
				endTime = argv[idx];
        }
        else if ( (strcmp(argv[idx], "-r") == 0 ||strcmp(argv[idx], "-R") == 0))
        {
            nRetry = 1;
        }
        else
        {
            usage();
            return 0;
        }
        ++ idx;
    }

	if (!nSet)
	{
        usage();
        return 0;
	}

	MyAssetSB AgClient(strIP.c_str(), nsPort);

	//prepare the schedule model
	AssetGear2__ScheduleModel* pSchdmodel = soap_new_AssetGear2__ScheduleModel(AgClient.soap, -1);
	if (NULL == pSchdmodel)
		return false;

	AssetGear2__StatusModel*  pStatusModel = soap_new_AssetGear2__StatusModel(AgClient.soap, -1);
	if (NULL == pStatusModel)
		return false;

	// required attribute
	pSchdmodel->scheduleID = scheduleID;	// required attribute
	// optional attributes
	int cid = 98;
	pSchdmodel->channelID = &cid;
	pSchdmodel->channelName = NULL;
	pSchdmodel->startTime = &startTime;
	pSchdmodel->endTime = &endTime;
	pSchdmodel->priority = &priority;

	pStatusModel->errorMessage = &errorMessage; 

	// prepare pAssetmodel
	AssetGear2__AssetModel* pAssetmodel = soap_new_AssetGear2__AssetModel(AgClient.soap, -1);
	if (NULL == pAssetmodel)
		return false;

	// required attribute
	pAssetmodel->programID = programId;
	pAssetmodel->Schedule = pSchdmodel;
	pAssetmodel->assetName = &assetName;
	pAssetmodel->bitRate = &bitrate;
	pAssetmodel->provider = &provider;
	pAssetmodel->Status = pStatusModel;

	// optional attribute
	pAssetmodel->ListOfMetaData = NULL;
	pAssetmodel->assetID = &assetId;
	pAssetmodel->url = &pushUrl;
	if (nRetry)
		pAssetmodel->retryFlag = _1;
	else
		pAssetmodel->retryFlag = _0;


	// invoke the AssetGear
	struct AssetGear2__createAssetResponse aResponse;
	AssetGear2__AssetModel result;	
	aResponse._serviceReturn = &result;
	if(0 != AgClient.AssetGear2__createAsset(pAssetmodel, aResponse))
	{
		// error
		soap_print_fault(AgClient.soap, stderr);
		return -1;
	}

	// process the result
//	assetId = *result.assetID;
//	pushUrl = *result.url;
	assetId = *aResponse._serviceReturn->assetID;
	pushUrl = *aResponse._serviceReturn->url;

	printf("AssetId: %08x, PushUrl: %s\n", assetId, pushUrl.c_str());

	AssetGear2__StatusModel*  pStatus = aResponse._serviceReturn->Status;
	if (pStatus)
		printf("Status code: %d, Status string: %s\n", pStatus->errorCode, (pStatus->errorMessage?pStatus->errorMessage->c_str():""));

	return 0;
}
