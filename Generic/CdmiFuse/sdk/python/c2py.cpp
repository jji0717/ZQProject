#include <FileLog.h>

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/exception_translator.hpp>
#include "../../CdmiFuseOps.h"

using namespace boost::python;

typedef std::vector<std::string> StrList;
typedef std::map<std::string, std::string> StrMap;

ZQ::common::NativeThreadPool* mThdPool;
ZQ::common::FileLog*	mFileLogger;
int clientCount = 0;

class NestedClient {
public:
	NestedClient( const std::string& config, const std::string& rootUrl, const std::string& userDomain, const std::string& homecontainer );
	virtual ~NestedClient();
	int exec_Cdmi( const std::string& cmd, const std::string& uri, const std::string& jsonArgs, std::string& jsonResult);
	int exec_nonCdmi( const std::string& cmd, const std::string& uri, const std::string& jsonArgs, std::string& jsonResult, char* buf, int& bufLen);
	std::string pathToUrl( const std::string& path );
protected:
	int call_cdmi_createDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_readDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_updateDataObjectEx( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_updateDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_deleteDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_createContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_readContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_updateContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_deleteContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );
	int call_cdmi_readDomain( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult );

	int call_noncdmi_createDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );
	int call_noncdmi_readDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );
	int call_noncdmi_updateDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );
	int call_noncdmi_deleteDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );
	int call_noncdmi_createContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );
	int call_noncdmi_deleteContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );
	int call_noncdmi_flushDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen );

private:
	void				initCdmiFuse( const std::string& config, const std::string& rooturl, const std::string& userDomain, const std::string& homeContainer );
	void				uninitCdmiFuse();
private:
	CdmiFuseOps*		mCdmiOps;
};

class C2PyException : public std::exception {
public:
	C2PyException() {
	}
	virtual ~C2PyException() throw (){
	}
	C2PyException( const char* what, ... ) {
		char buf[1024];
		static int bufSize = sizeof(buf);
		va_list args;
		va_start(args,what);
		int nCount = vsnprintf(buf, sizeof(buf)-1, what, args);
		buf[nCount > (bufSize-1) ? (bufSize-1) : nCount] = '\0';
		va_end(args);
		message = buf;
	}
	virtual const char* what() const throw() {
		return message.c_str();
	}
public:
	std::string message;
};

void str2json( const std::string& str, Json::Value& val ) {
	Json::Reader r;
	if(!r.parse(str,val)) {
		throw C2PyException("failto turn string into json structure");
	}
}

std::string json2str( const Json::Value& val ) {
	Json::FastWriter w;
	return w.write(val);
}

const Json::Value& getJsonValue( const Json::Value& jArgs, const std::string& key ) {
	if( !jArgs.isMember(key) ) {
		throw C2PyException("value with key[%s] is not found",key.c_str());
	}
	return jArgs[key];
}

void getProperties( const Json::Value& jArgs, const std::string& key, CdmiFuseOps::Properties& props ) {
	const Json::Value& val = getJsonValue( jArgs, key );
	if (!val.isObject()) {
		throw C2PyException("key[%s] found, but with a wrong type",key.c_str());
	}
	Json::Value::Members names = val.getMemberNames();
	Json::Value::Members::const_iterator it = names.begin();
	for( ; it != names.end(); it ++ ) {
		const Json::Value& v = val[*it];
		if( !v.isString() ) {
			throw C2PyException("key[%s], name[%s], not a string", key.c_str(), it->c_str());
		}
		props[*it]  = v.asString();
	}
}

std::string getString( const Json::Value& jArgs, const std::string& key) {
	const Json::Value& val = getJsonValue( jArgs, key );
	if( !val.isString() ) {
		throw C2PyException("key[%s] found, but not a string",key.c_str());
	}
	return val.asString();
}

std::string getStringWithDefault( const Json::Value& jArgs, const std::string& key, const std::string& defaultvalue ) {
	try{
		return getString( jArgs, key );
	}
	catch( const C2PyException& ) {
		return defaultvalue;
	}
}

void getStrList( const Json::Value& jArgs, const std::string& key, CdmiFuseOps::StrList& sl ) {
	const Json::Value& val = getJsonValue( jArgs, key ) ;
	if( !val.isArray() ) {
		throw C2PyException("key[%s] found, but not an array",key.c_str());
	}
	size_t arrSize = val.size();
	for( size_t i = 0 ; i < arrSize; i ++ ) {
		const Json::Value& v = val[i];
		if( !v.isString() ) {
			throw C2PyException("key[%s] found, array item[%d] is not a string",key.c_str(), i+1);
		}
		sl.push_back( v.asString() );
	}
}

