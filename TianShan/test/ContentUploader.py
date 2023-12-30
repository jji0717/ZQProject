#!/usr/bin/python
#
#
#   ContentUploader.py
#       schedule and upload content.
#
#   Author: fei huang
#   Date:   Sep 20th, 2006
#
#   Nov 29th, 2006 fei.huang use new content manager interface 
#   Dec 6th, 2006 fei.huang change the class hierarchy for uploaders
#   Dec 7th, 2006 fei.huang merge ingestsim into ContentUploader.
#   Dec 14th, 2006 fei.huang fixed issue when uploading interrupted, reset the file pointer.
#   Apr 12th, 2007 fei.huang add logic to allow book only request.
#

"""uploading content from local disk or multicast network to ftp server"""

try:
    import os
    from time import time
    from threading import Thread, Timer
    from ftplib import FTP
    from socket import *
    from Queue import Empty
    import struct

    import Ice, TianShanIce

    from Managers import ContentManager
    from Utils import Logger, makeTime, config, genFilename
except ImportError, ex:
    print __file__, "--> Import not completed! ", str(ex)
    raise


class ContentUploader(Thread):
    """
        common interface for content provision and uploading, this is an abstract class.
    """

    bookOnly = False

    def __init__(self, contentName, bitrate=None):
        if self.__class__.__name__ == 'ContentUploader':
            raise TianShanIce.InvalidParameter('Can not instantiate abstract class.')

        Thread.__init__(self, name=self.__class__.__name__)

        self.contentName = contentName
        self.bitrate = bitrate

        self.uploader = FTP()
        self.contentMgr = ContentManager()
        self.hostInfo = tuple()

        self.stopped = False

    def __del__(self):
        if 'uploader' in self.__dict__ and self.uploader.sock:
            self.uploader.quit()

    @staticmethod
    def setBookOnly():
        ContentUploader.bookOnly = True

    def parseURL(self, url):
        """
        detach IP address and port from the url returned by server
        """
        # (ftp://xx.xx.xx.xx, 9220/xxxx.mpg)
        t1 = url.rsplit(':', 1) 
        startIdx = t1[0].rindex('/') + 1
        # (9220, xxxx.mpg)
        t2 = t1[1].split('/')

        # (xx.xx.xx.xx, 9220)
        self.hostInfo = (t1[0][startIdx:], int(t2[0]))

    def connectToFtp(self, user='anonymous', password='anonymous@') :
        self.log.info("logging on (%s) port (%d) with user (%s)" \
                %(self.hostInfo[0], self.hostInfo[1], user))

        self.uploader.connect(self.hostInfo[0], self.hostInfo[1])
        self.uploader.login(user, password)

        self.log.info(self.uploader.getwelcome())

    def onStart(self, arg=None): pass

    def onProgress(self, url=None, name=None): pass

    def onComplete(self, arg=None): pass

    def onError(self, arg=None): pass

    def run(self): 
        num = 0
        failed = list()

        if config['UploadLimit']: provisioned = list()

        while not self.stopped:
            self.onStart()
            
            generatedName = genFilename(self.contentName, num)
            try :
                self.log.info("starting provision...")
                url = self.contentMgr.provisionPassive(generatedName, maxBitrate=self.bitrate) 

                if not self.bookOnly:
                    self.onProgress(url, generatedName)

            except TianShanIce.Storage.NoResourceException, ex:
                self.log.error(ex.message)
                break
            except Ice.ConnectionLostException, ex:
                self.log.error(ex.message)
                break
            except error, ex:
                self.log.error(str(ex))
                failed.append(generatedName)

                self.onError(generatedName)
                # continue here, ignore RDS error

            num = num + 1

            self.log.info('provision content (%s) completed!' %(generatedName))

            if config['UploadLimit']:
                if generatedName not in failed:
                    provisioned.append(generatedName) 

                if len(provisioned) > config['UploadLimit']: 
                    content = provisioned.pop(0)

                    self.log.warning('Reaching upload limit, trying to destroy (%s).' %(content))

                    try: 
                        self.contentMgr.destroyContent(content)
                    except TianShanIce.InvalidStateOfArt: 
                        self.log.error('Destroy content %s failed.' %(content))
                        # ignore the content destroy error, not to interrupt the task.
                        pass

            self.onComplete(num)
        
        if failed: self.log.warning('Following items failed to upload: %s' %(','.join(failed)))
        self.log.debug('thread leaving...')

    def shutdown(self):
        self.stopped = True
   
# end ContentUploader


class IngestsimUploader(ContentUploader):
    """
        Upload content with external program 'ingestsim'.
    """

    def __init__(self, source, contentName, timing):
        ContentUploader.__init__(self, contentName)

        self.__source = source
        self.__timing = timing

        if not config['Timeout']: config['Timeout'] = 15
            
        self.log = Logger.logger(self.__class__.__name__)

    def onStart(self, arg):
        self.log.debug('source: %s' %(self.__source))
        self.log.debug('content: %s' %(self.contentName))
        self.log.debug('timing: %d' %(self.__timing))

    def onProgress(self, url, name):
        self.log.debug("ftp url: %s" %(url))

        start = time()
        os.system('Ingestsim.exe -s %s -d %s -b %s -t %d -T %d'
                %(self.__source, url, config["LocalAddress"], config['Timeout'], self.__timing))

        self.log.debug("duration: %f" %(time() - start))

        if (time() - start) < self.__timing:
            self.log.info("End of stream. quiting...")
            self.stopped = True

    def onComplete(self, arg):
        pass

    def onError(self, arg):
        pass

    def shutdown(self):
        os.system("taskkill /F /IM Ingestsim.exe")    
        ContentUploader.shutdown(self)


