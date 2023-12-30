
#ifndef _CPH_RDS_DEFINE_INCLUDE_
#define _CPH_RDS_DEFINE_INCLUDE_

//
// method type defines
//
#define METHODTYPE_RDSVSVSTRM			"SeaChange.MediaCluster.RDS"
#define METHODTYPE_RTFRDSVSVSTRM		"SeaChange.MediaCluster.RTFRDS"
#define METHODTYPE_RTFRDSH264VSVSTRM    "SeaChange.MediaCluster.RTFRDSH264"
#define METHODTYPE_RTIVSVSTRM			"SeaChange.MediaCluster.RTI"
#define METHODTYPE_FSCOPYVSVSTRM		"SeaChange.MediaCluster.FSCOPY"
#define METHODTYPE_NTFSRTFVSVSTRM		"SeaChange.MediaCluster.NTFSRTF"
#define METHODTYPE_NASCOPYVSVSTRM		"SeaChange.MediaCluster.NASCOPY"
#define METHODTYPE_FTPRTFVSVSTRM		"SeaChange.MediaCluster.FTPRTF"
#define METHODTYPE_RTIVSNAS				"SeaChange.NAS.RTI"
#define METHODTYPE_NPVRVSVSTRM          "SeaChange.MediaCluster.NPVR"
#define METHODTYPE_FTPPropagation		"SeaChange.MediaCluster.FTPPropagation"
#define METHODTYPE_CSI					"XOR.MediaCluster.CSI"

#define METHODTYPE_CDN_FTPPropagation		"SeaChange.CDN.FTPPropagation"
#define METHODTYPE_CDN_HTTPPropagation		"SeaChange.CDN.HTTPPropagation"
#define METHODTYPE_CDN_C2Pull		        "SeaChange.CDN.C2Pull"
#define METHODTYPE_CDN_C2PullH264		    "SeaChange.CDN.C2Pull.H264"
#define METHODTYPE_CDN_FTPRTF			"SeaChange.CDN.FTPRTF"
#define METHODTYPE_CDN_FTPRTFH264			"SeaChange.CDN.FTPRTF.H264"
#define METHODTYPE_CDN_NTFSRTF			"SeaChange.CDN.NTFSRTF"
#define METHODTYPE_CDN_NTFSRTFH264			"SeaChange.CDN.NTFSRTF.H264"



#define METHODTYPE_NTFSRTFH264VSVSTRM	 "SeaChange.MediaCluster.NTFSRTF.H264"
#define METHODTYPE_FTPRTFH264VSVSTRM	 "SeaChange.MediaCluster.FTPRTF.H264"
#define METHODTYPE_RTIH264VSVSTRM		 "SeaChange.MediaCluster.RTI.H264"

#define METHODTYPE_COPYDEMO                     "CopyDemo"

#define METHODTYPE_AQUAREC                  "SeaChange.AQUAREC"
#define METHODTYPE_RTIRAW				   "XOR.Raw.RTI"

#define METHODTYPE_AQUA_FTPRTF				"XOR.Aqua.FTPRTF"
#define METHODTYPE_AQUA_FTPRTFH264			"XOR.Aqua.FTPRTF.H264"
#define METHODTYPE_AQUA_FTPRTFH265			"XOR.Aqua.FTPRTF.H265"
#define METHODTYPE_AQUA_NTFSRTF				"XOR.Aqua.NTFSRTF"
#define METHODTYPE_AQUA_NTFSRTFH264			"XOR.Aqua.NTFSRTF.H264"
#define METHODTYPE_AQUA_NTFSRTFH265			"XOR.Aqua.NTFSRTF.H265"
#define METHODTYPE_AQUA_RTI					"XOR.Aqua.RTI"
#define METHODTYPE_AQUA_RTIH264				"XOR.Aqua.RTI.H264"
#define METHODTYPE_AQUA_RTIH265				"XOR.Aqua.RTI.H265"
#define METHODTYPE_AQUA_INDEX				"XOR.Aqua.Index"
#define METHODTYPE_AQUA_INDEXH264			"XOR.Aqua.Index.H264"
#define METHODTYPE_AQUA_INDEXH265			"XOR.Aqua.Index.H265"
#define METHODTYPE_AQUA_CSI					"XOR.Aqua.CSI"

//
// CPH plug-in parameters from CPE to CPH
//
#define CPHPM_FILENAME					"filename" 
#define CPHPM_BANDWIDTH					"bandwidth"
#define CPHPM_SOURCEURL					"sourceurl"
#define CPHPM_OUTPUTURL                                 "outputurl"
#define CPHPM_SUBTYPE                    "subtype"

#define CPHPM_INDEXTYPE                   "IndexType"
#define CPHPM_PROVIDERID                  "ProviderId"
#define CPHPM_PROVIDERASSETID             "ProviderAssetId"
#define CPHPM_AUGMENTATIONPIDS            "AugmentationPids"
#define CPHPM_PREENCRYPTION               "PreEncryption"
#define CPHPM_WISHEDTRICKSPEEDS            "WishedTrickSpeeds"
#define CPHPM_STARTTIME                    "StartTime"
#define CPHPM_ENDTIME                      "EndTime"
#define CPHPM_NOTRICKSPEEDS                "NOTrickSpeeds"
//
// event parameters from CPH to CPE
//
#define EVTPM_ERRORMESSAGE				"errmsg"
#define EVTPM_ERRORCODE					"errcode"
#define EVTPM_TOTOALSIZE				"totalsize"
#define EVTPM_MPEGBITRATE				"mpegbitrate"
#define EVTPM_VIDEOBITRATE				"videobitrate"
#define EVTPM_VIDEOHEIGHT				"videoheight"
#define EVTPM_VIDEOWIDTH				"videowidth"
#define EVTPM_PLAYTIME					"playtime"
#define EVTPM_FRAMERATE					"framerate"
#define EVTPM_SUPPORTFILESIZE		    "supportFilesize"
#define EVTPM_MD5CHECKSUM				"md5checksum"
#define EVTPM_MEMBERFILEEXTS            "memberFileExts"
#define EVTPM_OPENFORWRITE            "openForWrite" 
#define EVTPM_INDEXEXT               "indexFileExt" 
#define EVTPM_AUGMENTATIONPIDS        "augmentationpPids"
#define EVTPM_PREENCRYPTION           "preencryption"
#define EVTPM_ORIGINALBITRATE        "originalBitRate"
#define EVTPM_AUGMENTATEDBITRATE     "augmentedBitRate"
#define EVTPM_SOURCETYPE             "sourceType"

//
// provision session properties
//
#define PROPTY_PUSHURL					"Pty.PushUrl"
#define PROPTY_PROVISIONCANCELLED			"Provision.Canceled"




#endif