uint64 getUint( const Json::Value& jArgs, const std::string& key ) {
	const Json::Value& val = getJsonValue(jArgs,key);
	if( !val.isIntegral() ) {
		throw C2PyException("key[%s] found, but not an uint",key.c_str());
	}
	return val.asUInt64();
}

uint64 getUintWithDefault( const Json::Value& jArgs, const std::string& key, uint64 defaultValue ) {
	try {
		return getUint( jArgs, key );
	} catch ( const C2PyException&) {
		return defaultValue;
	}
}

int64 getInt( const Json::Value& jArgs, const std::string& key ) {
	const Json::Value& val = getJsonValue(jArgs,key);
	if( !val.isIntegral() ) {
		throw C2PyException("key[%s] found, but not an uint",key.c_str());
	}
	return val.asInt64();
}
int64 getIntWithDefault( const Json::Value& jArgs, const std::string& key, int64 defaultValue ) {
	try {
		return getInt( jArgs, key );
	}
	catch( const C2PyException&  ) {
		return defaultValue;
	}
}

NestedClient::NestedClient( const std::string& config, const std::string& rooturl, const std::string& userDomain, const std::string& homecontainer )
	:mCdmiOps(0)
{
	initCdmiFuse(config, rooturl, userDomain, homecontainer);
}

NestedClient::~NestedClient() {
	uninitCdmiFuse();
}

int readSizeFromString( const std::string& str ) {
    if(str.empty())
        return 0;
    int32 factor = 1;
    int32 result = atoi(str.c_str());
    switch(str.at(str.length()-1))
    {
        case 'b':
        case 'B':
            factor = 1; break;
        case 'k':
        case 'K':
            factor = 1024;break;
        case 'm':
        case 'M':
            factor = 1024 * 1024; break;
        case 'g':
        case 'G':
            factor = 1024 * 1024 * 1024; break;
        default:
            factor = 1; break;
    }
    result *= factor;
    return result;
}

void NestedClient::initCdmiFuse(const std::string& config, const std::string& rooturl, const std::string& userDomain, const std::string& homecontainer ) {
	Json::Value jConf;
	str2json( config, jConf);
	std::string log_file;
	int  log_level = 6;
	int  log_count = 10;
	long log_size = 10 * 1024 * 1024;
	int  thread_count = 10;
	CdmiFuseOps::readEnvConfig( jConf, log_file, log_level, log_size, log_count, thread_count );
	log_file = log_file +"/cdmiclient.log";
	thread_count = thread_count < 3 ? 3: thread_count;

	if(!mFileLogger)
		mFileLogger = new ZQ::common::FileLog(log_file.c_str(), log_level, log_count, log_size );
	if(!mThdPool)
		mThdPool = new ZQ::common::NativeThreadPool(thread_count);
	ZQ::common::CURLClient::startCurlClientManager();
	
	FuseOpsConf fuseConf;
	CdmiFuseOps::readClientConfig( jConf, fuseConf);
	mCdmiOps = new CdmiFuseOps(*mFileLogger,*mThdPool, rooturl, userDomain, homecontainer,15,fuseConf);

	clientCount ++;
}

void NestedClient::uninitCdmiFuse( ) {
	delete mCdmiOps;

	if( --clientCount <= 0 ) {
		ZQ::common::CURLClient::stopCurlClientManager();
		mThdPool->stop();
		delete mThdPool;
		delete mFileLogger;
	}
}

std::string NestedClient::pathToUrl( const std::string& path ) {
	assert( mCdmiOps != 0 );
	return mCdmiOps->pathToUri( path );
}

