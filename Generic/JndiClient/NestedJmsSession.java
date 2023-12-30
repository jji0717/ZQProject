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
// Ident : $Id: NestedJmsSession.java, hui.shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JndiClient/NestedJmsSession.java $
// ===========================================================================

import java.util.*; 
import javax.jms.*;
import javax.naming.*;

public class NestedJmsSession implements MessageListener, ExceptionListener
{
	String _destType;

	NestedJndiClient       _client =null;
	javax.jms.Connection   _conn =null;
	javax.jms.Session      _sess =null;
	javax.jms.Destination  _dest =null;
	String      _destName=null;

	protected MessageProducer  _producer  = null;
	protected MessageConsumer  _consumer = null;
	protected boolean          _asProducer   = true;
	protected boolean          _asConsumer = false;
	
	private boolean _badConn = true;

	volatile ReconnectThread _thread;
	String _instanceId;
	
	// producer options
	int     _deliveryMode = DeliveryMode.PERSISTENT; // the producer's default delivery mode.  
	boolean _disableMessageID =false;          // whether message IDs are disabled.  
	boolean _disableMessageTimestamp =false;   // whether message timestamps are disabled.  
	int     _defaultPriority =4;               // the producer's default priority.  
	long    _messageTTL =0;                    // the default length of time in milliseconds if a produced message should be retained by the message system.
	
	public NestedJmsSession(NestedJndiClient client, String instanceId, String type, String destinationName, boolean asProducer, boolean asConsumer)
	{
		_badConn = true;
		_client =  client;
		_destName = destinationName;
		_instanceId = instanceId;
		
		_destType  = type.toLowerCase();
		if (_destType != "topic" && _destType != "queue")
			_destType  = "queue";
		
		_asProducer   = asProducer;
		_asConsumer = asConsumer;
		
		active();
	}
	
	protected void finalize()
	{
		try {
			deactive();
		}
     	catch(Exception ex)
     	{
     		_client.log(4, "NestedJmsSession::finalize("+ _destName +"@"+ _client.getProviderUrl() +") close connection error: " + ex.toString());
     	}
    }
    
    protected native void _notifyConnected(String instanceId, String notice);
    protected native void _forwardConnectionException(String instanceId, String notice);
    
    protected native void _dispatchTextMessage(String instanceId, String msg, String[] msgProps);
	protected native void _dispatchMapMessage(String _instanceId, String[] params, String[] msgProps);
	
	public synchronized void active()
	{
		if (null == _thread)
		_thread = new ReconnectThread();
		
		if (!_thread.isAlive())
			_thread.start();
	}

	public synchronized void deactive()
	{
		ReconnectThread theTh = _thread;
		_thread = null;
		try {
			if (_conn != null)
				_conn.close();
			_badConn = true;
		}
     	catch(Exception ex)
     	{
     		_client.log(4, "NestedJmsSession::deactive("+ _destName +"@"+ _client.getProviderUrl() +") close connection caught error: " + ex.toString());
     	}
			
		try {
			if (theTh !=null)
				theTh.notifyAll();
		}
     	catch(Throwable t)
     	{
     	}
	}
	
	protected synchronized void connect() throws Exception
	{
			if (_conn != null)
				_conn.close();
			_badConn = true;

			_client.log(7, "NestedJmsSession::connect("+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] creating QueueConnection");
			Context jndiCtx = _client.getContext();

			if ("queue" == _destType)
			{
				_client.log(7, "NestedJmsSession::connect() looking up for \"ConnectionFactory\"");
      			QueueConnectionFactory queueFactory = (QueueConnectionFactory) jndiCtx.lookup("ConnectionFactory");
      			_conn = queueFactory.createQueueConnection();
				_client.log(7, "NestedJmsSession::connect(Queue:"+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] sink the connection exceptions");
      			
				_client.log(7, "NestedJmsSession::connect(Queue:"+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] creating QueueSession");
				_sess = ((QueueConnection)_conn).createQueueSession(false, Session.AUTO_ACKNOWLEDGE);
      			_dest = (javax.jms.Queue) _client.getContext().lookup(_destName);
      			
				_client.log(7, "NestedJmsSession::connect(Queue:"+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] creating Sender");
			
				if (_asProducer)
					_producer = ((QueueSession)_sess).createSender((javax.jms.Queue)_dest);

				if (_asConsumer)
				{
					_consumer = ((QueueSession)_sess).createReceiver((javax.jms.Queue)_dest);
					_consumer.setMessageListener(this);
				}
			}
			else if ("topic" == _destType)
			{
				_client.log(7, "NestedJmsSession::connect() looking up for \"ConnectionFactory\"");
      			TopicConnectionFactory topicFactory = (TopicConnectionFactory) jndiCtx.lookup("ConnectionFactory");

      			_conn = topicFactory.createTopicConnection();
				_client.log(7, "NestedJmsSession::connect(Topic:"+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] sink the connection exceptions");
      			
				_client.log(7, "NestedTopicClient::connect(Topic:"+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] creating TopicSession");
				_sess = ((TopicConnection)_conn).createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
      			_dest = (javax.jms.Topic) _client.getContext().lookup(_destName);
      			
				_client.log(7, "NestedTopicClient::connect(Topic:"+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] creating Sender");
			
				if (_asProducer)
					_producer = ((TopicSession)_sess).createPublisher((javax.jms.Topic)_dest);

				if (_asConsumer)
				{
					_consumer = ((TopicSession)_sess).createSubscriber((javax.jms.Topic)_dest);
					_consumer.setMessageListener(this);
				}
			}
			
			if (_asProducer && _producer!=null)
				setProducerOptions(true, _deliveryMode, _disableMessageID, _disableMessageTimestamp, _defaultPriority, _messageTTL);
						
   			_conn.setExceptionListener(this);
			_conn.start();

			try {
				_client.log(7, "NestedJmsSession::connect("+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] connected, notifying owner");
				_notifyConnected(_instanceId, "Connected to [" + _destName +"@"+ _client.getProviderUrl() + "]");
			}
			catch (Throwable t) {}
			
			_client.log(7, "NestedJmsSession::connect("+ _destName +"@"+ _client.getProviderUrl() + ") th[" + Thread.currentThread().getName() +"] completed");

			_badConn = false;
	}
	