# End IngestThread


class StreamUploader(ContentUploader):
    """upload content to ftp server, the comsumer."""

    def __init__(self, contentName, queue, timing=None)  :
        """ initialize for uploading regarding to the mode specified."""

        ContentUploader.__init__(self, contentName)

        self.__source = SocketFile(queue)

        self.__timer = None
        self.__timing = timing

        self.log = Logger.logger(self.__class__.__name__)

    def __del__(self):
        if self.__timer: self.__timer.cancel()

    def onStart(self): 
        pass

    def onProgress(self, url, name):
        self.parseURL(url)
        self.connectToFtp()

        self.__timer = Timer(self.__timing, self.__source.abort)
        self.__timer.start()

        self.log.info('uploading content (%s).' %(name))
        self.uploader.storbinary("STOR %s" %(name), self.__source) 

    def onComplete(self, arg=None):
        if self.__source.eof(): 
            self.stopped = True

    def onError(self, arg):
        pass

    def shutdown(self):
        self.__source.abort()

        ContentUploader.shutdown(self)


# End StreamUploader


class FileUploader(ContentUploader):
    def __init__(self, file, contentName, count=1, bitrate=None):
        ContentUploader.__init__(self, contentName, bitrate)

        self.__source = LocalFile(file)
        self.__count = count

        self.log = Logger.logger(self.__class__.__name__)

    def onStart(self): 
        pass

    def onProgress(self, url, name):
        self.parseURL(url)
        self.connectToFtp()

        self.log.info('uploading content (%s).' %(name))
        self.uploader.storbinary("STOR %s" %(name), self.__source) 

    def onComplete(self, num): 
        if num == self.__count or self.__source.eof(): 
            self.stopped = True

    def onError(self, name):
        self.__source.reset()

    def shutdown(self):
        self.__source.abort()

        ContentUploader.shutdown(self)

# End FileUploader


class MulticastClient(Thread):
    """Receiving multicast stream, the producer."""

    def __init__(self, group, port, queue, buffSize=4096) :
        Thread.__init__(self, name=self.__class__.__name__)

        self.__group, self.__port, self.__buffSize = group, port, buffSize
        self.__dataQueue = queue

        self.__s = socket(AF_INET, SOCK_DGRAM)
        self.__s.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.__s.settimeout(config["Timeout"])

        self.__stopped = False

        self.log = Logger.logger(self.__class__.__name__)

    def __del__(self):
        if self.__s: self.__s.shutdown(SHUT_RDWR)

    def bindToLocal(self, address='') :
        self.log.info("Binding to local address \'%s\'." %(address))
        self.__s.bind((address, self.__port))

    def setTimeout(self, seconds=None) :
        self.__s.settimeout(seconds)

    def addToGroup(self) :
        bytes = map(int, self.__group.split('.'))

        grpaddr = 0
        for byte in bytes : 
            grpaddr = (grpaddr << 8) | byte
        ifaddr = INADDR_ANY

        mreq = struct.pack('ll', htonl(grpaddr), htonl(ifaddr))
        self.__s.setsockopt(IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq)

    def shutdown(self):
        self.__stopped = True

    def run(self) :
        while not self.__stopped:
            try :
                data = self.__s.recv(self.__buffSize)
                self.__dataQueue.put(data)
            except timeout :
                self.log.warning("Timed out receving data, exiting...")
                break

        self.log.debug('thread leaving...')

# End MulticastClient

class SocketFile:
    def __init__(self, queue):
        self.__dataQueue = queue
        self.__eof, self.__aborted = False, False

        self.__timeout = config["Timeout"]

        self.log = Logger.logger(self.__class__.__name__)

    def eof(self):
        return self.__eof

    def abort(self):
        self.__aborted = True

    def setTimeout(self, timeout=None):
        self.__timeout = timeout

    def read(self, buffSize=4096):
        if self.__aborted:
            self.log.debug("due time, abort receiving data.")
            self.__aborted = False
            return ''
     
        data = None
        try :
            data = self.__dataQueue.get(True, self.__timeout)
        except Empty :
            self.log.info("no more data, exiting...")
            self.__eof = True
        
        return data

# End SocketFile


class LocalFile:
    def __init__(self, filename):
        self.__src = open(filename, 'rb')
        self.__eof = False
        self.__aborted = False

        self.log = Logger.logger(self.__class__.__name__)

    def __del__(self):
        if self.__src: self.__src.close()

    def abort(self):
        self.__aborted = True

    def eof(self):
        return self.__eof

    def reset(self):
        self.__src.seek(0)

    def read(self, bufferSize=4096):
        if self.__aborted:
            self.log.warning('reading aborted...')
            self.__eof = True
            return ''
            
        data = self.__src.read(bufferSize)

        # finished reading file, start over from beginning.
        if not data:
            self.reset()
            return ''

        return data

# End LocalFile

if __name__ == '__main__':
    print __doc__

# vim ts=4
