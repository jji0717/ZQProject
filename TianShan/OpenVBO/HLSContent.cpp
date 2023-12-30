#include "HLSContent.h"
#include "urlstr.h"

typedef struct _BitrateName {
	const char* name; 
	uint32 bitrate;
} BitrateName;


static const BitrateName _bitrateNames[] =
{
	{ "bitrate1", 1280000 },
	{ "bitrate2", 2560000 },
	{ "bitrate3", 3750000 },
	{ "bitrate4", 10240000 },
	{ NULL, 2048000 },
};

static std::map<std::string,uint32> name2Bitrate;
static uint32 defaultBitrate = 2048000;

void HLSContent::setSubname2Bitrate( const std::map<std::string,uint32>& mapping, uint32 defBitrate )
{
	name2Bitrate = mapping;
	defaultBitrate = defBitrate;
}

uint32 convertSubname2Bitrate( const std::string& subname ) 
{
	if( name2Bitrate.empty())
	{
		for( size_t i = 0 ; i < sizeof(_bitrateNames)/sizeof(_bitrateNames[0]); i ++ )
		{
			if( !_bitrateNames[i].name )
				return _bitrateNames[i].bitrate;
			else if( strcmp( _bitrateNames[i].name,subname.c_str()) == 0 ) 
				return _bitrateNames[i].bitrate;
		}
		//should never be here
	}
	std::map<std::string,uint32>::const_iterator it = name2Bitrate.find(subname);
	if( it == name2Bitrate.end() )
		return defaultBitrate;
	else
		return it->second;
}

// -----------------------------
// class HLSContent
// -----------------------------
HLSContent::HLSContent(ZQ::common::Log& log, Authen5i& auth5i, CdmiFuseOps& cdmiClient, const std::string& contentName)
:_log(log), _auth5i(auth5i), _cdmiClient(cdmiClient), _contentName(contentName), _stampLastRead(0), _bStill(false)
{
}
#define FOPFMT(_OP, _X)  CLOGFMT(HLSContent, #_OP "(%s) " _X), _contentName.c_str()

bool HLSContent::read()
{
	std::string uri = _cdmiClient.pathToUri(_contentName + LOGIC_FNSEPS);
	_log(ZQ::common::Log::L_DEBUG, FOPFMT(read, "calling cdmi_ReadContainer(%s) to read content"), uri.c_str());
	int64 stampNow = ZQ::common::now();
	Json::Value result;
	CdmiFuseOps::CdmiRetCode cdmirc = _cdmiClient.cdmi_ReadContainer(result, uri);

	if (CdmiRet_FAIL(cdmirc))
	{
		_log(ZQ::common::Log::L_ERROR, FOPFMT(read, "cdmi_ReadContainer(%s) failed: %s(%d)"), uri.c_str(), _cdmiClient.cdmiRetStr(cdmirc), cdmirc);
		return false;
	}

	CdmiFuseOps::StrList playlistToRead;

	// step 2.1 read the children list
	if (result.isMember("children"))
	{
		ZQ::common::MutexGuard g(_lk);
		Json::Value& children = result["children"];
		for (Json::Value::iterator itC = children.begin(); itC != children.end(); itC++)
		{
			std::string subname = (*itC).asString();
			if (LOGIC_FNSEPC != subname[subname.length()-1]) // ignore files
				continue;

			subname = subname.substr(0, subname.length()-1);
			PlaylistMap::iterator itPL = _playlistMap.find(subname);
			if (_playlistMap.end() == itPL)
			{
				PlaylistInfo pi;
				pi.stampLastRead = pi.sequenceStart =0;
				pi.subName = subname;
				MAPSET(PlaylistMap, _playlistMap, pi.subName, pi);
				itPL = _playlistMap.find(subname);
			}

			if (_playlistMap.end() == itPL)
				continue;

			if (!_bStill && itPL->second.stampLastRead < stampNow - MIN_REFRESH_INTERVAL)
				playlistToRead.push_back(subname);
		}
	}

	// step 2.2 read the metadata
	if (result.isMember("metadata"))
	{
		Json::Value& metadata = result["metadata"];
		ZQ::common::MutexGuard g(_lk);
#define COPY_METADATA(key, valtype)   if (metadata.isMember(#key)) { MAPSET(CdmiFuseOps::Properties, _metadata, #key, metadata[#key].as##valtype()); }
		COPY_METADATA(npvr_name, String);
		COPY_METADATA(npvr_source, String);
		COPY_METADATA(npvr_recording_start, String);
		COPY_METADATA(npvr_recording_end, String);

		// adjust the bStill per npvr_recording_end
		if (!_bStill && _metadata.end() != _metadata.find("npvr_recording_end"))
		{
			int64 timeEnd = ZQ::common::TimeUtil::ISO8601ToTime(_metadata["npvr_recording_end"].c_str()); 
			if (timeEnd>0 && timeEnd < (stampNow - MIN_REFRESH_INTERVAL*10))
				_bStill = true;
		}
	}

	_stampLastRead = stampNow;

	// step 2.3 read the per-bitrate playlist
	for (size_t i =0; i < playlistToRead.size(); i++)
		readPlaylist(playlistToRead[i]);

	return true;
}


