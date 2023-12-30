package com.xormedia.aqua.sdk;

import java.util.*;
import java.io.*;
import java.nio.*;

// http://json-lib.sourceforge.net/
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import net.sf.json.util.JSONUtils;

public class NestedClient {

	protected static final String NTAG_RET = "NTAG_RET";
	protected static final String NTAG_RESPBODY = "NTAG_RESPBODY";

	protected static final String NTAG_MIME_TYPE = "NTAG_MIME_TYPE";
	protected static final String NTAG_LOCATION = "NTAG_LOCATION";
	protected static final String NTAG_VALUE = "NTAG_VALUE";
	protected static final String NTAG_CONTENT_TYPE = "NTAG_CONTENT_TYPE";
	protected static final String NTAG_METADATA = "NTAG_METADATA";
	protected static final String NTAG_TRANSFER_ENCODING = "NTAG_TRANSFER_ENCODING";
	protected static final String NTAG_DOMAIN_URI = "NTAG_DOMAIN_URI";
	protected static final String NTAG_DESERIALIZE = "NTAG_DESERIALIZE";
	protected static final String NTAG_SERIALIZE = "NTAG_SERIALIZE";
	protected static final String NTAG_COPY = "NTAG_COPY";
	protected static final String NTAG_MOVE = "NTAG_MOVE";
	protected static final String NTAG_REFERENCE = "NTAG_REFERENCE";
	protected static final String NTAG_DESER_VALUE = "NTAG_DESER_VALUE";
	protected static final String NTAG_START_OFFSET = "NTAG_START_OFFSET";
	protected static final String NTAG_DIRECTIO = "NTAG_DIRECTIO";
	protected static final String NTAG_NBYTE_EXECUTED = "NTAG_NBYTE_EXECUTED";
	
	protected static final String NTAG_BASE_VERSION = "NTAG_BASE_VERSION";
	protected static final String NTAG_PARTIAL = "NTAG_PARTIAL";
	protected static final String NTAG_EXPORTS = "NTAG_EXPORTS";
	protected static final String NTAG_SNAPSHOT = "NTAG_SNAPSHOT";
	protected static final String NTAG_OBJECT_SIZE = "NTAG_OBJECT_SIZE";
	
	native private static boolean _initSDK(String jsonConfig);

	native private static void _uninitSDK();

	protected static boolean initSDK(Properties settings) {
		String strConfig = new String();
		if (null != settings)
			strConfig = JSONObject.fromObject(settings).toString();
		
		// TODO package the dll into jar: http://blog.sheimi.me/blog/2012/02/22/jni-and-pack-jni-into-jar.html
		
		System.loadLibrary("AquaClient");
		return _initSDK(strConfig);
	}

	protected static void uninitSDK() {
		_uninitSDK();
	}

	native protected Object _create(String rootURL, String userDomain, String homeContainer,
			String jsonProps);

	native protected void _destroy();

	private Object _nativeClient;
	private String _nativeId;

	native protected int _exec_Cdmi(String cmd, String uri, String jsonArgs,
			StringBuffer jsonResult);

	native protected int _exec_nonCdmi(String cmd, String uri, String jsonArgs,
			StringBuffer jsonResult, ByteBuffer buf);

	protected void finalize() throws Throwable {
		_destroy();
	}

}
