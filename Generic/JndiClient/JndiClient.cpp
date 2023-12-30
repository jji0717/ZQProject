// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: JndiClient.java, hui.shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JndiClient/JndiClient.cpp $
// 
// 5     1/22/14 10:56a Build
// 
// 4     1/20/14 12:55p Hui.shao
// 
// 3     1/02/14 5:45p Hui.shao
// pglog
// 
// 2     1/09/12 12:46p Hui.shao
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 18    10-08-31 18:14 Xiaohui.chai
// 
// 17    10-07-08 15:55 Xiaohui.chai
// 
// 16    10-04-09 17:47 Xiaohui.chai
// "-server" not work, roll back
// 
// 14    10-03-04 15:48 Fei.huang
// * add option to disable java signal handler
// 
// 13    10-03-03 18:00 Xiaohui.chai
// fix the thread attach issue
// 
// 12    10-03-01 17:47 Xiaohui.chai
// 
// 11    10-02-24 18:16 Fei.huang
// 
// 10    10-02-24 17:10 Hui.shao
// supported message properties
// 
// 9     10-02-22 20:30 Hui.shao
// 
// 8     10-02-22 19:55 Hui.shao
// supported setProducerOptions; value types of MapMessage covered Integer
// and Boolean; todo:OnMapMessage() need more works
// 
// 7     10-02-22 15:11 Hui.shao
// sinks the event of connection-establish and connection-lost
// 
// 6     10-02-21 18:35 Hui.shao
// added the support of MapMessage
// 
// 5     10-02-10 15:38 Fei.huang
// 
// 4     10-02-10 15:36 Fei.huang
// * fix: jstring -> wchar convertion
// 
// 3     10-02-09 14:14 Fei.huang
// + merge to linux
// 
// 2     10-02-04 18:46 Hui.shao
// reviewed the log printing on reconnecting
// 
// 1     10-02-04 16:51 Hui.shao
// created
// ===========================================================================

#include "ZQ_common_conf.h"
#include "Locks.h"
#include "JndiClient.h"
#include "DynSharedObj.h"
// #include "JavaVM.h"
#include "NestedJndiClient.h"
#include "NestedJmsSession.h"
// #include "NestedTopicClient.h"
#include <NativeThread.h>
#include <SystemUtils.h>
#include <vector>

#ifdef ZQ_OS_MSWIN
HANDLE	_gCurrentModule=NULL;
#endif


namespace ZQ {
namespace JndiClient {

//
// JVM daemon thread
//
class JVMDaemonThread: public ZQ::common::NativeThread
{
public:
    JVMDaemonThread(ZQ::common::DynSharedObj &so, const std::string& clspath);
    ~JVMDaemonThread();

    JavaVM* getJVM();
protected:
    virtual int run();
private:
    ZQ::common::DynSharedObj &so_;
    std::string clspath_;
    JavaVM* pJVM_;
    SYS::SingleObject created_;
    SYS::SingleObject toDestroy_;
};
// -----------------------------
// internal class JVM
// -----------------------------
class JVM
{
	friend class ClientContext;
	friend class JmsSession;
public:
	JVM();
	virtual ~JVM();

	static int JNICALL exitHook(jint code);

	bool isReady();
    JNIEnv* getEnv();

	bool init(ZQ::common::Log& logger, const char* jvmSOFilename, const char* classpath=NULL);
	void uninit();

	jclass loadClass(const char *clsname, JNIEnv* jenv);

	jstring newJstring(const ::std::string& str);
	std::string readJstring(jstring jstr, JNIEnv * jenv=NULL);
	jobjectArray newJstringArray(const ::std::vector< ::std::string >& strs);
	int readJstringArray(::std::vector< ::std::string >& strs, jobjectArray jstrs, JNIEnv * jenv=NULL);

	void registerNativeObj(const std::string& instanceId, const void* nobj, const JNIEnv *jenv =NULL);
	void unregisterNativeObj(const std::string& instanceId, const JNIEnv *jenv =NULL);
	const void* findNativeObj(const std::string& instanceId, const JNIEnv *jenv =NULL);

	void log(int level, const char *fmt, ...);

    void attachCurrentThread() {
        JNIEnv* penv;
        _jvm->AttachCurrentThread((void**)&penv, NULL);
    }

    void detachCurrentThread() {
        _jvm->DetachCurrentThread();
    }
protected:
	ZQ::common::DynSharedObj _so;

	JavaVM*     _jvm;
    JVMDaemonThread* _jvmDaemon;

