#!/usr/bin/python
#
#
#   Managers.py
#       operations to manipulate contents and channels.
#
#   Author: fei huang
#   Date:   Nov 24th, 2006
#
#   Nov 29th, 2006 fei.huang adds channel manager
#   Dec 6th,  2006 fei.huang modify implementation of item insertion.  
#   Mar 26th, 2007 fei.huang add interface to list content properties.
#   May 25th, 2007 fei.huang add interface for active provision. 
#	Aug 1st,  2007 fei.huang try open before provision (to simulate LAM logic)
#	Aug 2nd,  2007 fei.huang add cancel provision routine.
#	Aug 21st, 2007 fei.huang add content store properties.
#   May 30th, 2008 fei.huang export cifs url for NAS
#   Jul 17th, 2008 fei.huang add source content type parameter
#   Aug 13th, 2008 fei.huang add localtype property
#   Oct 13th, 2008 fei.huang remove support for GUI, list specific item properties
#


""" 
    Content and Channel related operations 
"""

try:
    import sys
    from time import time

    from os import path

    import TianShanIce, TianShanIce.Storage, ChannelOnDemand 

    from Utils import Logger, makeTime, config, communicator
except ImportError, ex:
    print "critical error, import not completed! ", str(ex)
    raise

class ContentManager:

    def __init__(self, endpoint):
        base = communicator.stringToProxy(endpoint)
        self.__contentStore = TianShanIce.Storage.ContentStorePrx.checkedCast(base)
        self.__properties = dict()

        self.__type = self.__contentStore.type()

        self.log = Logger.logger(self.__class__.__name__)

    def provisionActive(self, name, sourceURL, startTime, interval, \
            export=False, sourceType=TianShanIce.Storage.ctMPEG2TS, specific=None,\
            overwrite=True, createAllowed=True, maxbitrate=3932160):

        if specific:
            tm = makeTime(specific, interval, True)
        else:
            tm = makeTime(startTime, interval)

        # try open to see if the content exists.
        (t1, t2, diff) = [0.0]*3
#        try:
#            self.__contentStore.openContent(name, ContentManager.CONTENT_TYPE, False)
#            self.__contentStore.openContent(name, ContentManager.CONTENT_TYPE, True)
#       except TianShanIce.InvalidStateOfArt:
        t1 = time()

        if export:
            content = self.__contentStore.openContent(sourceURL, sourceType, False)
            (url, ttl, prop) = content.getExportURL(TianShanIce.Storage.potoFTP, maxbitrate)
            content = self.__contentStore.openContent(name, sourceType, createAllowed)
            content.provision(url, sourceType, overwrite, tm[0], tm[1], maxbitrate)
        else:
            content = self.__contentStore.openContent(name, sourceType, createAllowed)
            content.provision(sourceURL, sourceType, overwrite, tm[0], tm[1], maxbitrate)

#       else:
#            raise TianShanIce.InvalidStateOfArt(self.__class__.__name__, 0, \
#                                                'content exists, cannot ingest an existing content.')

        t2 = time()

        diff = (t2 - t1) * 1000

        self.log.info('Session booked successfully. takes (%.3f ms)' %(diff))
        self.log.info('sourceURL: [%s], contentName: [%s], start: [%s], stop: [%s]' %(sourceURL, name, tm[0], tm[1]))

        return diff

    def provisionPassive(self, contentName, overwrite=True, createAllowed=True, maxBitrate=3932160):
        tm = makeTime()

        content = self.__contentStore.openContent(contentName, TianShanIce.Storage.ctMPEG2TS, createAllowed)
        url = content.provisionPassive(TianShanIce.Storage.ctMPEG2TS, overwrite, tm[0], tm[1], maxBitrate)

        return url

    def listContents(self, pattern='*'):
        contents = self.__contentStore.listContent(pattern)
        if not contents: 
            raise TianShanIce.InvalidStateOfArt(self.__class__.__name__, 0, 'No content(s) found!')

        [sys.stdout.write('%s\n' %(item)) for item in contents]

        return contents

    def listContentStoreProperties(self):
        sys.stdout.write("netID:\t%s\n" %(self.__contentStore.getNetId()))
        sys.stdout.write("type: \t%s\n" %(self.__type))

        (free, total) = self.__contentStore.getCapacity()
        sys.stdout.write("free: \t%dMB\n" %(free))
        sys.stdout.write("total:\t%dMB\n" %(total))

    def __populateProp(self, obj):
        prop = dict()
        prop['sourceURL'] = obj.getSourceUrl
        prop['exportURL'] = obj.getExportURL
        prop['isProvisioned'] = obj.isProvisioned
        prop['provisionTime'] = obj.getProvisionTime
        prop['framerate']  = obj.getFramerate
        prop['resolution'] = obj.getResolution
        prop['fileSize']    = obj.getFilesize
        prop['supportFileSize'] = obj.getSupportFileSize
        prop['playTime']    = obj.getPlayTime
