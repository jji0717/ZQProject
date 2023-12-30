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
// $Log: /ZQProjs/Generic/JndiClient/JndiClient.java $
// ===========================================================================

// build commands:
// set CLASSPATH=.;%JAVA_HOME%\..\3rd-party-classlib\jbossall-client.jar
// %JAVA_HOME%\bin\javac NestedJndiClient.java

import java.util.*; 
import java.io.*; 
import javax.jms.*;
import javax.naming.*;

public class NestedJndiClient
{
	Properties _props = new Properties();
	private Context _jndiCtx = null;
	private String _instanceId =null;
	
	static public boolean _javaTest = false;
	int _javaTraceLevel = 5; // ZQ::common::Log::L_NOTICE
	
    String providerUrl;
    final static String DEFAULT_INITIAL_CONTEXT_FACTORY ="org.jnp.interfaces.NamingContextFactory";
    final static String DEFAULT_STATE_FACTORIES         ="org.jboss.naming:org.jnp.interfaces";
    final static String DEFAULT_ProviderUrl             ="localhost:1099";
	
	protected static native void _nativelog(int level, String msg);
	
	public void log(int level, String msg)
	{
		if (level > _javaTraceLevel)
			return;
			
		if (_javaTest)
			System.out.println(msg); 
		else _nativelog(level, msg);
	}
	
	public Context getContext()
	{
		return _jndiCtx;
	}
	
	public String getProviderUrl()
	{
		return providerUrl;
	}
	
	public static String exceptionToString(Throwable t)
	{
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		t.printStackTrace(new PrintStream(baos));
		return baos.toString();
	}

	static public NestedJndiClient initContext(int traceLevel, String providerUrl, String instanceId, String params[])
	{
			if (!_javaTest)
			{
       			try
     			{
					Thread.currentThread().setContextClassLoader(ClassLoader.getSystemClassLoader());
				}
     			catch(Exception ex)
     			{
					_nativelog(4, "NestedJndiClient::initContext() hook system class loader failed: " + exceptionToString(ex));
     			}
     		}

       		try
     		{
				NestedJndiClient client  = new NestedJndiClient();
				client._instanceId 		= new String(instanceId);
				client._javaTraceLevel = traceLevel & 0x07;
				
				for(int i=0; i < params.length; i++)
				{
					String key=null, value="";
					int pos = params[i].indexOf('=');
					if (pos > 0)
					{
						key = params[i].substring(0, pos).trim();
						value = params[i].substring(pos+1).trim();
					}
				
					if (key == null || key.length() <=0 || value == null)
						continue;
						
     				client.log(7, "NestedJndiClient::initContext() param["+ key +"]=" + value);

					client._props.put(key, value);
				}

				if (!client._props.containsKey(Context.INITIAL_CONTEXT_FACTORY))
				{
     				client.log(7, "NestedJndiClient::initContext() prop["+ Context.INITIAL_CONTEXT_FACTORY +"] not specified, taking [" + DEFAULT_INITIAL_CONTEXT_FACTORY + "]");
     				client._props.put(Context.INITIAL_CONTEXT_FACTORY, DEFAULT_INITIAL_CONTEXT_FACTORY);
     			}
     				
				if (!client._props.containsKey(Context.STATE_FACTORIES))
				{
     				client.log(7, "NestedJndiClient::initContext() prop["+ Context.STATE_FACTORIES +"] not specified, taking [" + DEFAULT_STATE_FACTORIES + "]");
     				client._props.put(Context.STATE_FACTORIES, DEFAULT_STATE_FACTORIES);
     			}
					
				if (null == providerUrl || providerUrl.length()<=0)
     				client.providerUrl = DEFAULT_ProviderUrl;
     			else client.providerUrl = new String(providerUrl);
     			client._props.put(Context.PROVIDER_URL, client.providerUrl);
     			
     			client.log(7, "NestedJndiClient::initContext() initializing to provider[" + client.providerUrl +"]");
     			client._jndiCtx = new InitialContext(client._props);  
     			
     			client.log(6, "NestedJndiClient::initContext() context initialized to provider[" + client.providerUrl +"]");
     			return client;
     		}
     		catch(Exception ex)
     		{
				if (_javaTest)
					System.out.println("NestedJndiClient::initContext() failed: " + exceptionToString(ex)); 
				else _nativelog(3, "NestedJndiClient::initContext() failed: " + exceptionToString(ex));
     		}
     		
   			return null;
	}
	
	public NestedJmsSession createJmsQueueSession(String instanceId, String queueName, boolean asProducer, boolean asConsumer)
	{
			if (!_javaTest)
			{
       			try
     			{
					Thread.currentThread().setContextClassLoader(ClassLoader.getSystemClassLoader());
				}
     			catch(Exception ex)
     			{
					_nativelog(4, "NestedJndiClient::initContext() hook system class loader failed: " + exceptionToString(ex));
     			}
     		}

    		try
     		{
     			return new NestedJmsSession(this, instanceId, "queue", queueName, asProducer, asConsumer);
     		}
     		catch(Exception ex)
     		{
     			log(3, "NestedJndiClient::createJmsQueueSession() failed: " + exceptionToString(ex));
     			return null;
     		}
	}
	
	public NestedJmsSession createJmsTopicSession(String instanceId, String topicName, boolean asPublisher, boolean asSubscriber)
	{
			if (!_javaTest)
			{
       			try
     			{
					Thread.currentThread().setContextClassLoader(ClassLoader.getSystemClassLoader());
				}
     			catch(Exception ex)
     			{
					_nativelog(4, "NestedJndiClient::initContext() hook system class loader failed: " + exceptionToString(ex));
     			}
     		}

    		try
     		{
     			return new NestedJmsSession(this, instanceId, "topic", topicName, asPublisher, asSubscriber);
     		}
     		catch(Exception ex)
     		{
     			log(3, "NestedJndiClient::createJmsTopicSession() failed: " + exceptionToString(ex));
     			return null;
     		}
	}
	
	public static void main(String[] args)
	{
		System.loadLibrary("JndiClient");
		Vector sv =new Vector<String>();
		sv.add(Context.INITIAL_CONTEXT_FACTORY + "=" + DEFAULT_INITIAL_CONTEXT_FACTORY);
		sv.add(Context.STATE_FACTORIES + "=" + DEFAULT_STATE_FACTORIES);
		
		NestedJndiClient._javaTest = true;
		NestedJndiClient jc = NestedJndiClient.initContext(7, "10.15.10.32:13001", "test", (String[]) sv.toArray(new String[sv.size()]));
//		NestedJndiClient jc = NestedJndiClient.initContext(7, "192.168.80.67:2099", "", "", "test");
		if (jc ==null)
			return;
		
/*
		NestedQueueSender qs = new NestedQueueSender(jc, "queue/testQueue");
//		qs.connect();

		NestedQueueReceiver qr = new NestedQueueReceiver(jc, "queue/testQueue");
//		qr.connect();
 */
//		NestedQueueClient qc = new NestedQueueClient(jc, "queue/testQueue", true, true);
		NestedJmsSession sess = new NestedJmsSession(jc, "testIsnt", "topic", "topic/testTopic", true, true);
		
		for (int i=1; true; i++)
		{
			try {
				String[] params = new String[]{"a=1","b=2","c=3"};
//				sess.sendText("hello", null);
				sess.sendMap(params, null);
				for (int j=1; j <10; j++)
				jc.wait(1000);
			}
			catch(Exception ex)
			{
				// qc.wait(1000);
			}
		}
    }

}