	typedef ::std::map <std::string, const void* > InstanceId2NativeMap; //instanceId to NativeObj map
	InstanceId2NativeMap _instId2nativeMap;
	ZQ::common::Mutex _lkInstId2nativeMap;

	ZQ::common::Log* _pLog;
};

JVM gJVM;

class JVMThreadGuard
{
public:
    JVMThreadGuard(JVM& jvm):jvm_(jvm) {
        jvm_.attachCurrentThread();
    }
    ~JVMThreadGuard() {
        jvm_.detachCurrentThread();
    }
private:
    JVM& jvm_;
};
/// -----------------------------
/// class JndiException
/// -----------------------------
JndiException::JndiException(const std::string &what_arg) throw()
            :Exception(what_arg)
{
}

JndiException::~JndiException() throw()
{
}

// -----------------------------
// class ClientContext
// -----------------------------
bool ClientContext::initJVM(ZQ::common::Log& log, const char* classpath, const char* jvmsofile) // static
{
	if (gJVM.isReady())
	{
		log(ZQ::common::Log::L_WARNING, CLOGFMT(ClientContext, "initVM() JVM has already loaded, quit the initialization"));
		return true;
	}

	if (NULL == jvmsofile)
		jvmsofile = "jvm.dll";

	gJVM.init(log, jvmsofile, classpath);

	return gJVM.isReady();
}

void ClientContext::uninitJVM()
{
	gJVM.uninit();
}

ClientContext::ClientContext(const ::std::string& serverUrl, const ZQ::common::Log::loglevel_t javaTraceLevel, const Properties& properties) throw(JndiException)
							 :_nestedCls(NULL), _nestedObj(NULL), _props(properties), _javaTraceLevel(javaTraceLevel)
{
	char buf[30];
	snprintf(buf, sizeof(buf)-2, "%p", this);
	_instanceId = buf;

    JVMThreadGuard jguard(gJVM);
    JNIEnv* jenv = gJVM.getEnv();

	if (!gJVM.isReady())
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "ClientContext() constructing before JVM is initialized");

	::std::vector<std::string > params;
	::std::string paramstr;
	for (Properties::const_iterator it = _props.begin(); it != _props.end(); it++)
	{
		if (!it->first.empty())
		{
			params.push_back(it->first + "=" + it->second);
			paramstr += it->first + "[" + it->second + "]; ";
		}
	}

	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientContext, "initializing to server[%s] with params: %s"), serverUrl.c_str(), paramstr.c_str());

    if(NULL == jenv)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "ClientContext() Can't get jni env object");

	_nestedCls = gJVM.loadClass("NestedJndiClient", jenv);
	if (NULL == _nestedCls)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "ClientContext() failed to load JAVA class NestedJndiClient");

	// registering the native methods of NestedJndiClient
	JNINativeMethod nm;
	nm.name = "_nativelog";
	nm.signature = "(ILjava/lang/String;)V";
	nm.fnPtr = (void*) Java_NestedJndiClient__1nativelog;
	jenv->RegisterNatives((jclass) _nestedCls, &nm, 1);

	jclass _jSess = gJVM.loadClass("NestedJmsSession", jenv);
	if (NULL == _jSess)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "ClientContext() failed to load JAVA class NestedJmsSession");

	// registering the native methods of NestedJmsSession

	// NestedJmsSession::_notifyConnected
	nm.name = "_notifyConnected";
	nm.signature = "(Ljava/lang/String;Ljava/lang/String;)V";
	nm.fnPtr = (void*) Java_NestedJmsSession__1notifyConnected;
	jenv->RegisterNatives(_jSess, &nm, 1);

	// NestedJmsSession::_forwardConnectionException
	nm.name = "_forwardConnectionException";
	nm.signature = "(Ljava/lang/String;Ljava/lang/String;)V";
	nm.fnPtr = (void*) Java_NestedJmsSession__1forwardConnectionException;
	jenv->RegisterNatives(_jSess, &nm, 1);

	// NestedJmsSession::_dispatchTextMessage
	nm.name = "_dispatchTextMessage";
	nm.signature = "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V";
	nm.fnPtr = (void*) Java_NestedJmsSession__1dispatchTextMessage;
	jenv->RegisterNatives(_jSess, &nm, 1);

	// NestedJmsSession::_dispatchMapMessage
	nm.name = "_dispatchMapMessage";
	nm.signature = "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V";
	nm.fnPtr = (void*) Java_NestedJmsSession__1dispatchMapMessage;
	jenv->RegisterNatives(_jSess, &nm, 1);

	// calling static NestedJndiClient::initContext(String providerUrl, String instanceId, String params[])