int NestedClient::call_cdmi_createDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert(mCdmiOps != 0 );
	Json::Value result;
	std::string mimetype = getString( jArgs, "mimetype");
	CdmiFuseOps::Properties metadata;
	getProperties( jArgs, "metadata",metadata);
	std::string value = getString( jArgs, "value");
	CdmiFuseOps::StrList valuetransferencoding;
	getStrList( jArgs, "valuetransferencoding", valuetransferencoding );
	std::string domainURI = getString(jArgs, "domainURI");
	std::string deserialize = getString(jArgs, "deserialize");
	std::string serialize = getString(jArgs,"serialize");
	std::string copy = getString(jArgs,"copy");
	std::string move = getString(jArgs,"move");
	std::string reference = getString(jArgs,"reference");
	std::string deserializevalue = getString(jArgs,"deserializevalue");
	int rc =  mCdmiOps->cdmi_CreateDataObject( result, uri, mimetype, metadata, value, 
			valuetransferencoding, domainURI, deserialize, serialize, copy, move, reference, deserializevalue );
	jResult["result"] = result;
	return rc;
}

int NestedClient::call_cdmi_readDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	Json::Value result;
	std::string location;
	int rc = mCdmiOps->cdmi_ReadDataObject( result, uri, location );
	jResult["location"] = location;
	jResult["result"] = result;
	return rc;
}

int NestedClient::call_cdmi_updateDataObjectEx( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	std::string location;
	const Json::Value& metadata = getJsonValue( jArgs, "metadata");
	uint64 startOffset = getUint( jArgs, "startOffset" );
	std::string value = getString( jArgs, "value");
	int base_version = (int)getInt( jArgs, "base_version");
	bool partial = getInt( jArgs,"partial") != 0;
	CdmiFuseOps::StrList valuetransferencoding;
	getStrList( jArgs, "valuetransferencoding", valuetransferencoding);
	std::string domainURI = getString( jArgs, "domainURI");
	std::string deserialize = getString( jArgs, "deserialize");
	std::string copy = getString( jArgs, "copy");
	std::string deserializevalue = getString( jArgs, "deserializevalue");
	int rc = mCdmiOps->cdmi_UpdateDataObjectEx( location, uri, metadata, startOffset, value, base_version, partial, valuetransferencoding,
			domainURI, deserialize, copy, deserializevalue );
	jResult["location"] = location;
	return rc;
}

int NestedClient::call_cdmi_updateDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	std::string location;
	CdmiFuseOps::Properties metadata;
	getProperties( jArgs,"metadata", metadata );
	uint64 startOffset = getUint( jArgs, "startOffset");
	std::string value = getString( jArgs, "value");
	bool partial = getInt( jArgs, "partial") != 0 ;
	CdmiFuseOps::StrList valuetransferencoding;
	getStrList( jArgs ,"valuetransferencoding",valuetransferencoding);
	std::string domainURI = getString( jArgs, "domainURI");
	std::string deserialize = getString( jArgs, "deserialize");
	std::string copy = getString(jArgs,"copy");
	std::string deserializevalue = getString( jArgs, "deserializevalue" );
	int rc = mCdmiOps->cdmi_UpdateDataObject( location, uri, metadata, startOffset, value, partial,
			valuetransferencoding, domainURI, deserialize, copy, deserializevalue );
	jResult["location"] = location;
	return rc;
}

int NestedClient::call_cdmi_deleteDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	return mCdmiOps->cdmi_DeleteDataObject( jResult, uri );
}

int NestedClient::call_cdmi_createContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	Json::Value result;
	CdmiFuseOps::Properties metadata;
	getProperties( jArgs, "metadata", metadata );
	const Json::Value& exports = getJsonValue( jArgs,"exports");
	std::string domainURI = getString( jArgs, "domainURI");
	std::string deserialize = getString( jArgs, "deserialize");
	std::string copy = getString( jArgs, "copy");
	std::string move = getString( jArgs, "move");
	std::string reference = getString( jArgs, "reference");
	std::string deserializevalue = getString( jArgs, "deserializevalue");
	int rc = mCdmiOps->cdmi_CreateContainer( result, uri, metadata, exports, domainURI, deserialize,
			copy, move, reference, deserializevalue);
	jResult["result"] = result;
	return rc;
}

int NestedClient::call_cdmi_readContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	Json::Value result;
	int rc= mCdmiOps->cdmi_ReadContainer( result, uri );
	jResult["result"] = result;
	return rc;
}