bool HLSContent::readPlaylist(const std::string& subname)
{
	std::string uri = _cdmiClient.pathToUri(_contentName + LOGIC_FNSEPS + subname + LOGIC_FNSEPS);
	_log(ZQ::common::Log::L_DEBUG, FOPFMT(read, "calling cdmi_ReadContainer(%s) to read playlist[%s]"), uri.c_str(), subname.c_str());
	int64 stampNow = ZQ::common::now();
	Json::Value result;
	CdmiFuseOps::CdmiRetCode cdmirc = _cdmiClient.cdmi_ReadContainer(result, uri);

	if (CdmiRet_FAIL(cdmirc))
	{
		_log(ZQ::common::Log::L_ERROR, FOPFMT(read, "cdmi_ReadContainer(%s) failed: %s(%d)"), uri.c_str(), _cdmiClient.cdmiRetStr(cdmirc), cdmirc);
		return false;
	}

	PlaylistInfo pi;
	pi.subName = subname;
	pi.bitrate =0;
	pi.sequenceStart =0;

	// step 2.0 map the subname to bitrate
	pi.bitrate = convertSubname2Bitrate( pi.subName);

	// step 2.1 read the children list
	if (result.isMember("children"))
	{
		Json::Value& children = result["children"];
		for (Json::Value::iterator itC = children.begin(); itC != children.end(); itC++)
		{
			std::string filename = (*itC).asString();
			if (LOGIC_FNSEPC == filename[filename.length()-1]) // ignore containers
				continue;
			pi.segments.push_back(filename);
		}
	}

	// step 2.2 read the metadata
	if (result.isMember("metadata"))
	{
		try {
			Json::Value& metadata = result["metadata"];
			if (metadata.isMember("stream_inf"))
			{
				pi.streamInf = metadata["stream_inf"].asString();

				// adjust the bitrate if stream_inf presents it
				size_t pos = pi.streamInf.find("BANDWIDTH=");
				if (std::string::npos != pos)
					pi.bitrate = atol(pi.streamInf.substr(pos + sizeof("BANDWIDTH=")-1).c_str());
			}

			if (metadata.isMember("sequence_start"))
			{
				pi.sequenceStart = metadata["sequence_start"].asInt();
			}
		}
		catch(...) {}
	}

	pi.stampLastRead = ZQ::common::now();

	// step 3 about the metadata read from the first segement, also make its location cached
	if (pi.segments.size() >0)
	{
		std::string contentType, location;
		char buf[20];
		uint32 len = 1;
//		cdmirc = _cdmiClient.cdmi_ReadDataObject(result, uri +"?metadata", tmp);
		cdmirc = _cdmiClient.nonCdmi_ReadDataObject(uri + pi.segments[0], contentType, location, 0, len, buf);
		if (CdmiRet_FAIL(cdmirc))
			_log(ZQ::common::Log::L_ERROR, FOPFMT(read, "cdmi_ReadDataObject(%s) read first segment failed: %s(%d)"), uri.c_str(), _cdmiClient.cdmiRetStr(cdmirc), cdmirc);
		else
		{
			pi.mimeType = contentType;
/*
			try {
				if (result.isMember("metadata"))
				{
#pragma message ( __MSGLOC__ "TODO: read the MIME-type and so on")
				}
			}
			catch(...) {}
*/
		}
	}

	ZQ::common::MutexGuard g(_lk);
	MAPSET(PlaylistMap, _playlistMap, pi.subName, pi);

	return true;
}