#define SIG_initContext "(ILjava/lang/String;Ljava/lang/String;[Ljava/lang/String;)LNestedJndiClient;"
	jmethodID methodId = jenv->GetStaticMethodID((jclass) _nestedCls, "initContext", SIG_initContext);
	if (methodId == NULL)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "ClientContext() failed to find NestedJndiClient::initContext" SIG_initContext);

	jstring jserverUrl    = gJVM.newJstring(serverUrl);
	jstring jinstanceId   = gJVM.newJstring(_instanceId);
	jobjectArray jparams = gJVM.newJstringArray(params);

    jobject localRef = jenv->CallStaticObjectMethod(((jclass) _nestedCls), methodId, _javaTraceLevel&0x07, jserverUrl, jinstanceId, jparams);

    if (NULL == localRef)
        ZQ::common::_throw<JndiException>((*gJVM._pLog), "ClientContext() call NestedJndiClient::initContext() failed");

    // convert it to a global reference
    _nestedObj = jenv->NewGlobalRef(localRef);
    jenv->DeleteLocalRef(localRef);


	gJVM.registerNativeObj(_instanceId, this);
	gJVM.log(ZQ::common::Log::L_INFO, CLOGFMT(ClientContext, "ctx[%p] created to server[%s]"), _nestedObj, serverUrl.c_str());
}

ClientContext::~ClientContext()
{
	gJVM.unregisterNativeObj(_instanceId);
	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(ClientContext, "ctx[%p] unregistered"), _nestedObj);
}


// -----------------------------
// class JmsSession
// -----------------------------
JmsSession::JmsSession(ClientContext& context, DestType destType, const std::string& destName, bool asProducer, bool asConsumer) 
		throw(JndiException)
		: _context(context)
{
	char buf[30];
	snprintf(buf, sizeof(buf)-2, "%p", this);
	_instanceId = buf;

	_destDescription = (destType == DT_Queue) ? "queue[" : "topic[";
	_destDescription += destName +"]";
	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(JmsSession, "opening session to %s"), _destDescription.c_str());

    JVMThreadGuard jguard(gJVM);
    JNIEnv* jenv = gJVM.getEnv();
    if(NULL == jenv)
        ZQ::common::_throw<JndiException>((*gJVM._pLog), "JmsSession() Can't get jni env object");

	_nestedCls = gJVM.loadClass("NestedJmsSession", jenv);
	if (NULL == _nestedCls)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "JmsSession() failed to load JAVA class NestedJmsSession");

#define SIG_CreateJmsSession "(Ljava/lang/String;Ljava/lang/String;ZZ)LNestedJmsSession;"

	::std::string createMethodName = (destType == DT_Queue) ? "createJmsQueueSession" : "createJmsTopicSession";
	jstring jinstanceId   = gJVM.newJstring(_instanceId);
	jstring jdestName   = gJVM.newJstring(destName);
	jboolean jasProducer  = asProducer ? 1:0;
	jboolean jasConsumer  = asConsumer ? 1:0;

	jmethodID methodId = jenv->GetMethodID((jclass) _context._nestedCls, createMethodName.c_str(), SIG_CreateJmsSession);
	if (methodId == NULL)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "JmsSession() failed to find NestedJndiClient::%s(" SIG_CreateJmsSession ")", createMethodName.c_str());

	jobject localRef = jenv->CallObjectMethod(((jobject) _context._nestedObj), methodId, jinstanceId, jdestName, jasProducer, jasConsumer);

    if (NULL == localRef)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "JmsSession() call NestedJndiClient::%s() failed", createMethodName.c_str());

    // convert it to a global reference
    _nestedObj = jenv->NewGlobalRef(localRef);
    jenv->DeleteLocalRef(localRef);

	gJVM.registerNativeObj(_instanceId, this);
	gJVM.log(ZQ::common::Log::L_INFO, CLOGFMT(JmsSession, "sess[%p] created to %s"), _nestedObj, _destDescription.c_str());
}

JmsSession::~JmsSession()
{
    JVMThreadGuard jguard(gJVM);
    JNIEnv* jenv = gJVM.getEnv();

	gJVM.unregisterNativeObj(_instanceId);

	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(JmsSession, "sess[%p] stopping reconnect thread"), _nestedObj);
	jmethodID methodId = jenv->GetMethodID((jclass) _nestedCls, "deactive", "()V");
	jenv->CallVoidMethod(((jobject) _nestedObj), methodId);

	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(JmsSession, "sess[%p] unregistered"), _nestedObj);
}