int NestedClient::call_cdmi_updateContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	std::string location;
	Json::Value result;
	CdmiFuseOps::Properties metadata;
	getProperties( jArgs, "metadata", metadata);
	const Json::Value& exports = getJsonValue( jArgs,"exports");
	std::string domainURI = getString( jArgs, "domainURI");
	std::string deserialize = getString( jArgs, "deserialize");
	std::string copy = getString(jArgs,"copy");
	std::string snapshot = getString( jArgs, "snapshot");
	std::string deserializevalue = getString( jArgs, "deserializevalue");
	int rc = mCdmiOps->cdmi_UpdateContainer( result, location, uri, metadata, exports, domainURI, deserialize,
			copy, snapshot, deserializevalue );
	jResult["result"] = result;
	jResult["location"] = location;
	return rc;
}

int NestedClient::call_cdmi_deleteContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps != 0 );
	Json::Value result;
	int rc = mCdmiOps->cdmi_DeleteContainer( result, uri );
	jResult["result"] = result;
	return rc;
}

int NestedClient::call_cdmi_readDomain( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult ) {
	assert( mCdmiOps!= 0);
	Json::Value result;
	std::string domainURI = uri;
	int rc = mCdmiOps->cdmi_ReadAquaDomain( result, domainURI);
	jResult["result"] = result;
	return rc;
}


int NestedClient::call_noncdmi_createDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0 );
	bufLen = 0;
	std::string contentType = getString( jArgs, "contentType");
	std::string value = getString( jArgs, "value");
	int rc= mCdmiOps->nonCdmi_CreateDataObject( uri, contentType, value.c_str(), value.length());
	jResult["contentType"] = contentType;
	return rc;
}

int NestedClient::call_noncdmi_readDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0 );
	uint32 len = bufLen;
	std::string location,contentType;
	uint64 startOffset = getUint( jArgs, "startOffset");
	bool disableCache = getInt( jArgs, "disableCache") != 0;
	int rc = mCdmiOps->nonCdmi_ReadDataObject( uri, contentType, location, startOffset, len, buf, disableCache );
	jResult["contentType"] = contentType;
	jResult["location"] = location;
	jResult["size"] = len;
	bufLen = len;
	return rc;
}

int NestedClient::call_noncdmi_updateDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0 );
	std::string location;
	std::string contentType = getString( jArgs, "contentType");
	uint64 startOffset = getUint( jArgs, "startOffset");
	int64 objectSize = getInt( jArgs, "objectSize");
	bool partial = getInt(jArgs, "partial") != 0;
	bool disableCache = getInt( jArgs, "disableCache") != 0;
	int rc = mCdmiOps->nonCdmi_UpdateDataObject( uri, location, contentType, startOffset, bufLen, buf, objectSize, partial, disableCache);
	jResult["location"] = location;
	return rc;
}

int NestedClient::call_noncdmi_deleteDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0 );
	bufLen = 0;
	int rc = mCdmiOps->nonCdmi_DeleteDataObject( uri );
	return rc;
}

int NestedClient::call_noncdmi_createContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0 );
	return mCdmiOps->nonCdmi_CreateContainer( uri );
}

int NestedClient::call_noncdmi_deleteContainer( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0);
	bufLen = 0;
	return mCdmiOps->nonCdmi_DeleteContainer( uri );
}

int NestedClient::call_noncdmi_flushDataObject( const std::string& uri, const Json::Value& jArgs, Json::Value& jResult, char* buf, int& bufLen ) {
	assert( mCdmiOps != 0);
	bufLen = 0;
	return mCdmiOps->flushdata( uri );
}
#ifdef ZQ_OS_MSWIN
#define strcasecmp stricmp
#endif

#define STRSWITCH()     if(0) {
#define STRCASE(x)      } else  if( strcasecmp(x,cmd.c_str()) == 0 ) {
#define STRENDSWITCH()  }

