#!/usr/bin/python
#
#
#   utils.py
#       misc utilities
#
#   Author: fei huang
#   Date:   Sep 30th, 2006
#   
# Nov 23th, 2006 fei.huang minor fixes for command line parsing and shutdown routine
# Nov 24th, 2006 fei.huang
# Nov 29th, 2006 fei.huang change logger interface
# Dec 1st,  2006 fei.huang adds command line arg -d for destroy contents
# Dec 5th,  2006 fei.huang change instance count implementation, use shutdown in stead of sys.exit
# Dec 12th, 2006 fei.huang change topic configuration, seperate topics apart.
# Dec 26th, 2006 fei.huang bug in command line parsing, report error no matter channel specified or not.
#                          bug in shutdown, segmentation fault.
# Mar 26th, 2007 fei.huang add switch to view content properties.
# Mar 30th, 2007 fei.huang count option defaults to 1 when omited.
# Apr 12th, 2007 fei.huang add book only option to book session without uploading content.
# May 25th, 2007 fei.huang add support for active provision.
# Jun 7th, 2007 fei.huang add RootURL configuation key for active provision.
# Jun 14th, 2007 fei.huang refine the command line parser
# Jul 19th, 2007 fei.huang dont parse source file when doing active provision.
# Jan 15th, 2008 fei.huang add static start time
# May 15th, 2008 fei.huang UTC time supported
# May 22th, 2008 fei.huang remove RootURL from config.
# May 26th, 2008 fei.huang refine the url validation, unify udp source for udp:// scheme
# Jul 17th, 2008 fei.huang add parameter for source content type
# Sep 17th, 2008 fei.huang add export and provision (active)
# Oct 8th,  2008 fei.huang support destroy2
# Oct 13th, 2008 fei.huang list specific properties
# Oct 16th, 2008 fei.huang remove endpoint configuration for ContentStore, use param instead
# Oct 20th, 2008 fei.huang support multiple content propery query concurrently
#

"""Utilities and global declaration commonly used by TianShanTest"""

try:
    import sys, os, re
    import logging
    from logging import DEBUG, INFO, ERROR, WARNING 
    from os.path import basename, isfile
    from socket import socket, error, AF_INET, SOCK_STREAM, gethostname, gethostbyname
    from getopt import getopt, GetoptError
    from time import time, strftime, localtime, mktime, gmtime
    from urlparse import urlparse

    import Ice
except ImportError, ex:
    print __file__, "--> Import not completed! ", str(ex)
    raise
    
# global communicator for ice
communicator = Ice.initialize()

# default settings for critical services
config = { \
#        "ContentStoreEndpoint"      :   "ContentStore:tcp -h 192.168.80.135 -p 55599",
        "TopicManagerEndpoint"      :   "TianShanEvents/TopicManager:default -h 10.11.0.21 -p 10000",
        "SubscriberEndpoint"        :   "tcp -p 20100",
        "ChannelPublisherEndpoint"  :   "ChannelPublisher:tcp -h 10.15.10.250 -p 9832",
        "StateChangeTopic"          :   False,  # no subscription to this topic.
        "ProgressTopic"             :   False,
        "ProvisionMode"             :   "active",
#        "RootURL"                  :   "",
        "ProvisionInterval"         :   0,
        "UTCTime"                   :   True,
        "LogFile"                   :   None,   # no logging to file, console only
        "LogLevel"                  :   DEBUG,
        "MaxInstances"              :   1,      # how many test program should run at the same time.
        "Timeout"                   :   20,     # socket wait time.
        "UploadModule"              :   "Native",
        "UploadLimit"               :   0,      # no limit, upload without destroying previous item.
        "LocalAddress"              :   ''}

