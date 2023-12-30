#ifndef __ZQTianShan_CDN_Defines_H__
#define __ZQTianShan_CDN_Defines_H__


//CDN transfer initiate private data key
#define CDNPROP(x) "cdn."#x
#define CDN_TRANSFERID			CDNPROP(transferId)
#define CDN_CLIENTTRANSFER		CDNPROP(clientTransfer)
#define CDN_TRANSFERPORT		CDNPROP(transferPort)
#define CDN_TRANSFERPORTNUM		CDNPROP(transferPortNum)
#define CDN_TRANSFERADDRESS		CDNPROP(transferAddress)
#define CDN_INGRESSCAPACITY		CDNPROP(ingressCapacity)
#define CDN_ALLOCATEDCAPACITY	CDNPROP(allocatedIngressCapacity)
#define CDN_FILENAME			CDNPROP(fileName)
#define CDN_TRANSFERRATE		CDNPROP(transferRate)
#define CDN_TRANSFERTIMEOUT		CDNPROP(transferTimeout)
#define CDN_RANGE				CDNPROP(range)
#define CDN_DELAY				CDNPROP(delay)									
#define CDN_SUBTYPE				CDNPROP(subType)								//std::string
#define CDN_EXTENSIONNAME		CDNPROP(extensionFileName)						//std::string
#define CDN_AVAILRANGE			CDNPROP(availableRange)							//std::string
#define CDN_OPENFORWRITE		CDNPROP(openForWrite)							//std::string

#define CDN_PID                 CDNPROP(pid)    // string
#define CDN_PAID                CDNPROP(paid)   // string

// CDN type: SeaChange or NGODC2(default)
#define CDN_CDNTYPE             CDNPROP(CDNType)   // string

// predefined subtype
#define CDN_SUBTYPE_Index       "index"
#define CDN_SUBTYPE_NormalForward "forward/1"

//
#define CDN_IDXCONTENT_GENERIC	"idx_content_generic"
#define CDN_IDXCONTENT_SUBFILES	"idx_content_subfiles"

//Cdn Streamer replica property key
#define STREAMERPROP_CAPACITY					"StreamerCapacity"
#define STREAMERPROP_ACTIVETRANSFERCOUNT		"ActiveTrasferCount"
#define STREAMERPROP_ACTIVEBANDWIDTH			"ActiveBandwidth"
#define STREAMERPROP_ADDRESSIPV4				"StreamerAddressV4"
#define STREAMERPROP_ADDRESSIPV6				"StreamerAddressV6"
#define STREAMERPROP_ADDRESSTCPPORT				"StreamerTcpPort"
#define STREAMERPROP_VOLUMENETID				"ContentVolumeNetId"


#endif
