#ifndef __ZQ_CRM_C3DSERVER_DEFINE_H_
#define __ZQ_CRM_C3DSERVER_DEFINE_H_
#include  <string>
#include <vector>
#include <map>

#define XML_HEADER			"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
#define XML_XMLNS			"urn:eventis:cpi:1.0"
///#define http request URL
#define URL_CONTENT        ".*/Content/.*"
#define URL_CONTENTS		 ".*/Contents?.*"

//#define 3d http request key
#define KEY_CONTENT		"/Content"
#define KEY_CONTENT_ID	"id"
#define KEY_CONTENT_XMLNS	"xmlns"
#define KEY_NAME				"Name"
#define KEY_PROVIDER		"Provider"
#define KEY_SOURCEURI	"SourceUri"
#define KEY_INGESTSTARTTIME	"IngestStartTime"
#define KEY_INGESTENDTIME		"IngestEndTime"
#define KEY_BITRATEINBPS			"BitrateInBps"
#define KEY_STATUS						"Status"
#define KEY_INGESTPERCENTAGE	"IngestPercentage"

#define METHODTYPE_AQUAREC		"SeaChange.AQUAREC"
//define 3d server metadata type
#define METADATA_NAME				"npvr_name"
#define METADATA_PROVIDER		"npvr_provider"
#define METADATA_SOURCE			"npvr_source"
#define METADATA_START				"npvr_recording_start"
#define METADATA_END					"npvr_recording_end"
#define METADATA_BITRATE			"npvr_bandwidth"

//define npvr status
#define STATUS_INITIAL					"Initial"
#define STATUS_INGESTING			"Ingesting"
#define STATUS_INGESTINGANDPLAYABLE		"IngestingAndPlayable"
#define STATUS_INGESTED			"Ingested"
#define STATUS_INGESTMISSED	"IngestMissed"
#define STATUS_NOTVALID			"NotValid"

//define response code
#define STATUS_OK							200
#define STATUS_BADREQUEST		400
#define STATUS_FORBIDDEN			403
#define STATUS_NOTFOUND			404
#define STATUS_METHODNOTVALID	455
#define STATUS_ERROR					500
#define STATUS_STOP						501

//define status codes reason
#define REASON_OK						"OK"
#define REASON_BADREQUEST	"Bad Request"
#define REASON_FORBIDDEN		"Forbidden"
#define REASON_NOTFOUND		"Not Found"
#define REASON_ERROR				"Internal server error"
#define REASON_STOP					"User Stop"
#define REASON_METHODNOTVALID	"Method Not Valid in This State"

#define MAX_SESSIOONCOUNT    20
#define TIME_PERIOD						10*60*500


#endif//__ZQ_CRM_3DSERVER_DEFINE_H_