#        prop['playTime']  = obj.getPlayTimeEx
        prop['bitrate']     = obj.getBitRate
        prop['localtype']   = obj.getLocaltype
        prop['subtype']     = obj.getSubtype
        prop['md5'] = obj.getMD5Checksum
        
        return prop

    def listProperties(self, contentName, meta=list()):
        content_t = self.__contentStore.openContent(contentName, TianShanIce.Storage.ctMPEG2TS, False)
        content = TianShanIce.Storage.MediaClusterContentPrx.uncheckedCast(content_t) 

        propertyMap = self.__populateProp(content)

        if not meta:
            [meta.append(item[0]) for item in propertyMap.items()]
        
        sys.stdout.write('----------- %s -----------\n' %contentName)

        for item in meta:
            if item == 'exportURL': 
                (proto, prop, url) = (None, None, None);

                if self.__type == TianShanIce.Storage.csNAS:
                    proto = TianShanIce.Storage.potoCIFS
                else: 
                    proto = TianShanIce.Storage.potoFTP

                try:
                    (url, ttl, prop) = propertyMap['exportURL'](proto, 3932160)
                except: pass

                sys.stdout.write('%s:\t%s\n' %(item, url))
                if prop and prop.items():
                    sys.stdout.write('\n****** exported properties ******\n')

                    [sys.stdout.write('%s --> %s\n' %(property[0], property[1])) for property in prop.items()]

                    sys.stdout.write('****** exported properties ******\n\n')

                continue
                
            try:
                sys.stdout.write('%s:\t%s\n' %(item, propertyMap[item]())) 
            except:
                pass

    def cancelProvision(self, contentName):
        content = self.__contentStore.openContent(contentName, TianShanIce.Storage.ctMPEG2TS, False)
        content.cancelProvision()

    def destroyContent(self, contentName, force=False):
        content = self.__contentStore.openContent(contentName, TianShanIce.Storage.ctMPEG2TS, False)

        if not force:
            content.destroy()
        else:
            content.destroy2(True)

# end ContentManager


class ChannelManager:
    def __init__(self):
        base = communicator.stringToProxy(config['ChannelPublisherEndpoint'])
        self.__publisher = ChannelOnDemand.ChannelPublisherPrx.checkedCast(base);

        self.__channel = None
        self.log = Logger.logger(self.__class__.__name__)

    def openChannel(self, channelName, createAllowed=True, desc='py_test'):
        self.log.debug('opening channel (%s).' %(channelName))
        theChannel = None
        try:
            theChannel = self.__publisher.open(channelName)
        except TianShanIce.InvalidParameter:
            if createAllowed:
                self.log.warning('Failed to open channel (%s), trying to publish as new.' %(channelName))
                theChannel = self.__publisher.publish(channelName, 0, desc)
            else: raise
        return theChannel
        
    def listChannels(self, channelName=None):
        channelList = {} 

        # list specific channel.
        if channelName:
            theChannel = self.openChannel(channelName, False)
            channelList[channelName] = theChannel.getItemSequence()
        # default get all channels
        else:
            channels = self.__publisher.list()
        
            for channel in channels :
                theChannel = self.openChannel(channel, False)
                channelList[channel] = theChannel.getItemSequence()
        
        if not channelList:
            raise TianShanIce.InvalidStateOfArt(self.__class__.__name__, 0, 'No channel(s) available!')

        if output: [sys.stdout.write('%s -> %s\n' %(ch, ','.join(item))) for ch, item in channelList.items()]

        return channelList
        
    def destroyChannel(self, channel):
        channel.destroy()

    def editItem(self, channel, oldItemName, newItemName):
        tm = makeTime(3600 * 24)

        newItem = ChannelOnDemand.ChannelItem()
        newItem.contentName = newItemName
        newItem.broadcastStart = tm[0]
        newItem.expiration = tm[1]

        channel.replaceItem(oldItemName, newItem)

    def insertItem(self, channel, newItemName, atItemName=None):
        tm = makeTime(3600 * 24)

        newItem = ChannelOnDemand.ChannelItem()
        newItem.contentName = newItemName
        newItem.broadcastStart = tm[0]
        newItem.expiration = tm[1]

        if not atItemName:
            self.log.warning("appending content (%s) to channel (%s)," %(newItemName, channel.getName()))
            channel.appendItem(newItem)
        else:
            self.log.warning("inserting content (%s), before (%s)," %(newItemName, atItemName))
            channel.insertItem(atItemName, newItem)

    def removeItem(self, channel, item):
        channel.removeItem(item)


# end ChannelManager

if __name__ == '__main__':
    print __doc__