bool JmsSession::setProducerOptions(int priority, long messageTTL, DeliveryMode deliveryMode, bool disableMessageID, bool disableMessageTimestamp)
{
	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(JmsSession, "setProducerOptions() priority[%d], messageTTL[%d], deliveryMode[%d], disableMessageID[%c], disableMessageTimestamp[%c]"), 
		priority, messageTTL, deliveryMode, disableMessageID?'T':'F', disableMessageID?'T':'F');

    JVMThreadGuard jguard(gJVM);
    JNIEnv* jenv = gJVM.getEnv();

	jmethodID methodId = jenv->GetMethodID((jclass) _nestedCls, "setProducerOptions", "(ZIZZIJ)Z");
	if (methodId == NULL)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "failed to find NestedJmsSession::setProducerOptions" "(ZIZZIJ)Z");

	jboolean ret = jenv->CallBooleanMethod(((jobject) _nestedObj), methodId, JNI_FALSE, (jint) deliveryMode, (disableMessageID?JNI_TRUE:JNI_FALSE), (disableMessageTimestamp?JNI_TRUE:JNI_FALSE), (jint)priority, (jlong)messageTTL);
	if (JNI_FALSE != ret)
		return true;

	gJVM.log(ZQ::common::Log::L_WARNING, CLOGFMT(JmsSession, "setProducerOptions() failed on %s"), _destDescription.c_str());
	return false;
}

bool JmsSession::sendTextMessage(const std::string& message, const ClientContext::Properties& msgProps)
{
	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(JmsSession, "sending text message to %s"), _destDescription.c_str());

    JVMThreadGuard jguard(gJVM);
    JNIEnv* jenv = gJVM.getEnv();

#define SIG_sendText "([Ljava/lang/String;Ljava/lang/String;)Z"
	jmethodID methodId = jenv->GetMethodID((jclass) _nestedCls, "sendText", SIG_sendText);
	if (methodId == NULL)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "sendTextMessage() failed to find NestedJmsSession::sendText" SIG_sendText );

	::std::vector<std::string > msgPropsExpressions;
	for (MapMessage::const_iterator it = msgProps.begin(); it != msgProps.end(); it++)
	{
		::std::string expression = it->first + "=" + it->second;
		msgPropsExpressions.push_back(expression);
	}
	jobjectArray jmsgPropsExpressions = gJVM.newJstringArray(msgPropsExpressions);

	jstring jmsg   = gJVM.newJstring(message);
	jboolean ret = jenv->CallBooleanMethod(((jobject) _nestedObj), methodId, jmsgPropsExpressions, jmsg);

	if (JNI_FALSE != ret)
		return true;

	gJVM.log(ZQ::common::Log::L_WARNING, CLOGFMT(JmsSession, "failed to send text message to %s"), _destDescription.c_str());
	return false;
}

bool JmsSession::sendMapMessage(const MapMessage& message, const ClientContext::Properties& msgProps)
{
	gJVM.log(ZQ::common::Log::L_DEBUG, CLOGFMT(JmsSession, "sending map message to %s"), _destDescription.c_str());

    JVMThreadGuard jguard(gJVM);
    JNIEnv* jenv = gJVM.getEnv();

#define SIG_sendMap "([Ljava/lang/String;[Ljava/lang/String;)Z"
	jmethodID methodId = jenv->GetMethodID((jclass) _nestedCls, "sendMap", SIG_sendMap);
	if (methodId == NULL)
		ZQ::common::_throw<JndiException>((*gJVM._pLog), "sendMapMessage() failed to find NestedJmsSession::sendMap" SIG_sendMap);

	::std::vector<std::string > msgPropsExpressions;
	for (MapMessage::const_iterator it = msgProps.begin(); it != msgProps.end(); it++)
	{
		::std::string expression = it->first + "=" + it->second;
		msgPropsExpressions.push_back(expression);
	}
	jobjectArray jmsgPropsExpressions = gJVM.newJstringArray(msgPropsExpressions);

	::std::vector<std::string > params;
	for (MapMessage::const_iterator it = message.begin(); it != message.end(); it++)
	{
		::std::string expression = it->first + "=" + it->second;
		params.push_back(expression);
	}

	jobjectArray jparams = gJVM.newJstringArray(params);
	jboolean ret = jenv->CallBooleanMethod(((jobject) _nestedObj), methodId, jmsgPropsExpressions, jparams);

	if (JNI_FALSE != ret)
		return true;

	gJVM.log(ZQ::common::Log::L_WARNING, CLOGFMT(JmsSession, "failed to send map message to %s"), _destDescription.c_str());
	return false;
}