	public boolean setProducerOptions(boolean duringConnecting, int deliveryMode, boolean disableMessageID, boolean disableMessageTimestamp, int defaultPriority, long messageTTL)
	{
		if (!_asProducer)
			return false;
			
		if (!duringConnecting)
		{
			_deliveryMode = deliveryMode;
			_disableMessageID = disableMessageID;
			_disableMessageTimestamp = disableMessageTimestamp;
			_defaultPriority = defaultPriority;
			_messageTTL = messageTTL;
			
			if (_deliveryMode != DeliveryMode.PERSISTENT && _deliveryMode != DeliveryMode.NON_PERSISTENT)
				_deliveryMode = DeliveryMode.PERSISTENT;
			
			if (_defaultPriority <0)
				_defaultPriority =0;
			else if (_defaultPriority >9)
				_defaultPriority =9;
			
			if (_messageTTL <0)
				_messageTTL =0;
			
			_client.log(7, "NestedJmsSession::setProducerOptions() options updated: deliveryMode["+_deliveryMode+"], defaultPriority["+_defaultPriority+"], messageTTL["+_messageTTL+"], disableMessageID["+(_disableMessageID?"T":"F")+"], disableMessageTimestamp["+(_disableMessageID?"T":"F")+"]");
		}
		
		if (_producer !=null)
		{
			try {
				// apply the producer options
				_producer.setDeliveryMode(_deliveryMode);
				_producer.setDisableMessageID(_disableMessageID);
				_producer.setDisableMessageTimestamp(_disableMessageTimestamp);
				_producer.setPriority(_defaultPriority);
				_producer.setTimeToLive(_messageTTL);
			}
			catch (Throwable t) {}
		}
		
		return true;
	}
	
	private String[] readMessageProperties(Message msg)
	{
		if (msg ==null)
			return null;
		
		Vector sv =new Vector<String>();
		Enumeration keys;
		try {
			keys = msg.getPropertyNames();
		}
		catch(Throwable t)
		{
			_client.log(4, "NestedJmsSession::readMessageProperties() failed to enumerate messsage properties: " + NestedJndiClient.exceptionToString(t));
			return null;
		}
		
		while (keys.hasMoreElements())
		{
			String key = (String) keys.nextElement();
			try {
				Object value = msg.getObjectProperty(key);
				if (key ==null || key.length() <=0 || value ==null)
							continue;
						
				if (value instanceof String)
					sv.add((String) key.trim() + "=" + ((String)value).trim());
				else if (value instanceof Integer)
					sv.add((String) key.trim() + "=$I$" + ((Integer)value).toString());
				else if (value instanceof Integer)
					sv.add((String) key.trim() + "=$L$" + ((Long)value).toString());
				else if (value instanceof Float)
					sv.add((String) key.trim() + "=$F$" + ((Float)value).toString());
				else if (value instanceof Double)
					sv.add((String) key.trim() + "=$D$" + ((Double)value).toString());
				else if (value instanceof Boolean)
					sv.add((String) key.trim() + "=$B$" + (String)(((Boolean)value).booleanValue() ? "1" : "0") );
			}
			catch (Throwable t)
			{
				_client.log(4, "NestedJmsSession::readMessageProperties() failed to read messsage property[" + key +"] " + NestedJndiClient.exceptionToString(t));
			}
		}
		
		try {
			sv.add("SYS.timestamp=$L$" + msg.getJMSTimestamp());
		}
		catch (Throwable t) {}
					
		return (String[]) sv.toArray(new String[sv.size()]); 
	}
	