int NestedClient::exec_Cdmi( const std::string& cmd, const std::string& uri, const std::string& jsonArgs, std::string& jsonResult ) {
	Json::Value jResult;
	Json::Value jArgs;
	str2json( jsonArgs, jArgs );

	int (NestedClient::*call_func)( const std::string&, const Json::Value&, Json::Value& );
	STRSWITCH()
		STRCASE("createDataObject")
		{
			call_func = &NestedClient::call_cdmi_createDataObject;
		}
		STRCASE("readDataObject")
		{
			call_func = &NestedClient::call_cdmi_readDataObject;
		}
		STRCASE("updateDataObjectEx")
		{
			call_func = &NestedClient::call_cdmi_updateDataObjectEx;
		}
		STRCASE("updateDataObject")
		{
			call_func = &NestedClient::call_cdmi_updateDataObject;
		}
		STRCASE("deleteDataObject")
		{
			call_func = &NestedClient::call_cdmi_deleteDataObject;
		}
		STRCASE("createContainer")
		{
			call_func = &NestedClient::call_cdmi_createContainer;
		}
		STRCASE("readContainer")
		{
			call_func = &NestedClient::call_cdmi_readContainer;
		}
		STRCASE("updateContainer")
		{
			call_func = &NestedClient::call_cdmi_updateContainer;
		}
		STRCASE("deleteContainer")
		{
			call_func = &NestedClient::call_cdmi_deleteContainer;
		}
		STRCASE("readDomain")
		{
			call_func = &NestedClient::call_cdmi_readDomain;
		}
	STRENDSWITCH()

	int rc = (this->*call_func)( uri, jArgs, jResult );
	jsonResult = json2str( jResult);
	return rc;
}

int NestedClient::exec_nonCdmi( const std::string& cmd, const std::string& uri, const std::string& jsonArgs, std::string& jsonResult, char* buf, int& bufLen ) {
	Json::Value jArgs;
	Json::Value jResult;

	str2json( jsonArgs, jArgs );

	int (NestedClient::*call_func)( const std::string&, const Json::Value&, Json::Value&, char* ,int& );
	STRSWITCH()
		STRCASE("createDataObject")
		{
			call_func = &NestedClient::call_noncdmi_createDataObject;
		}
		STRCASE("readDataObject")
		{
			call_func = &NestedClient::call_noncdmi_readDataObject;
		}
		STRCASE("updateDataObject")
		{
			call_func = &NestedClient::call_noncdmi_updateDataObject;
		}
		STRCASE("deleteDataObject")
		{
			call_func = &NestedClient::call_noncdmi_deleteDataObject;
		}
		STRCASE("createContainer")
		{
			call_func = &NestedClient::call_noncdmi_createContainer;
		}
		STRCASE("deleteContainer")
		{
			call_func = &NestedClient::call_noncdmi_deleteContainer;
		}
		STRCASE("flushDataObject")
		{
			call_func = &NestedClient::call_noncdmi_flushDataObject;
		}
	STRENDSWITCH()
	int rc=  (this->*call_func)( uri, jArgs, jResult, buf, bufLen );
	jsonResult = json2str( jResult);
	return rc;
}


static tuple do_Cdmi( NestedClient& self, 
					const std::string& cmd, const std::string& uri,
					const std::string& jsonArg) {
	std::string jsonResult = "{}";
	int rc = self.exec_Cdmi( cmd, self.pathToUrl( uri ), jsonArg, jsonResult);
	return make_tuple( rc, jsonResult );
}

static tuple do_nonCdmi( NestedClient& self, 
					const std::string& cmd, const std::string& uri, 
					const std::string& jsonArg, object buffer ) {
	std::string jsonResult="{}";
	int bufLen = 0;
	Py_buffer py_buffer; 
	int py_flag = PyBUF_WRITABLE|PyBUF_C_CONTIGUOUS; //PyBUF_SIMPLE
	if( strcasecmp( cmd.c_str(), "readDataObject")  != 0 )
		py_flag = PyBUF_SIMPLE;

	if( 0 != PyObject_GetBuffer( buffer.ptr(), &py_buffer, py_flag )) {
		return make_tuple(-3,jsonResult);
	}
	char* cxx_buf = (char*)py_buffer.buf;
	bufLen = py_buffer.len;
	int rc = self.exec_nonCdmi( cmd, self.pathToUrl(uri), jsonArg, jsonResult, cxx_buf, bufLen );
	py_buffer.len = bufLen;
	return make_tuple(rc,jsonResult);
}

void translate( C2PyException const& e)
{
	// Use the Python 'C' API to set up an exception object
	PyErr_SetString( PyExc_RuntimeError, e.what());
}


BOOST_PYTHON_MODULE(AquaClient) {
		
	register_exception_translator<C2PyException>(&translate);

	class_<NestedClient>("NestedClient",init<std::string,std::string,std::string,std::string>())
		.def("doCdmi",	  &do_Cdmi,    ( arg("self"), arg("cmd"), arg("uri"), arg("jsonArg") ) ) 
		.def("doNonCdmi", &do_nonCdmi, ( arg("self"), arg("cmd"), arg("uri"), arg("jsonArg"), arg("buffer") ) )
		;
}