void JmsSession::setProperty(ClientContext::Properties& message, const char* key, const char* value)
{
	setProperty(message, ::std::string(key ? key :""), ::std::string(value ? value :""));
}

void JmsSession::setIntegerProperty(ClientContext::Properties& message, const char* key, long value)
{
	char buf[64];
	snprintf(buf, sizeof(buf)-2, "$I$%ld", value);
	setProperty(message, ::std::string(key ? key :""), ::std::string(buf));
}

void JmsSession::setBooleanProperty(ClientContext::Properties& message, const char* key, bool value)
{
	char buf[8];
	snprintf(buf, sizeof(buf)-2, "$B$%d", value?1:0);
	setProperty(message, ::std::string(key ? key :""), ::std::string(buf));
}

void JmsSession::setProperty(ClientContext::Properties& message, const ::std::string& key, const ::std::string& value)
{
	if (key.empty())
		return;

	if (message.end() == message.find(key))
		message.insert(MapMessage::value_type(key, value));
	else message[key] = value;
}

// -----------------------------
// class Hotspot
// -----------------------------
class Hotspot : public ZQ::common::DynSharedFacet
{
	DECLARE_DSOFACET(Hotspot, ZQ::common::DynSharedFacet);

	DECLARE_PROC_WITH_APITYPE(jint, JNICALL, createJavaVM, (JavaVM **pvm, void **jenv, void *args));
	DECLARE_PROC_WITH_APITYPE(jint, JNICALL, getDefaultJavaVMInitArgs, (void *args));

	DSOFACET_PROC_BEGIN()
		DSOFACET_PROC_SPECIAL(createJavaVM, "JNI_CreateJavaVM");
		DSOFACET_PROC_SPECIAL(getDefaultJavaVMInitArgs, "JNI_GetDefaultJavaVMInitArgs");
	DSOFACET_PROC_END();
};


//
// JVM daemon thread
//
JVMDaemonThread::JVMDaemonThread(ZQ::common::DynSharedObj &so, const std::string& clspath)
    :so_(so), clspath_(clspath), pJVM_(NULL)
{
    start();
}
JVMDaemonThread::~JVMDaemonThread() {
    toDestroy_.signal();
    waitHandle(-1);
}

JavaVM* JVMDaemonThread::getJVM() {
    created_.wait(-1);
    created_.signal(); // keep the signaled state
    return pJVM_;
}
int JVMDaemonThread::run()
{
    JavaVMOption jopts[3];
    memset(jopts, 0x00, sizeof(jopts));
    jopts[0].optionString = "exit";
    jopts[0].extraInfo = (void*) JVM::exitHook;
    jopts[1].optionString = (char*)clspath_.c_str();
    jopts[2].optionString = "-Xrs";

    JavaVMInitArgs jargs;
    memset(&jargs, 0, sizeof(jargs));
    jargs.version  = JNI_VERSION_1_4;
    jargs.ignoreUnrecognized = JNI_FALSE;
    jargs.nOptions = 3;
    jargs.options  = jopts;

    Hotspot hs(so_);
    JNIEnv* jenv = NULL;
    if(JNI_OK == hs.createJavaVM(&pJVM_, (void **)&jenv, &jargs)) {
        created_.signal();
        toDestroy_.wait(-1);
        pJVM_->DestroyJavaVM();
        return 0;
    } else {
        pJVM_ = NULL;
        return -1;
    }
}

// -----------------------------
// class JVM
// -----------------------------
JVM::JVM()
    : _jvm(NULL), _pLog(NULL)
{
    _jvmDaemon = NULL;
}

JVM::~JVM()
{
	uninit();
}

int JNICALL JVM::exitHook(jint code)
{
	return 0;
}

bool JVM::isReady()
{
	return (NULL !=_jvm);
}