	private void applyMessageProperties(Message msg, String[] props)
	{
		if (msg ==null || props==null)
			return;
		
		for (int i=0; i < props.length; i++)
		{
			try {
				String key=null, value="";
				int pos = props[i].indexOf('=');
				if (pos > 0)
				{
					key = props[i].substring(0, pos).trim();
					value = props[i].substring(pos+1).trim();
				}
			
				if (key == null || key.length() <=0 || value == null || value.length() <=0)
					continue;
				
				char ch = 'S';
				if (value.length() >3 && value.charAt(0) == '$' && value.charAt(2) == '$')
				{
					ch = value.charAt(1);
					value = value.substring(3);
				}
				
				if (key == "SYS.expiration" && ch =='L')
				{
					msg.setJMSExpiration(Long.parseLong(value));
					continue;
				}
				
				if (key == "SYS.priority" && ch =='I')
				{
					msg.setJMSPriority(Integer.parseInt(value));
					continue;
				}

				if (key == "SYS.deliveryMode" && ch =='I')
				{
					int dm = Integer.parseInt(value);
					if (dm != DeliveryMode.PERSISTENT && dm != DeliveryMode.NON_PERSISTENT)
						continue;

					msg.setJMSDeliveryMode(dm);
					continue;
				}

				switch (ch)
				{
					case 'B' : msg.setBooleanProperty(key, Integer.parseInt(value) !=0); break;
					case 'I' : msg.setIntProperty(key, Integer.parseInt(value)); break;
					case 'L' : msg.setLongProperty(key, Long.parseLong(value)); break;
					case 'F' : msg.setFloatProperty(key, Float.parseFloat(value)); break;
					case 'D' : msg.setDoubleProperty(key, Double.parseDouble(value)); break;
					default  : msg.setStringProperty(key, value); break;
					case 'S' : msg.setStringProperty(key, value); break;
				}
			}
			catch (Throwable t) {}
		}
	}

	public synchronized boolean sendText(String[] props, String message)
	{
		if (!_asProducer)
		{
			_client.log(3, "NestedJmsSession::sendText(" + message	+ ") illegal call as the client is not created as a Sender");
			return false;
		}
		
		try {
				
			TextMessage msg = _sess.createTextMessage();
			
			_client.log(7, "NestedJmsSession::sendText(" + message	+ ") applying message properties");
			applyMessageProperties(msg, props);
			
			_client.log(7, "NestedJmsSession::sendText(" + message	+ ") sending message");
			msg.setText(message);
			_producer.send(msg);
			_client.log(7, "NestedJmsSession::sendText(" + message	+ ") message sent");
			
			return true;
		}
		catch (Exception ex)
		{
			_client.log(3, "NestedJmsSession::sendText(" + message	+ ") failed: " + NestedJndiClient.exceptionToString(ex));
				
			if (!_badConn)
			{
				_badConn = true;
				_client.log(7, "NestedJmsSession::sendText(" + message	+ ") marked to reconnect next time");
			}
			return false;
		}
	}
	
	public synchronized boolean sendMap(String[] props, String[] message)
	{
		if (!_asProducer)
		{
			_client.log(3, "NestedJmsSession::sendMap() illegal call as the client is not created as a Sender");
			return false;
		}
		
		try {
			MapMessage msg = _sess.createMapMessage();

			_client.log(7, "NestedJmsSession::sendMap() applying message properties");
			applyMessageProperties(msg, props);
			
			for(int i=0; i < message.length; i++)
			{
				String key=null, value="";
				int pos = message[i].indexOf('=');
				if (pos > 0)
				{
					key = message[i].substring(0, pos).trim();
					value = message[i].substring(pos+1).trim();
				}
				
				if (key == null || key.length() <=0)
					continue;
				
				char ch = 'S';
				if (value != null && value.length() >3 && value.charAt(0) == '$' && value.charAt(2) == '$')
				{
					ch = value.charAt(1);
					value = value.substring(3);
				}
				
				if (value == null || value.length() <=0)
				{
					msg.setString(key, "");
					continue;
				}
				
				// msg.setString(key, value);
				
				switch (ch)
				{
					case 'B' : msg.setBoolean(key, Integer.parseInt(value) !=0); break;
					case 'I' : msg.setInt(key, Integer.parseInt(value)); break;
					case 'L' : msg.setLong(key, Long.parseLong(value)); break;
					case 'F' : msg.setFloat(key, Float.parseFloat(value)); break;
					default  : msg.setString(key, value); break;
					case 'S' : msg.setString(key, value); break;
				}
			}

			_client.log(7, "NestedJmsSession::sendMap() sending message");
			_producer.send(msg);
			_client.log(7, "NestedJmsSession::sendMap() message sent");
			return true;
		}
		catch (Exception ex)
		{
			_client.log(3, "NestedJmsSession::sendMap() failed: " + NestedJndiClient.exceptionToString(ex));
				
			if (!_badConn)
			{
				_badConn = true;
				_client.log(7, "NestedJmsSession::sendMap() marked to reconnect next time");
			}
			return false;
		}
	}