bool HLSContent::exportMasterM3u8(std::string& m3u8Content, int keyId, const char* sessionId, const char* ipAddr, const char* hlsRootUri, int relativeUriLevel)
{
	int64 stampNow = ZQ::common::now();

	if (_stampLastRead <=0 || !_bStill && (_stampLastRead < stampNow - MIN_REFRESH_INTERVAL))
		read();

	if (_stampLastRead <=0)
		return false;

	std::string rootUri, expiration, sessId, ip;

	// validate rootUri
	if (NULL != hlsRootUri)
		rootUri = hlsRootUri;
	if (!rootUri.empty() && LOGIC_FNSEPC != rootUri[rootUri.length() -1])
		rootUri +=LOGIC_FNSEPS;

	// validate sessionId
	sessId = (NULL != sessionId) ? sessionId: "000";

	// validate ipAddr
	ip = (NULL != ipAddr) ? ipAddr: "127.0.0.1";

	// determine expiration
	char buf[100];
	time_t tExp = time(NULL) + 8*60*60; // 8hour
	struct tm gmt = *gmtime(&tExp);
	strftime(buf, sizeof(buf)-2, "%Y%m%d%H%M%S", &gmt);
	expiration = buf;

	m3u8Content = "#EXTM3U\r\n";

	ZQ::common::MutexGuard g(_lk);
	int i=1;
	for (PlaylistMap::iterator itPL = _playlistMap.begin(); itPL != _playlistMap.end(); itPL++)
	{
		if (!itPL->second.streamInf.empty())
			m3u8Content += std::string("\r\n#EXT-X-STREAM-INF:") + itPL->second.streamInf + "\r\n";
		else
		{
			snprintf(buf, sizeof(buf)-2, "PROGRAM-ID=%d,BANDWIDTH=%u", i++, itPL->second.bitrate);
			m3u8Content += std::string("\r\n#EXT-X-STREAM-INF:") + buf + "\r\n";
		}

		std::string playlistName = _contentName + "_" + itPL->first;
		std::string uri = rootUri + playlistName + ".m3u8";
		std::string fullUrl = uri;
		bool bFullURLGiven = true;
		if (std::string::npos == fullUrl.find("://"))
		{
			// borrow a dummy server to complete the url for _auth5i.sign()
			
			// prefill the leading '/' if not exists
			if (LOGIC_FNSEPC != fullUrl[0])
				fullUrl = std::string(LOGIC_FNSEPS) + fullUrl;

			fullUrl = std::string("http://localhost") + fullUrl;
			bFullURLGiven = false;
		}

		_auth5i.sign(fullUrl, keyId, sessId, ip, expiration);
		uri = fullUrl.substr(fullUrl.find(uri));
		
		_log(ZQ::common::Log::L_DEBUG, FOPFMT(exportMasterM3u8, "auth5i signed: %s"), uri.c_str());

		// adjust the output uri string per relativeUriLevel
		if (0 == relativeUriLevel && bFullURLGiven)
			uri = fullUrl;
		else if (1 == relativeUriLevel)
			; // uri already has the expected value
		else if (2 == relativeUriLevel)
			uri = fullUrl.substr(fullUrl.find(playlistName));

		// the output uri string in m3u8
		m3u8Content += uri + "\r\n";
	}

	return (_playlistMap.size() >0);
}

