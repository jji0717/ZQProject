package com.xormedia.aqua.sdk;

import java.util.*;
import java.io.*;
import java.nio.*;
import java.nio.channels.FileChannel;

// http://json-lib.sourceforge.net/
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;
import net.sf.json.util.JSONUtils;

// $(JAVA_HOME)\bin\javac $(InputName).java; $(JAVA_HOME)\bin\javah $(InputName);

/**
 * A client designed to connect to the front-ends of Aqua, which is a cloud storage from XOR media.<p>
 * The client supplies the methods to interact with Aqua front-ends via SNIA's CDMI protocol, see http://cdmi.sniacloud.com/ for more details.<p>
 * Moreover, AquaClient covers authentication steps to Aqua. It will sign the out-going URIs that are about to operate according to Aqua's authentication policies.<p> 
 * With some built-in fault tolerances, AquaClient retrieves the list of available front-end server. If the current connected server became unreachable, it would retry the rest available.
 * This client has built-in caches at several points:
 * <table bolder='0'>
 * <tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>
 *              <th width=150><u>read-ahead cache</u></th><td>the client is able to read-ahead to preload the next data chunk before they are really used by the user in order to speed up the operation</td></tr>
 * <tr><td></td><th><u>write-back cache</u></th><td>the client is able to delay the modification and merge data pieces to larger data chunk then submit, so that it speeds up the user's write operations and reduce the amount of submissions thru the network</td></tr>
 * <tr><td></td><th><u>location cache</u></th><td>Aqua may redirect to another front-end and/or storage instance per an individual data object or container, this client is designed to cache such per-uri location in order to reduce redirections</td></tr>
 * <tr><td></td><th><u>attribute cache</u></th><td>the client can cache the attributes of data object or container in order to reduce the amount of queries to the server and speeds up the operations</td></tr>
 * </table><p>
 * 
 * @author hui.shao
 */
public class AquaClient extends NestedClient {

	private Properties _props = new Properties();
	
	/**
	 * Initiliaze this SDK with properties
	 * @param settings the properties to initialize the SDK<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;the following are the defined properties:
	 * <table bolder='1'><tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><th width=100>Property</th><th>ValueType</th><th>Description</th></tr>
	 * <tr><td></td><td>log.dir</td><td>String</td><td>The path name of the folder under where the log files of this SDK will be put</td></tr>
	 * <tr><td></td><td>log.level</td><td>int</td><td>The integer level to print log: 0-none, 3-ERROR, 4-WARNING, 6-INFO, 7-DEBUG</td></tr>
	 * <tr><td></td><td>log.size</td><td>int</td><td>The byte size limitation of each log file, minimal allowed 100KB </td></tr>
	 * <tr><td></td><td>log.count</td><td>int</td><td>The maximal count of log files to keep</td></tr>
	 * <tr><td></td><td>threads</td><td>int</td><td>The size of the processing thread pool</td></tr>
	 * </table>
	 * @return true if successful
	 */
	public static boolean initSDK(Properties settings) {
		return NestedClient.initSDK(settings);
	}

	public static void uninitSDK() {
		NestedClient.uninitSDK();
	}
	