	public void onMessage(Message msg)
    {
        try {
       		_client.log(7, "NestedJmsSession::onMessage() reading message properties");
       		String[] msgProps = readMessageProperties(msg);
        
        	if (msg instanceof TextMessage)
        	{
				TextMessage tm = (TextMessage) msg;
        		if (NestedJndiClient._javaTest)
	        		_client.log(7, "NestedJmsSession::onMessage() text:" + tm.getText());
				else
					_dispatchTextMessage(_instanceId, tm.getText(), msgProps);
					
				return;
        	}
        	
        	if (msg instanceof MapMessage)
        	{
				Vector sv =new Vector<String>();
				MapMessage mm = (MapMessage) msg;

				Enumeration keys = mm.getMapNames();
				while (keys.hasMoreElements())
				{
					try {
						String key = (String) keys.nextElement();
						Object value = mm.getObject(key);
						if (key ==null || key.length() <=0 || value ==null)
							continue;
						
						if (value instanceof String)
							sv.add((String) key.trim() + "=" + ((String)value).trim());
						else if (value instanceof Integer)
							sv.add((String) key.trim() + "=$I$" + ((Integer)value).toString());
						else if (value instanceof Float)
							sv.add((String) key.trim() + "=$F$" + ((Float)value).toString());
						else if (value instanceof Boolean)
							sv.add((String) key.trim() + "=$B$" + (String)(((Boolean)value).booleanValue() ? "1" : "0") );
					}
					catch(Throwable t)
					{
						_client.log(3, "NestedJmsSession::onMessage() read map caught exception" + NestedJndiClient.exceptionToString(t));
					}
				}
					
				String[] params = (String[]) sv.toArray(new String[sv.size()]); 

        		if (NestedJndiClient._javaTest)
        		{
        			String msgstr = new String();
        			for (int i = 0; i < params.length; i++)
        				msgstr += params[i] +"; ";
	        		_client.log(7, "NestedJmsSession::onMessage() map: " + msgstr);
	        	}
				else
				{
					_dispatchMapMessage(_instanceId, params, msgProps);
				}
					
				return;
        	}
        }
        catch(Throwable t)
        {
        	_client.log(3, "NestedJmsSession::onMessage() caught exception" + NestedJndiClient.exceptionToString(t));
        }
    }
    
    public void onException(JMSException ex)
    {
		try {
			_client.log(4, "NestedJmsSession::onException(" + _destName + "@" + _client.getProviderUrl() + ") caught connection exception: " + NestedJndiClient.exceptionToString(ex));
			_forwardConnectionException(_instanceId, "Connection lost to [" + _destName +"@"+ _client.getProviderUrl() + "]");
		}
		catch (Throwable t) {}
		
        _badConn = true;
    }
	
	class ReconnectThread extends Thread
	{
			int waitweight =0;
			
      		public ReconnectThread() {}
      
      		public void run()
      		{
      			Thread thisThread = currentThread();

				while (_thread == thisThread)
				{
					if (_badConn)
					{
						try {
							_client.log(7, "NestedJmsSession::ReconnectThread(" + _destName + "@" + _client.getProviderUrl() + ") bad connection, try reconnecting");
							connect();
							waitweight = 0;
//							_client.log(6, "NestedJmsSession::ReconnectThread(" + _destName + "@" + _client.getProviderUrl() + ") connection established");
						}
						catch (Exception ex)
						{
							if (0 == waitweight)
								waitweight =1;
							else waitweight *=2;
							
							if (waitweight <10) // no need to repeat printing the excpetion if keep failed to reconnect
								_client.log(3, "NestedJmsSession::ReconnectThread(" + _destName + "@" + _client.getProviderUrl()	+ ") failed: " + NestedJndiClient.exceptionToString(ex));
						}
					}
					
					try{
						int secToSleep = 10; // 10 sec
						if (_badConn)
						{
							if (waitweight > 30)
								waitweight = 30;
							else if (waitweight<=0)
								waitweight =1;
								
							secToSleep = waitweight; // up to 60 sec
						}
						
						for (int i=0; _thread == thisThread && i < secToSleep*2; i++)
							sleep(500);
					}
					catch(Exception ex) {}
				}
      		}
	}
}