bool HLSContent::exportPlaylistM3u8(const std::string& subname, std::string& m3u8Content)
{
	int64 stampNow = ZQ::common::now();

	if (_stampLastRead <=0 || !_bStill && (_stampLastRead < stampNow - MIN_REFRESH_INTERVAL))
		read();

	if (_stampLastRead <=0)
		return false;

	PlaylistInfo pi;
	{
		ZQ::common::MutexGuard g(_lk);
		PlaylistMap::iterator itPL = _playlistMap.find(subname);
		if (_playlistMap.end() == itPL)
			return false;
		if (itPL->second.stampLastRead <=0)
			return false;

		pi = itPL->second;
	}

	ZQ::common::M3U8Stream::IndexInfo idxInfo;
	idxInfo.bandwidth = pi.bitrate;

	ZQ::common::M3U8Stream m3u8strm(idxInfo);
	ZQ::common::M3U8Stream::Segment seg;
	seg.byteStart = seg.byteStop  =0;
	seg.bandwidth = pi.bitrate;
	seg.duration  = DEFAULT_SEGMENT_DUR;

	size_t i=0;
	char buf[256];
	CdmiFuseOps::ResourceLocation playlistLoc;
	for (i=0; i < pi.segments.size(); i++)
	{
		// step 1. compose the uri
		seg.uri = _cdmiClient.pathToUri(_contentName + LOGIC_FNSEPS + subname + LOGIC_FNSEPS + pi.segments[i]);
		if (playlistLoc.locationIp.empty()&& _cdmiClient.readLocationFromCache(seg.uri, playlistLoc))
			_log(ZQ::common::Log::L_DEBUG, FOPFMT(exportPlaylistM3u8, "sub[%s] found cached location[%s] by %s"), subname.c_str(), playlistLoc.locationIp.c_str(), seg.uri.c_str());

		CdmiFuseOps::ResourceLocation loc;
		loc.locationIp = playlistLoc.locationIp.empty() ? _cdmiClient.getServerIp(): playlistLoc.locationIp;

		// step 2. generate token
		std::string xAquaDate = _cdmiClient.timeToAquaData(ZQ::common::now() + DEFAULT_AQUA_TOKEN_TIMEOUT);
		std::string signature;

		//FIXME: get root url from _cdmiClient ? or parse it from configuration file
		static std::string rootURI;
		if(rootURI.empty() ) {
			CdmiFuseOps::ServerSideParams ssp;
			_cdmiClient.getServerSideParams(ssp);
			rootURI = ssp.rootURI;
		}
		std::string uriToSign = rootURI;
		if(!uriToSign.empty() && uriToSign[uriToSign.length() -1] != '/')
			uriToSign += "/";
		uriToSign += seg.uri;

		if(!_cdmiClient.generateSignature(signature, uriToSign, pi.mimeType, ZQ::common::CURLClient::HTTP_GET, xAquaDate))
		{
			_log(ZQ::common::Log::L_ERROR, FOPFMT(exportPlaylistM3u8, "cdmi_CreateDataObject[%s] failed to generate signature"), seg.uri.c_str());
			return false;
		}

		ZQ::common::URLStr::encode(signature.c_str(), buf, sizeof(buf)-2);
		signature = buf;

		// step 3. assemble the URL with loc
		seg.uri = _cdmiClient.getRelocatedURL(seg.uri, loc, CdmiFuseOps::CDMI_FILE, false);

		// step 4. append the URL
		seg.uri += std::string("?aquatoken=") + _cdmiClient.getAccessKey() + ":" + signature + "&xaquadate=" + xAquaDate;

		ZQ::common::URLStr::encode(pi.mimeType.c_str(), buf, sizeof(buf)-2);
		seg.uri += std::string("&contenttype=") + buf;

		m3u8strm.push_back(seg);
	}

	if (_bStill)
		m3u8strm.endPlaylist();

	return (m3u8strm.exportM3U8(m3u8Content) >0);
}

