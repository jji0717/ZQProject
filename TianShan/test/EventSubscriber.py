#!/usr/bin/python
#
#
#   EventSubscriber.py
#       subscribe to IceStorm topics on server.
#
#   Author: fei huang
#   Date:   Sep 25th, 2006
#
#   Nov 23th, 2006 fei.huang changes on channel on demand interfaces
#   Dec 7th, 2006 fei.huang ContentPublisher removed.
#   Jan 12th, 2007 fei.huang move event subscribers out of the main routine.
#                       rewrite subscribe function to fix the weird unhandled exception raised when exit.
#

"""Subscribe to the Ice storm topics"""

try :
    import sys
    from threading import Thread

    import Ice, IceStorm
    import TianShanIce
    from TianShanIce.Storage import ProvisionStateChangeSink, ProvisionProgressSink, ContentProvisionStatus
    from Managers import ChannelManager

    from Utils import Logger, config, communicator
except ImportError, ex:
    print __file__, '--> Import not completed! ', str(ex)
    raise


class ProvisionStateChangeSinkI(ProvisionStateChangeSink):
    """ Call back interface for state change event """

    def __init__(self, channelName) :
        """
        channelName: channel to which the specified content will be published.
        """

        ProvisionStateChangeSink.__init__(self)

        self.__channelName = channelName
        self.__channelMgr = None
        
        if channelName:
            self.__channelMgr = ChannelManager() 

        self.log = Logger.logger(self.__class__.__name__)

    def ping(self, timestamp, context=Ice.Current()): pass

    def OnStateChange(self, netId, contentName, triggerTimeUTC, 
                            previousStatus, currentStatus, errorCode, errorMsg, context=Ice.Current()):

        self.log.info("content(%s) : previous(%s) : current(%s) : time(%s)" \
                    %(contentName, previousStatus, currentStatus, triggerTimeUTC))

        if currentStatus == ContentProvisionStatus.cpsStreamable:
            if self.__channelMgr:
                try:
                    channel = self.__channelMgr.openChannel(self.__channelName, True)
                    self.__channelMgr.insertItem(channel, contentName)
                except (TianShanIce.InvalidParameter, TianShanIce.ServerError), ex:
                    self.log.error(ex.message)

# End ProvisionStateChangeSinkI


class ProvisionProgressSinkI(ProvisionProgressSink):
    """ Call back interface for provision progress event """

    def __init__(self):
        ProvisionProgressSink.__init__(self)

        self.log = Logger.logger(self.__class__.__name__)

    def ping(self, timestamp, context=Ice.Current()): pass

    def OnProgress(self, netId, contentName, timestamp, 
            processed, total, stepNo, totalSteps, context=Ice.Current()):
            self.log.info("content(%s) : processed(%d) : stepNo(%d) : totalSteps(%d)" \
                    %(contentName, processed, stepNo, totalSteps))

# End ProvisionProgressSinkI



class EventSubscriber(Thread):
    __qos = {"reliability" : "twoway ordered"}

    def __init__(self, channel=None, isCreateAllowed=True) :
        Thread.__init__(self, name=self.__class__.__name__)

        self.log = Logger.logger(self.__class__.__name__)

        self.__channel = channel
        self.__isCreateAllowed = isCreateAllowed

        self.__topicSubscribed = list()
        self.__subscribers = dict()

        self.log.info("generating topic manager...")
        base = communicator.stringToProxy(config["TopicManagerEndpoint"])
        self.__topicManagerPrx = IceStorm.TopicManagerPrx.checkedCast(base)
        if not self.__topicManagerPrx:
            raise TianShanIce.InvalidParameter("Invalid endpoint (%s)." %(config["TopicManagerEndpoint"]))

        self.log.info("creating adapter...")
        self.__adapter = communicator.createObjectAdapterWithEndpoints(
                'EventSubscriber', config['SubscriberEndpoint'])

        self.__adapter.activate()
    
    def subscribe(self, subscriber, topic):
        base = self.__adapter.addWithUUID(subscriber)
        eventSink = Ice.ObjectPrx.checkedCast(base)

        theTopic = None
        try:
            self.log.info("retrieving topic (%s)..." %(topic))
            theTopic = self.__topicManagerPrx.retrieve(topic)
        except IceStorm.NoSuchTopic:
            self.log.warning("failed retrieving topic (%s)..." %(topic))
            if self.__isCreateAllowed: 
                self.log.info("creating new topic (%s)" %(topic))
                theTopic = self.__topicManagerPrx.create(topic)

        self.log.debug("subscribing to topic \"%s\"." %(topic))
        theTopic.subscribeAndGetPublisher(EventSubscriber.__qos, eventSink)

        # if another subscriber has the topic, just unsubscribe
        if topic in self.__subscribers and self.__subscribers[topic] != eventSink :
            theTopic.unsubscribe(self.__subscribers[topic])

        self.__subscribers[topic] = eventSink
        self.__topicSubscribed.append(theTopic)

    def run(self):
        if config['StateChangeTopic']:
            stateChangeSink = ProvisionStateChangeSinkI(self.__channel)
            self.subscribe(stateChangeSink, 'TianShan/Event/Provision/StateChange')
        if config['ProgressTopic']:
            progressSink = ProvisionProgressSinkI()
            self.subscribe(progressSink, 'TianShan/Event/Provision/Progress')

        communicator.waitForShutdown()
        self.log.debug('thread leaving...')

    def shutdown(self):
        self.log.debug("shutting down...")
        for topic, subscriber in self.__subscribers.items() :
            try :
                theTopic = self.__topicManagerPrx.retrieve(topic)
                theTopic.unsubscribe(subscriber)
            except IceStorm.NoSuchTopic :
                self.log.error("error unsubscribing topic (%s), not exist." %(topic))

        communicator.shutdown()

# End EventSubscriber

if __name__ == '__main__':
    print __doc__

# vim ts=4