bool JVM::init(ZQ::common::Log& logger, const char* jvmSOFilename, const char* classpath)
{
	if (isReady())
	{
		logger(ZQ::common::Log::L_ERROR, CLOGFMT(JndiClient_JVM, "init() JVM has already initialized, can not be repeated in the same process"));
		return false;
	}

	_pLog = &logger;

	// load DLL
	if (_so.isLoaded())
		return false;
#ifndef SO_EXTNAME
#ifdef ZQ_OS_MSWIN
#  define SO_EXTNAME ".dll"
#else
#  define SO_EXTNAME ".so"
#endif // ZQ_OS_MSWIN
#endif // SO_EXTNAME

	if (jvmSOFilename==NULL|| *jvmSOFilename==0x00)
		jvmSOFilename = "jvm" SO_EXTNAME;

	// load the JVM.dll
	log(ZQ::common::Log::L_DEBUG, CLOGFMT(JndiClient_JVM, "loading %s"), jvmSOFilename);
	if (!_so.load(jvmSOFilename))
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(JndiClient_JVM, "failed to load %s"), jvmSOFilename);
		return false;
	}

	char classpathstr[MAX_MSG_LENGTH];
	snprintf(classpathstr, sizeof(classpathstr)-2, "-Djava.class.path=%s", (classpath==NULL|| *classpath==0x00)?".":classpath);
#ifndef ZQ_OS_MSWIN
	for (int i=0; i< MAX_MSG_LENGTH && classpathstr[i]; i++)
	{
		if (';' == classpathstr[i])
			classpathstr[i] = PHSEPC;
	}
#endif // ZQ_OS_MSWIN

	// initializing JVM
	log(ZQ::common::Log::L_DEBUG, CLOGFMT(JndiClient_JVM, "initializing JVM with classpath[%s]"), classpathstr);
    /*
	JavaVMOption jopts[3];
	memset(jopts, 0x00, sizeof(jopts));
	jopts[0].optionString = "exit";
    jopts[0].extraInfo = (void*) exitHook;
	jopts[1].optionString = classpathstr;

	JavaVMInitArgs jargs;
	memset(&jargs, 0, sizeof(jargs));
	jargs.version  = JNI_VERSION_1_4;
	jargs.ignoreUnrecognized = JNI_FALSE;
	jargs.nOptions = 2;
	jargs.options  = jopts;

	Hotspot hs(_so);
    JNIEnv* jenv = NULL;
	jint r = hs.createJavaVM(&_jvm, (void **)&jenv, &jargs);
    */
    _jvmDaemon = new JVMDaemonThread(_so, classpathstr);
    _jvm = _jvmDaemon->getJVM();
	if (NULL == _jvm)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(JndiClient_JVM, "initializing JVM with classpath[%s]"), classpathstr);
		return false;
	}
	return true;
}
void JVM::uninit()
{
	{
		ZQ::common::MutexGuard g(_lkInstId2nativeMap);
		_instId2nativeMap.clear();
	}

	if (NULL != _jvm)
	{
		//		DisplayThreads();
		delete _jvmDaemon;
	}

	_jvm = NULL;
	_pLog = NULL;
}
/*
jclass JVM::loadClass(const char *clsname)
{
    JNIEnv* jenv = getEnv();
	if (NULL == jenv || clsname==NULL || *clsname ==0x00)
		return NULL;

	::std::string str = clsname;
	for (size_t i =0; i < str.size(); i++)
		str[i] = (str[i] == '.') ? '/' : str[i];

	return jenv->FindClass(str.c_str());
}
*/
jclass JVM::loadClass(const char *clsname, JNIEnv* jenv)
{
	if (NULL == jenv || clsname==NULL || *clsname ==0x00)
		return NULL;

	::std::string str = clsname;
	for (size_t i =0; i < str.size(); i++)
		str[i] = (str[i] == '.') ? '/' : str[i];

	jclass localRefCls = jenv->FindClass(str.c_str());

	if (localRefCls == NULL)
		return NULL; 

	// Create a global reference
	jclass globalRefCls = (jclass)jenv->NewGlobalRef((jclass)localRefCls);

	//     if (_jenv != jenv && jenv !=NULL)
	//     {
	//              // the local reference is no longer useful
	//              _jenv->DeleteLocalRef(localRefCls);
	//              jenv->
	//     }

	jenv->DeleteLocalRef(localRefCls);
	return globalRefCls;
}

JNIEnv* JVM::getEnv() {
    JNIEnv* penv = NULL;
    if(JNI_OK == _jvm->GetEnv((void**)&penv, JNI_VERSION_1_4))
        return penv;

	return NULL;
}

jstring JVM::newJstring(const ::std::string& str)
{
    JNIEnv* jenv = getEnv();
	if (NULL == jenv)
		return NULL;

	jclass strClass  = jenv->FindClass("Ljava/lang/String;");
	jmethodID ctorID = jenv->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = jenv->NewByteArray((jsize)str.length());
	jenv->SetByteArrayRegion(bytes, 0, (jsize)str.length(), (const jbyte*)str.c_str());
	jstring encoding = jenv->NewStringUTF("utf-8");
	return (jstring) jenv->NewObject(strClass, ctorID, bytes, encoding);
}