def usage() :
    """
        Help message of the test program
    """

    print 'Usage: TianShanTest.py <options> <args>\n\n\
    -h display this help and exit.\n\n\
    Service:\n\
    -e <endpoint> host and port in a form host:port\n\
    Active provision:\n\
    -s <source>   url to a the source file name eg. file://xxxx/file.mpg\n\
    -E <name>     content name to be exported, must not specify with <source>\n\
    -n <name>     Content name.\n\
    -S <seconds>  How many seconds delay from now will the provision start.\n\
    -i <interval> Interval between start and stop time in seconds.\n\
    -g <time>     start at a specific time <hh:mm:ss>.\n\
    -B <bitrate>  specify transfer bitrate <bps>.\n\
    -T <type>     source content type. (MPEG2TS|H264|MPEG2TS:VVX|H264:VV2)\n\n\
    Passive provision:\n\
    -s <source>  IP address of the multicast server, <addr:port> or path to a local file.\n\
    -n <name>    Name of the content. use source name if omited. must specify with multicast.\n\
    -c <count>   How many times the content will be passiveed, local file only. default to 1.\n\
    -C <channel> The channel name to which new content will join, create if not exist.\n\
    -t <timing>  The timing that triggers switching to a new content. multicast only\n\
    -B <bitrate> specify transfer bitrate.\n\
    -T <type>    source content type. (mpeg2ts|h264)\n\
    -b           optional switch, to book a session without uploading.\n\n\
    List:\n\
    -l <content [pattern]|channel [name]|store>. List contents, channels or volumn info.\n\n\
    Contents:\n\
    -d <n1,n2,n3,...> [true] Destroy the specified contents, force deletion if "true" specified.\n\
    -p <n1,n2,n3....> [prop1,prop2,...] Display properties for the specified content name.\n\
        properties: sourceURL \n\
                    exportURL \n\
                    isProvisioned \n\
                    provisionTime \n\
                    framerate \n\
                    resolution \n\
                    fileSize \n\
                    supportFileSize \n\
                    playTime \n\
                    bitrate \n\
                    localtype \n\
                    subtype \n\
                    md5 \n\n\
    -D <[name]> cancel a provision.\n\n\
    -a <channel [items,...]> add items under the channel, if item omited, create a channel.\n\
    -r <channel [items,...]> destroy items. if item omited, then destroy the channel.\n\
    -m <channel <old item> <new item>>  modify specified item name.\n\n\n\
    Samples:\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -s test.mpg -n testName -c 2\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -E test.mpg -n newTest\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -s test.mpg -g 08:10:00\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -s test.mpg \n\
    \tTianShanTest.py -e 192.168.0.1:10040 -s test.mpg -b\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -s 225.25.1.1:8080 -n testName -t 30\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -s test.mpg -S 30 -i 600 -B 3932160 -T H264\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -l content test*\n\
    \tTianShanTest.py -l channel channelName\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -d test*\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -d c1,c2,c3\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -p test\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -p test,test1 playTime\n\
    \tTianShanTest.py -e 192.168.0.1:10040 -p test exportURL,fileSize,playTime\n\
    \tTianShanTest.py -a tv1 p1,p2\n\
    \tTianShanTest.py -r tv2\n\
    \tTianShanTest.py -m tv1 p1 p2\n\n'
    shutdown()

def summary():
    """
        A summary of usage
    """
    print
    print 'please use \'-h\' for help.'
    print 'TianShanTest.py [-h][-l category][-s src][-n content][-c cnt][-C channel][-t timing]'
    print
    shutdown()