	/**
	 * Create a client connects to the Aqua front-ends.
	 * @param rootURL       to specify the root URL to connect to the Aqua front-end, in the format of <u>http://&lt;username&gt;:&lt;password&gt;@&lt;server&gt;:&lt;port&gt;/aqua/rest/cdmi.
	 * @param homeContainer to specify the home container under the root URL, all the operations of the client will be limited under such a container
	 * @param props         to specify additional properties of the client<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The following are the defined properties:
	 * <table bolder='1'><tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><th width=100>Property</th><th>ValueType</th><th>Description</th></tr>
	 * <tr><td></td><td>cache.enable</td><td>int</td><td>Non-zero to enable the readahead and writeback data cache</td></tr>
	 * <tr><td></td><td>cache.block.size</td><td>int</td><td>The byte size of cache blocks</td></tr>
	 * <tr><td></td><td>cache.block.count.read</td><td>int</td><td>The total amount of blocks that reserved for all data read purposes</td></tr>
	 * <tr><td></td><td>cache.block.count.write</td><td>int</td><td>The total amount of blocks that reserved for all write read purposes</td></tr>
	 * <tr><td></td><td>cache.block.count.readahead</td><td>int</td><td>The amount of blocks that will be read ahead before use for each data object</td></tr>
	 * <tr><td></td><td>cache.block.ttl.read</td><td>int</td><td>The time-to-live(TTL) in millisecond of the cached blocks for read. The blocks will be dropped when expired</td></tr>
	 * <tr><td></td><td>cache.block.ttl.write</td><td>int</td><td>The time-to-live(TTL) in millisecond of the cached blocks for write. The blocks will be flushed when expired</td></tr>
	 * <tr><td></td><td>cache.log.flags</td><td>int</td><td>The flags how to print the activities of the cache module, reserved</td></tr>
	 * <tr><td></td><td>cache.threads.flush</td><td>int</td><td>The number of threads reserved on submitting write data to Aqua front-ends</td></tr>
	 * <tr><td></td><td>client.flags</td><td>int</td><td>The flags of client, reserved</td></tr>
	 * </table>
	 * @return the client instance if successful, otherwise null
	 * @see initSDK() initSDK() must be called before instanize any AquaClients.<p>
	 * @see uinitSDK() when all the AquaClient instances are no more used, uninitSDK() should be called before program quits.
	 */
	public static AquaClient newClient(String rootURL, String userDomain, String homeContainer,
			Properties props) {

		try {
			AquaClient client = new AquaClient();

			if (null != props)
				client._props = props;
			String jsonProps = JSONObject.fromObject(props).toString();

			client._create(rootURL, userDomain, homeContainer, jsonProps);
			// System.out.println("javaId=" + client._nativeId);
			return client;
		} catch (Exception ex) {
			// if (_javaTest)
			// System.out.println("NestedJndiClient::initContext() failed: " +
			// exceptionToString(ex));
			// else _nativelog(3, "NestedJndiClient::initContext() failed: " +
			// exceptionToString(ex));
		}

		return null;
	}

	/**
	 * Create a Data Object Using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param mimetype     to specify the MIME type of the data contained within the value field of the data object
	 * @param jsonMetadata to specify the initial metadata, formatted in JSON text, Metadata for the data object
	 * @param value        to specify the initial data object value
	 * @param valuetransferencoding to specify the value transfer encoding used for the data object value
	 * @param domainURI    to specify the URI of the owner's
	 * @param deserialize  to specify the URI of a serialized CDMI data object that shallbe deserialized to create the new data object
	 * @param serialize    to specify the URI of a serialize URI of a CDMI object that shall be serialized into the new data object
	 * @param copy         to specify the URI of a CDMI data object or queue that shall be copied into the new data object
	 * @param move         to specify the URI of an existing local or remote CDMI data object
	 * @param reference    to specify the URI of a CDMI data object that shall be redirected to by a reference.
	 * @param deserializeValue to specify a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	public int cdmi_CreateDataObject(StringBuffer jsonResult, String uri,
			String mimetype, String jsonMetadata, String value,
			String valuetransferencoding[], String domainURI,
			String deserialize, String serialize, String copy, String move,
			String reference, String deserializeValue) {
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();
			map.put(NTAG_MIME_TYPE, mimetype);
			map.put(NTAG_VALUE, value);
			map.put(NTAG_METADATA, jsonMetadata);
			map.put(NTAG_TRANSFER_ENCODING, valuetransferencoding);

			map.put(NTAG_DOMAIN_URI, domainURI);
			map.put(NTAG_DESERIALIZE, deserialize);
			map.put(NTAG_SERIALIZE, serialize);

			map.put(NTAG_COPY, copy);
			map.put(NTAG_MOVE, move);
			map.put(NTAG_REFERENCE, reference);

			map.put(NTAG_DESER_VALUE, deserializeValue);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("CreateDataObject", uri, jsonParams, privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		if (jo.has(NTAG_RESPBODY))
			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		else
			jsonResult.append(privateResult);

		return ret;
	}

	/**
	 * Create a Data Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param contentType  to specify the content type of the data to be stored as a data object.
	 * @param value        to specify the initial data object value, range [0, value.limit()) will be taken as the available data to post to the server
	 * @return HTTP status responded from the server
	 */
	public int nonCdmi_CreateDataObject(String uri, String contentType,
			ByteBuffer value) {
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();

			if (contentType.length() <= 0)
				map.put(NTAG_CONTENT_TYPE, contentType);

			jsonParams = JSONObject.fromObject(map).toString();
			// byte[] buf = value.getBytes();
		} catch (Exception ex) {
		}

		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		value.flip();

		int ret = _exec_nonCdmi("CreateDataObject", uri, jsonParams.toString(),
				privateResult, value);