jobjectArray JVM::newJstringArray(const ::std::vector< ::std::string >& strs)
{
    JNIEnv* jenv = getEnv();
	if (NULL == jenv)
		return NULL;

    jarray cls = (jarray)jenv->FindClass("java/lang/String");
    jarray ary = (jarray)jenv->NewObjectArray((jsize)strs.size(), (jclass)cls, 0);
	if (NULL == cls || NULL == ary)
		return NULL;

	for (size_t i =0; i < strs.size(); i++)
	{
		jstring str = newJstring(strs[i]);
		jenv->SetObjectArrayElement((jobjectArray) ary, (jsize)i, str);
		jenv->DeleteLocalRef(str);
    }

    return (jobjectArray) ary;
}


std::string JVM::readJstring(jstring jstr, JNIEnv* jenv)
{
	if (!jenv)
		jenv = getEnv();
	if(!jenv)
		return "";

	int   length        = jenv->GetStringLength(jstr);  
	const jchar*  jcstr = jenv->GetStringChars(jstr, 0);

	if(length <= 0 || !jcstr)
		return "";

	wchar_t* wbuf = 0;
	char buf[MAX_MSG_LENGTH];
#ifdef ZQ_OS_MSWIN
	wbuf = (wchar_t*)jcstr;
#else
	wbuf = (wchar_t*)malloc((length+1)*sizeof(wchar_t));
	memset(wbuf, '\0', (length+1)*sizeof(wchar_t));
	for(int i = 0; i < length; ++i)
		wbuf[i] = (wchar_t)jcstr[i];
#endif
	size_t size = wcstombs(buf, wbuf, sizeof(buf)-1);

#ifndef ZQ_OS_MSWIN
	free(wbuf);
#endif

	if (size < 0)
		return "";  

	jenv->ReleaseStringChars(jstr, jcstr);

	buf[size] = 0;  
	return buf;
}

int JVM::readJstringArray(::std::vector< ::std::string >& strs, jobjectArray jstrs, JNIEnv * jenv)
{
	strs.clear();

	if (!jenv)
		jenv = getEnv();
	if(!jenv)
		return 0;

	jsize len = jenv->GetArrayLength(jstrs);
	for (jsize i =0; i < len; i++)
	{
		jstring jstr = (jstring)jenv->GetObjectArrayElement(jstrs, i);
		std::string str = readJstring(jstr, jenv);
		strs.push_back(str);
	}

	return (int) strs.size();
}


void JVM::registerNativeObj(const std::string& instanceId, const void* nobj, const JNIEnv *jenv)
{
//	if ((NULL != jenv && _jenv != jenv) || NULL == nobj || NULL ==jobj)
	if (instanceId.empty())
		return;

	ZQ::common::MutexGuard g(_lkInstId2nativeMap);
	if (_instId2nativeMap.end() == _instId2nativeMap.find(instanceId))
		_instId2nativeMap.insert(InstanceId2NativeMap::value_type(instanceId, nobj));
	else _instId2nativeMap[instanceId] = nobj;
}

void JVM::unregisterNativeObj(const std::string& instanceId, const JNIEnv *jenv)
{
//	if ((NULL != jenv && _jenv != jenv) || NULL ==jobj)
	if (instanceId.empty())
		return;

	ZQ::common::MutexGuard g(_lkInstId2nativeMap);
	_instId2nativeMap.erase(instanceId);
}

const void* JVM::findNativeObj(const std::string& instanceId, const JNIEnv *jenv)
{
//	if ((NULL != jenv && _jenv != jenv) || NULL ==jobj)
	if (instanceId.empty())
		return NULL;

	ZQ::common::MutexGuard g(_lkInstId2nativeMap);
	InstanceId2NativeMap::iterator it = _instId2nativeMap.find(instanceId);
	if (_instId2nativeMap.end() == it)
		return NULL;

	return it->second;
}

void JVM::log(int level, const char *fmt, ...)
{
	if (NULL == _pLog)
		return;

	char msg[MAX_MSG_LENGTH];
	va_list args;

	va_start(args, fmt);
	int nCount = vsnprintf(msg, sizeof(msg)-2, fmt, args);
	va_end(args);

	if (nCount == -1)
		msg[MAX_MSG_LENGTH-1] = '\0';
	else
		msg[nCount] = '\0';

	(*_pLog)(level, "%s", msg);
}

}}