def parseArgs(args) :
    """parse command line arguments.

    args: command line arguments, typically sys.argv[1:]
    return a dictionary that contains all appropriate values required to run the test.
    """

    params = {'action':None}

    opts, args = getopt(args, "hd:p:l:D:s:n:e:c:C:t:B:T:ba:r:m:E:g:S:i:")
    for opt, arg in opts :
        # for help
        if opt == '-h':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            params['action'] = 'help'

        # for endpoint
        if opt == '-e':
            if params['action'] and \
               ('edit' in params['action'] or \
               'remove' in params['action'] or \
               'publish' in params['action'] or \
               'help' in params['action']):
                raise GetoptError('Ambiguous! must not exist at the same time.')
            if arg.startswith('-'): 
                raise GetoptError('missing endpoint.')

            tmp = arg.split(':')
            if len(tmp) != 2:
                raise GetoptError('Bad endpoint format. eg. <127.0.0.1:10040>')

            params['endpoint'] = 'ContentStore:tcp -h %s -p %s' %(tmp[0], tmp[1])

            continue

        # for content or channel list
        if opt == '-l':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if arg not in ('content', 'channel', 'store'): raise GetoptError('Bad category name.')
            tmp = []; tmp.append(arg)

            if args: tmp.append(args[0]) 

            params['category'] = tmp
            params['action'] = 'list'

        # for cancel provision
        if opt == '-D':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if not arg.strip('\'\" '): raise GetoptError('Empty content name!')

            params['action'] = 'cancel'
            params['name'] = arg

        # for content destroy
        if opt == '-d':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if not arg.strip('\'\" '): raise GetoptError('Empty content name!')

            params['destroy'] = arg.split(',')

            if args and (not cmp(args[0], 'true')):
                params['action'] = 'destroy2'
            else:        
                params['action'] = 'destroy'

        # for content properties
        if opt == '-p':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if not arg.strip('\'\" '): raise GetoptError('Empty content name!')

            tmp = arg.split(',')

            tmp2 = list()
            if args: tmp2 = args[0].split(',')

            params['property'] = (tmp, tmp2) 
            params['action'] = 'property'

        # for channel operations.
        if opt == '-a':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if not arg.strip('\'\" '): raise GetoptError('Empty channel name!')
            tmp = []
            tmp.append(arg)

            if args: tmp.append(args[0].split(','))

            params['publish'] = tmp 
            params['action'] = 'publish'
            
        if opt == '-r':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if arg.startswith('-') or not arg.strip('\'\" '): raise GetoptError('Empty channel name!')
            tmp = []
            tmp.append(arg)

            if args: tmp.append(args[0].split(','))

            params['remove'] = tmp 
            params['action'] = 'remove'

        if opt == '-m':
            if params['action']: raise GetoptError('Ambiguous! must not exist at the same time.')

            if arg.startswith('-') or not arg.strip('\'\" ') \
                    or len(args) != 2: raise GetoptError('Bad channel or program name!')

            params['edit'] = (arg, args)
            params['action'] = 'edit'
            
        # no action specified so far, do provision.
        if not params['action']: params['action'] = 'provision'

        # for content provision.
        if opt == '-s':
            if params['action'] != 'provision': 
                raise GetoptError('Ambiguous! must not exist at the same time.')
            if arg.startswith('-'): 
                raise GetoptError('missing source address.')

            if config['ProvisionMode'] == 'passive':
                if validateAddress(arg):
                    params['streaming'] = True
                    tmp = arg.split(':')
                    params['source'] = (tmp[1].strip('//'), int(tmp[2]))
                elif isfile(arg): 
                    params['streaming'] = False
                    params['source'] = arg
                else : raise GetoptError('Bad source file!')
            elif config['ProvisionMode'] == 'active': 
                if urlparse(arg)[0] == 'udp':
                    if not validateAddress(arg):
                        raise GetoptError('Bad source URL eg. <udp://225.25.1.1:1234>')
                else:
                    if not validateURL(arg):
                        raise GetoptError('Bad source URL eg. <ftp://192.168.0.1/test>')

                params['source'] = arg 

        if opt == '-E':
            if params['action'] != 'provision' or 'source' in params:
                raise GetoptError('Ambiguous! must not exist at the same time.')

            config['ProvisionMode'] = 'active'

            params['export'] = True
            params['source'] = arg

        

        if opt == '-n':
            if params['action'] != 'provision': 
                raise GetoptError('Ambiguous! must not exist at the same time.')
            if arg.startswith('-') or not arg.strip('\'\" '): raise GetoptError('missing or bad content name.')
            params['contentName'] = arg

        if opt == '-c':
            if params['action'] != 'provision': 
                raise GetoptError('Ambiguous! must not exist at the same time.') 

            try : params['count'] = int(arg)
            except ValueError : raise GetoptError('Bad count number.')

        if opt == '-g':
            if params['action'] != 'provision': 
                raise GetoptError('Ambiguous! must not exist at the same time.') 

            try:
                strptime(arg, '%H:%M:%S')
            except ValueError: raise GetoptError('bad time format')

            t1 = localtime()
            t2 = '%d:%d:%dT%s' %(t1[0], t1[1], t1[2], arg)
            t3 = strptime(t2, '%Y:%m:%dT%H:%M:%S')
            t4 = mktime(t3)

#            if t4 < time(): raise GetoptError('invalid time')

            params['specific'] = t4

        if opt == '-B':
            if params['action'] != 'provision': 
                raise GetoptError('Ambiguous! must not exist at the same time.') 

            try : params['bitrate'] = int(arg)
            except ValueError : raise GetoptError('Bad transfer bitrate.')

        if opt == '-T':
            if params['action'] != 'provision': 
                raise GetoptError('Ambiguous! must not exist at the same time.')
            
            if arg not in ('MPEG2TS', 'H264', 'MPEG2TS:VVX', 'H264:VV2'):
                raise GetoptError('Unknown source content type')

            params['sourceType'] = arg


        # for passive provision
        if opt == '-C':
            if params['action'] != 'provision' or config['ProvisionMode'] != 'passive': 
                raise GetoptError('Ambiguous! must not exist at the same time.')

            if arg.startswith('-') : raise GetoptError('missing channel number.')
            params['channel'] = arg

        if opt == '-t':
            if params['action'] != 'provision' or config['ProvisionMode'] != 'passive': 
                raise GetoptError('Ambiguous! must not exist at the same time.')

            try : params['timing'] = float(arg) * 60
            except ValueError: raise GetoptError('Bad timing value!')

        if opt == '-b':
            if params['action'] != 'provision' or config['ProvisionMode'] != 'passive': 
                raise GetoptError('Ambiguous! must not exist at the same time.')

            params['book'] = True

        
        # for active provision
        if opt == '-S' or opt == '-i':
            if params['action'] != 'provision' or config['ProvisionMode'] != 'active': 
                raise GetoptError('Ambiguous! must not exist at the same time.')

            try:
                if opt == '-S':
                    params['start'] = int(arg)
                elif opt == '-i':
                    params['interval'] = int(arg)
            except ValueError: 
                raise GetoptError('Bad timing value!')


    # wrong flag.
    if not params['action']: raise GetoptError('Unknown arguments!')

    # critical for every operation?
    if params['action'] != 'help'   and \
       params['action'] != 'edit'   and \
       params['action'] != 'remove' and \
       params['action'] != 'publish'and \
       'endpoint' not in params:
        raise GetoptError('please specify <endpoint>.')

    # nothing to do for operations other than provision.
    if params['action'] != 'provision': return params

    # critical for provision.
    if 'source' not in params:
        raise GetoptError('please specify <source>.')

    if 'contentName' not in params: 
        params['contentName'] = basename(params['source'])

    if 'count' not in params:
        params['count'] = 1

    if 'bitrate' not in params:
        params['bitrate'] = 3932160

    if 'sourceType' not in params:
        params['sourceType'] = 'mpeg2ts'
 
    # invalidate passive provision params
    if config['ProvisionMode'] == 'passive':
        # no channel needed when not subscribed to any topics
        if (config['StateChangeTopic'] or config['ProgressTopic']) and 'channel' not in params:
            raise GetoptError('please specify <channel>.')
        elif (not config['StateChangeTopic'] and not config['ProgressTopic']) and 'channel' in params:
            raise GetoptError('No <channel> needs to be specified.')

        # needs some additional flags depending on origin of the source.
        if params['streaming'] and ((not 'contentName' in params) or (not 'timing' in params)):
            raise GetoptError("please specify \'contentName\' and \'timing\'.")
    
    if config['ProvisionMode'] == 'active':
        # start immediately
        if not 'start' in params:
            params['start'] = 0
        # interval defaults to 3600 seconds
        if not 'interval' in params:
            params['interval'] = 60 * 60  

        if config['StateChangeTopic'] or config['ProgressTopic']:
            params['channel'] = None

    return params

def makeTime(start=None, interval=3600, specific=False):
    """ make a pair of time (start - end) within a certain range.

    interval: interval which the time pair generated will base on.
              unit of interval is second.

    return a tuple contains start and end time in UTC format based on interval provided
    """

    now = time();
      
    if not start: 
        startTime = now
    elif specific == False:
        startTime = now + start
    else:
        startTime = start

    endTime = startTime + interval

    if config['UTCTime']:
        startTime, endTime = gmtime(startTime), gmtime(endTime)
    else:
        startTime, endTime = localtime(startTime), localtime(endTime)

    tstart = strftime("%Y-%m-%dT%H:%M:%S", startTime)
    tend   = strftime("%Y-%m-%dT%H:%M:%S", endTime)

    return (tstart, tend)


def genFilename(baseName, num) :
    """ generate a file name based on the base name and a range from 1 to num."""

    tup = baseName.split('_')
    fileName = "%s%d_%s" %(tup[0], num + 1, tup[1])

    return fileName

class Logger:
    """
        provide logging facility, a simple wrap of the logging module.
    """

    (file, console) = [None]*2

    @staticmethod
    def setup(stream=None):
        if config['LogFile']:
            try:
                Logger.file = logging.FileHandler(config['LogFile'], 'a+')
            except IOError:
                print 'cannot open the specified log file for writing, logging to file disabled.'
                print
            else:
                Logger.file.setLevel(config['LogLevel'])
                Logger.file.setFormatter(
                    logging.Formatter("%(asctime)s %(name)s -> %(levelname)s: %(message)s", "%y-%m-%d %H:%M:%S"))

                logging.getLogger('').addHandler(Logger.file)

        Logger.console = logging.StreamHandler(stream)
        Logger.console.setLevel(config['LogLevel'])
        Logger.console.setFormatter(logging.Formatter("%(levelname)s \"%(name)s\" -> %(message)s"))
        
        logging.getLogger('').addHandler(Logger.console)

        logging.getLogger('').setLevel(config['LogLevel'])
    
    @staticmethod
    def cleanup():
        if Logger.console: logging.getLogger('').removeHandler(Logger.console)
        if Logger.file:    logging.getLogger('').removeHandler(Logger.file)
        logging.shutdown()

    @staticmethod
    def logger(loggerName):
        return logging.getLogger(loggerName) 


def loadConfig() :
    """
        Read configuration file, the file name is hard-coded 
        and the corresponding physical file must exists
    """

    configFile = open("TianShanTest.conf", "r")

    badKeys = []
    while True :
        line = configFile.readline()

        # end of file?
        if not line : break

        # remove the leading and trailing spaces and tabs
        line = line.strip("\t\r\n ")
        # if it remains nothing or just comments, skip
        if not line or line.startswith('#') : continue
        # keep the useful stuff
        pairs = line.split('=')

        property = pairs[0].rstrip("\t ")

        # if the value part contains just empty strings, or even no value part, ignore it.
        if len(pairs) < 2 or not pairs[1].strip('\t\"\' ') :
            badKeys.append(property)
            continue

        value = pairs[1].strip('\t\"\' ')

        try :
            if property == 'MaxInstances':
                value = int(value)
            elif property == 'Timeout':
                if float(value) <= 0: value = None
                else: value = float(value)
            elif property == 'StateChangeTopic' or property == 'ProgressTopic':
                if value not in ('True', 'False'):
                    badKeys.append(property)
                    continue
                value = eval(value)
            elif property == 'UTCTime':
                if value not in ('True', 'False'):
                    badkeys.append(property)
                    continue
                value = eval(value)
            elif property == 'ProvisionInterval':
                if int(value) < 0: value = 0
                else: value = int(value)
            elif property == 'LogLevel':
                if value not in ('DEBUG', 'INFO', 'WARNING', 'ERROR'):
                    badKeys.append(property)
                    continue
                value = eval(value)
            elif property == 'UploadModule':
                if value not in ('Ingestsim', 'Native'):
                    badKeys.append(property)
                    continue
            elif property == 'UploadLimit':
                if int(value) < 0: value = 0
                else: value = int(value)
        except ValueError:
            badKeys.append(property)
            continue

        config[property] = value

    # no value specified, not in loop coz the log property might not have been loaded yet
    if badKeys: sys.stderr.write('property (%s) not set correctly, will be disabled.\n\n' %(','.join(badKeys)))

    configFile.close()

def validateAddress(address):
    """
        validate host address, in the form of <address:port>
    """

    # <udp://225.25.1.1:1234>
    pattern = "^udp://([0-5]{1,3}\.[0-5]{1,3}\.[0-5]{1,3}\.[0-5]{1,3}):(\d+$)"    

    return re.search(pattern, address)

def validateURL(url):
    """ 
        validate source url for active provision. <proto://address/>
    """
    # <ftp://a:b@x.x.x.x/yyy>
    pattern = "^\S{1,5}://(\S+:\S+@)?(\d{1,3}\.){3}\d{1,3}(/)[0-9a-zA-Z\-]+"

    return re.search(pattern, url)


def getInstanceCount() :
    """
        To determine how many test instances are running, 
        and get the instance count for calling thread.
    """

    tup = config["SubscriberEndpoint"].split()
    try :
        s = socket(AF_INET, SOCK_STREAM)
        s.bind(('', int(tup[-1])))
    except ValueError :
        Logger.logger('main').error("Invalid port number within \"SubscriberEndpoint\".")
        return 0
    except error :
        Logger.logger('main').warning("Another instances might be running.")
        # I'm not the first one, let's see what the last port num is
        try :
            tmpFile = open(".TianShan", "r+")
            lines = tmpFile.readlines()

            if not lines: return 0

            currPortNum = int(lines[0]) + 1
            id = int(lines[-1]) + 1

            if id > config["MaxInstances"] : 
                Logger.logger('main').warning("only %d instance(s) allowed!" %(config["MaxInstances"]))
                tmpFile.close()
                return 0

            tmpFile.seek(0)
            tmpFile.write("%s\n%d" %(currPortNum, id))
            tmpFile.flush()
            tmpFile.close()

            tup[-1] = str(currPortNum)
        except (IOError, ValueError), ex :
            Logger.logger('main').error("update instances number error, instance Id disabled!")
            return 0

        config["SubscriberEndpoint"] = ' '.join(tup)
        return id

    # binding successfully, I'm the first instance
    try :
        # write the first port number to the tmp file
        tmpFile = open(".TianShan", "w+")
        # current port num + instance number 
        tmpFile.write("%s\n%d" %(tup[-1], 1))
        tmpFile.flush()
        tmpFile.close()
    except IOError, ex :
        Logger.logger('main').error("can't write port number, instance Id disabled!")
        return 0

    return 1

def updateInstanceCount():
    """
        Decrease the count of running instances by one when any instance exit.
    """
    try:
        tmpFile = open(".TianShan", "r+")
    except IOError: return

    lines = tmpFile.readlines()

    if not lines: return

    if int(lines[-1]) - 1:
        tmpFile.seek(0)
        tmpFile.write("%d%d" %(int(lines[0])-1, int(lines[-1])-1))
        tmpFile.flush()
    tmpFile.close()

def shutdown(threads=None, exit=True):
    """
        Shut down the running threads and clean up resources
    """

    if threads:
        updateInstanceCount()
        for th in threads:
            Logger.logger('main').info('shutting down thread [%s].' %(th.__class__.__name__))
            th.shutdown()
            # wait till the thread __really__ exits...
            th.join()

    if exit:
        if communicator: communicator.destroy()
        Logger.cleanup()
        sys.exit(0)


if __name__ == '__main__':
    print __doc__    

# vim ts=4