		// step 3. parse the jsonResult and dispatch the output parameters
		return ret;
	}

	/**
	 * Read a Data Object using CDMI Content Type
	 * @param jsonResult  the output text formatted JSON value returned from the server
	 * @param uri         to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location    the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @return HTTP status responded from the server
	 */
	public int cdmi_ReadDataObject(StringBuffer jsonResult, String uri,
			StringBuffer location) {
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("ReadDataObject", uri, "", privateResult);
		// System.out.println("presult=" + privateResult.toString());

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		if (jo.has(NTAG_RESPBODY))
			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		else
			jsonResult.append(privateResult);

		if (null != location && jo.has(NTAG_LOCATION))
			location.append(jo.getJSONObject(NTAG_LOCATION).toString());

		return ret;
	}

	/**
	 * Read a Data Object using Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param contentType  the output content type of the data that the data object contains.
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param startOffset  to specify the offset to start reading
	 * @param byteRead     the output number of bytes that have been read
	 * @param buffer       the buffer that the read data would be filled to. The space of this buffer is required to be direct-allocated and the capacity of the buffer will be taken as the maximal bytes that can be filled
	 * @param direct	   true if want to skip the built-in cache layer
	 * @return HTTP status responded from the server
	 */
	public int nonCdmi_ReadDataObject(String uri, StringBuffer contentType,
			StringBuffer location, long startOffset, LongVariable byteRead,
			ByteBuffer buffer, boolean direct) {
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();

//			if (!contentType.length() <= 0)
//				map.put(NTAG_CONTENT_TYPE, contentType);

			map.put(NTAG_START_OFFSET, startOffset);
			map.put(NTAG_DIRECTIO, direct);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_nonCdmi()
		byteRead.set(0L);
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_nonCdmi("ReadDataObject", uri, jsonParams.toString(),
				privateResult, buffer);

		// step 3. parse the jsonResult and dispatch the output parameters
		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		if (null != location && jo.has(NTAG_LOCATION))
			location.append(jo.getJSONObject(NTAG_LOCATION).toString());
			
		if (null != contentType && jo.has(NTAG_CONTENT_TYPE))
			contentType.append(jo.getJSONObject(NTAG_CONTENT_TYPE).toString());

		buffer.flip();

		if (jo.has(NTAG_NBYTE_EXECUTED)) {
			int len = (int) jo.getLong(NTAG_NBYTE_EXECUTED);
			buffer.limit(len);
			byteRead.set(len);
			// System.out.println("native read " + byteRead.get());
		}

		return ret;
	}
	
	/**
	 * Update a Data Object using a CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param jsonMetadata to specify the metadata of the data object, formatted in JSON text
	 * @param startOffset  to specify the offset to start writting
	 * @param value        to specify the value of data object, range [0, value.limit()) will be taken as the available data to post to the server
	 * @param base_version to specify the version of the data object that will be based on to modify, server may reject the update if there is version conflicts
	 * @param partial      filling true to indicates that the object is in the process of being updated, and has not yet been fully updated. Maps to HTTP header X-CDMI-Partial
	 * @param valuetransferencoding to specify the value transfer encoding used for the data object value.
	 * @param domainURI    to specify the URI of the owning domain
	 * @param deserialize  to specify the URI of a serialized CDMI data object that shall be deserialized to create the new data object
	 * @param copy         to specify the URI of a CDMI data object or queue that shall be copied into the new data object
	 * @param deserializeValue to specify a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	public int cdmi_UpdateDataObject(String uri, StringBuffer location, String jsonMetadata,
		long startOffset, String value, int base_version, boolean partial, String valuetransferencoding[],
		String domainURI, String deserialize, String copy,
		String deserializevalue)
	{
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();
			map.put(NTAG_METADATA, jsonMetadata);
			map.put(NTAG_TRANSFER_ENCODING, valuetransferencoding);

			map.put(NTAG_TRANSFER_ENCODING, valuetransferencoding);

			map.put(NTAG_BASE_VERSION, base_version);
			map.put(NTAG_PARTIAL, partial);
			map.put(NTAG_START_OFFSET, startOffset);
			map.put(NTAG_VALUE, value);
			map.put(NTAG_DOMAIN_URI, domainURI);
			map.put(NTAG_DESERIALIZE, deserialize);
			map.put(NTAG_COPY, copy);
			map.put(NTAG_DESER_VALUE, deserializevalue);
			map.put(NTAG_DESERIALIZE, deserialize);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("UpdateDataObject", uri, jsonParams, privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
//		if (null == jsonResult)
//			return ret;

		JSONObject jo = JSONObject.fromObject(privateResult.toString());
//		if (jo.has(NTAG_RESPBODY))
//			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
//		else
//			jsonResult.append(privateResult);

		if (null != location && jo.has(NTAG_LOCATION))
			location.append(jo.getJSONObject(NTAG_LOCATION).toString());

		return ret;
	}
	
	/**
	 * Update a Data Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param contentType  to specify the content type of the data to be stored as a data object.
	 * @param startOffset  to specify the offset to start updating from
	 * @param objectSize   to indicate the total size of the data object for the purpose of truncating, negative means is not specified
	 * @param buffer       to specify the data object value, range [0, buffer.limit()) of the buffer will be taken as the available data to post to the server
	 * @param partial      filling true to indicates that the object is in the process of being updated, and has not yet been fully updated. Maps to HTTP header X-CDMI-Partial
	 * @param direct       fill true if want to skip the built-in cache layer
	 * @return HTTP status responded from the server
	 */
	public int nonCdmi_UpdateDataObject(String uri, StringBuffer location, String contentType, long startOffset, long objectSize, ByteBuffer buffer, boolean partial, boolean direct)
	{
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();

			if (contentType.length() <= 0)
				map.put(NTAG_CONTENT_TYPE, contentType);

			map.put(NTAG_START_OFFSET, startOffset);
			map.put(NTAG_PARTIAL, partial);
			map.put(NTAG_OBJECT_SIZE, objectSize);
			map.put(NTAG_DIRECTIO, direct);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_nonCdmi()
		// byteRead.set(0L);
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_nonCdmi("UpdateDataObject", uri, jsonParams,
				privateResult, buffer);

		// step 3. parse the jsonResult and dispatch the output parameters
		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		if (null != location && jo.has(NTAG_LOCATION))
			location.append(jo.getJSONObject(NTAG_LOCATION).toString());

		buffer.flip();

		if (jo.has(NTAG_NBYTE_EXECUTED)) {
			int len = (int) jo.getLong(NTAG_NBYTE_EXECUTED);
			buffer.limit(len);
		}

		return ret;
	}
	
	/**
	 * Delete a Data Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	public int cdmi_DeleteDataObject(StringBuffer jsonResult, String uri)
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("DeleteDataObject", uri, "", privateResult);
		// System.out.println("presult=" + privateResult.toString());

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		// jo.remove(NTAG_RET);
		if (jo.has(NTAG_RESPBODY))
			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		else
			jsonResult.append(privateResult);

		return ret;
	}
	
	/**
	 * Delete a Data Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	public int nonCdmi_DeleteDataObject(String uri)
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_nonCdmi("DeleteDataObject", uri, "",
				privateResult, null);

		// step 3. parse the jsonResult and dispatch the output parameters
		return ret;
	}

	/**
	 * Create a Container Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param jsonMetadata to specify the initial metadata, formatted in JSON text, Metadata for the container
	 * @param jsonExports  to specify the structure, formatted in JSON text, for each protocol enabled for this container object
	 * @param domainURI    to specify the URI of the owning domain
	 * @param deserialize  to specify the URI of a serialized CDMI container object that shall be deserialized to create the new container object
	 * @param copy         to specify the URI of a CDMI container object or queue that shall be copied into the new container object
	 * @param move         to specify the URI of an existing local or remote CDMI container object (source URI) that shall be relocated to the URI specified in the PUT
	 * @param reference    to specify the URI of a CDMI container object that shall be redirected to by a reference.
	 * @param deserializeValue A container object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	public int cdmi_CreateContainer(StringBuffer jsonResult, String uri, String jsonMetadata,
			String jsonExports, String domainURI, String deserialize, 
		String copy, String move, String reference, 
		String deserializeValue)
	{
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();
			map.put(NTAG_METADATA, jsonMetadata);
			map.put(NTAG_EXPORTS,  jsonExports);

			map.put(NTAG_DOMAIN_URI, domainURI);
			map.put(NTAG_DESERIALIZE, deserialize);

			map.put(NTAG_COPY, copy);
			map.put(NTAG_MOVE, move);
			map.put(NTAG_REFERENCE, reference);

			map.put(NTAG_DESER_VALUE, deserializeValue);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("CreateContainer", uri, jsonParams, privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		// JSONObject jo = JSONObject.fromObject(privateResult.toString());
		// if (jo.has(NTAG_RESPBODY))
		// 	jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		// else
		//	jsonResult.append(privateResult);
		
		return ret;
	}

	/**
	 * Create a Container Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	///@param[in] uri  the container name appends to the rootUri, will be appended with a '/' if not available
	public int nonCdmi_CreateContainer(String uri)
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_nonCdmi("CreateContainer", uri, "",
				privateResult, null);

		// step 3. parse the jsonResult and dispatch the output parameters
		return ret;
	}

	/**
	 * Read a Container Object using CDMI Content Type
	 * @param jsonResult  the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	public int cdmi_ReadContainer(StringBuffer jsonResult, String uri)
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("ReadContainer", uri, "", privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		if (jo.has(NTAG_RESPBODY))
			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		else
			jsonResult.append(privateResult);

		return ret;
	}

	/**
	 * Update a Container Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param jsonMetadata to specify the metadata, formatted in JSON text,metadata for the container
	 * @param jsonExports  to specify the structure, formatted in JSON text, for each protocol enabled for this container object
	 * @param domainURI    to specify the URI of the owning domain
	 * @param deserialize  to specify the URI of a serialized CDMI container object that shall be deserialized to create the new container object
	 * @param copy         to specify the URI of a CDMI container object or queue that shall be copied into the new container object
	 * @param snapshot     to specify the name of the snapshot to be taken
	 * @param deserializeValue A container object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	public int cdmi_UpdateContainer(StringBuffer jsonResult, String uri, StringBuffer location,
			String jsonMetadata, String jsonExports,
		String domainURI, String deserialize, String copy,
		String snapshot, String deserializeValue)
	{
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();
			map.put(NTAG_METADATA, jsonMetadata);
			map.put(NTAG_EXPORTS,  jsonExports);

			map.put(NTAG_DOMAIN_URI, domainURI);
			map.put(NTAG_DESERIALIZE, deserialize);

			map.put(NTAG_COPY, copy);
			map.put(NTAG_SNAPSHOT, snapshot);

			map.put(NTAG_DESER_VALUE, deserializeValue);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("UpdateContainer", uri, jsonParams, privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		// JSONObject jo = JSONObject.fromObject(privateResult.toString());
		// if (jo.has(NTAG_RESPBODY))
		// 	jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		// else
		//	jsonResult.append(privateResult);
		
		return ret;
	}

	/**
	 * Delete a Container Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	public int cdmi_DeleteContainer(StringBuffer jsonResult, String uri)
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("DeleteContainer", uri, "", privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		// JSONObject jo = JSONObject.fromObject(privateResult.toString());
		// if (jo.has(NTAG_RESPBODY))
		// 	jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		// else
		//	jsonResult.append(privateResult);
		
		return ret;
	}

	/**
	 * Delete a Container Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	public int nonCdmi_DeleteContainer(String uri)
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_nonCdmi("DeleteContainer", uri, "",
				privateResult, null);

		// step 3. parse the jsonResult and dispatch the output parameters
		return ret;
	}

	/**
	 * Read the Aqua domain extension parameters
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param domainURI    to specify the URI of the domain to query for
	 * @return HTTP status responded from the server
	 */
	public int cdmi_ReadAquaDomain(StringBuffer jsonResult, String domainURI)
	{
		// step 1. envelop the input parameters into jsonParams
		String jsonParams = new String();

		try {
			Map map = new HashMap();
			map.put(NTAG_DOMAIN_URI, domainURI);

			jsonParams = JSONObject.fromObject(map).toString();
		} catch (Exception ex) {
		}

		// step 2. call _exec_Cdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_Cdmi("ReadAquaDomain", "", jsonParams, privateResult);

		// step 3. parse the jsonResult and dispatch the output parameters
		if (null == jsonResult)
			return ret;

		JSONObject jo = JSONObject.fromObject(privateResult.toString());
		if (jo.has(NTAG_RESPBODY))
			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
		else
			jsonResult.append(privateResult);
		
		return ret;
	}

	/**
	 * Flush the cached content of a data object
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	public int flushDataObject( String uri )
	{
		// step 1. envelop the input parameters into jsonParams
		// step 2. call _exec_nonCdmi()
		StringBuffer privateResult = new StringBuffer();
		int ret = _exec_nonCdmi("FlushDataObject", uri, "",
				privateResult, null);

		// step 3. parse the jsonResult and dispatch the output parameters
		return ret;
	}

}