// -----------------------------
// JNI Entries
// -----------------------------
void JNICALL Java_NestedJndiClient__1nativelog(JNIEnv* jenv, jclass jcls, jint loglevel, jstring jmsg)
{
	::std::string message = ::ZQ::JndiClient::gJVM.readJstring(jmsg, jenv);
	::ZQ::JndiClient::gJVM.log(loglevel & 0x0f, "%s", message.c_str());
}

void JNICALL Java_NestedJmsSession__1notifyConnected(JNIEnv* jenv, jobject jobj, jstring jinstanceId, jstring jnotice)
{
	::std::string notice = ::ZQ::JndiClient::gJVM.readJstring(jnotice, jenv);
	::std::string instanceId = ::ZQ::JndiClient::gJVM.readJstring(jinstanceId, jenv);
	try {
		::ZQ::JndiClient::JmsSession* pSess = (::ZQ::JndiClient::JmsSession*) ::ZQ::JndiClient::gJVM.findNativeObj(instanceId, jenv);
		if (NULL == pSess)
			return;

		pSess->OnConnected(notice);
	}
	catch(...) {}
}

void JNICALL Java_NestedJmsSession__1forwardConnectionException(JNIEnv* jenv, jobject jobj, jstring jinstanceId, jstring jnotice)
{
	::std::string notice = ::ZQ::JndiClient::gJVM.readJstring(jnotice, jenv);
	::std::string instanceId = ::ZQ::JndiClient::gJVM.readJstring(jinstanceId, jenv);
	try {
		::ZQ::JndiClient::JmsSession* pSess = (::ZQ::JndiClient::JmsSession*) ::ZQ::JndiClient::gJVM.findNativeObj(instanceId, jenv);
		if (NULL == pSess)
			return;

		pSess->OnConnectionLost(notice);
	}
	catch(...) {}
}

static void readExpression(::ZQ::JndiClient::ClientContext::Properties& props, jobjectArray jexpressions, JNIEnv* jenv)
{
	::std::vector <std::string > expressions;
	::ZQ::JndiClient::gJVM.readJstringArray(expressions, jexpressions, jenv);
	
	for (size_t i =0; i < expressions.size(); i++)
	{
		size_t npos = expressions[i].find_first_of("=");
		std::string key  = expressions[i], value="";
		if (npos>0)
		{
			key = expressions[i].substr(0, npos);
			value = expressions[i].substr(npos+1);
		}

		if (key.length() >0)
			props.insert(::ZQ::JndiClient::ClientContext::Properties::value_type(key, value));
	}
}

void JNICALL Java_NestedJmsSession__1dispatchTextMessage(JNIEnv* jenv, jobject jobj, jstring jinstanceId, jstring jmsg, jobjectArray jmsgProps)
{
	::std::string message = ::ZQ::JndiClient::gJVM.readJstring(jmsg, jenv);
	::std::string instanceId = ::ZQ::JndiClient::gJVM.readJstring(jinstanceId, jenv);
	::ZQ::JndiClient::ClientContext::Properties msgProps;
	readExpression(msgProps, jmsgProps, jenv);
	
	try {
		::ZQ::JndiClient::JmsSession* pSess = (::ZQ::JndiClient::JmsSession*) ::ZQ::JndiClient::gJVM.findNativeObj(instanceId, jenv);
		if (NULL == pSess)
			return;

		pSess->OnTextMessage(message, msgProps);
	}
	catch(...) {}
}

void JNICALL Java_NestedJmsSession__1dispatchMapMessage(JNIEnv* jenv, jobject jobj, jstring jinstanceId, jobjectArray jexpressions, jobjectArray jmsgProps)
{
	::std::string instanceId = ::ZQ::JndiClient::gJVM.readJstring(jinstanceId, jenv);
	
	::ZQ::JndiClient::ClientContext::Properties msgProps, params;
	readExpression(msgProps, jmsgProps, jenv);
	readExpression(params, jexpressions, jenv);
	
	try {
		::ZQ::JndiClient::JmsSession* pSess = (::ZQ::JndiClient::JmsSession*) ::ZQ::JndiClient::gJVM.findNativeObj(instanceId, jenv);
		if (NULL == pSess)
			return;

		pSess->OnMapMessage(params, msgProps);
	}
	catch(...) {}
}


#ifdef ZQ_OS_MSWIN
// -----------------------------
// DLL Entries
// -----------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	_gCurrentModule=(HMODULE)hModule;
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif
