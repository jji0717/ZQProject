#!/usr/bin/python
#
#
#   TianShanTest.py
#       uploading content from content store to RDS,
#       and trace the state change of content files
#       by subscribing through IceStorm
#
#   Author: fei huang
#   Date:   Sep 25th, 2006
#
#   Nov 29th 2006 fei.huang adds feature for listing current channels or contents
#   Dec 1st 2006  fei.huang adds feature for delete specified contents
#   Dec 5th 2006  fei.huang minor fixes
#   Dec 6th 2006  fei.huang adds channel manipulation. command handler added
#   Dec 7th 2006  fei.huang move ingestsim thread to ContentUploader
#   Dec 12th 2006  fei.huang event subscriber can now be disabled by configuration.
#   Dec 14th 2006  fei.huang change the quit behavior, join() with timeout seems work.
#   Dec 19th 2006 fei.huang devide main into small parts to it easier be invoked by GUI
#   Dec 20th 2006 fei.huang seperate provision exception handler to main
#   Dec 22th 2006 fei.huang add test for python compiler and compiled files
#   Dec 25th 2006 fei.huang test can now compile ice files automatically if not present.
#   Jan 12th 2007 fei.huang move event subscribers out, leave all topic related operations to EventSubscriber
#   Mar 26th 2007 fei.huang add routine to view content properties. avoid unnecessary creation of channel 
#                           or content manager.
#   Mar 28th 2007 fei.huang use environ ICE32 to determine new ice installation path.
#   Apr 12th 2007 fei.huang add book only request support.
#   May 25th 2007 fei.huang add support for active provision.
#   Jun 25th 2008 fei.huang upgraded to ICE 3.2
#	Jul 17th 2008 fei.huang specify source content type for provision
#   Aug 10th 2008 fei.huang TianShanIce updated to ICE 3.2
#   Oct 8th  2008 fei.huang support destroy2
#   Oct 20th 2008 fei.huang support query multiple contents property concurrently 
#

"""
    Main entry of TianShanTest 
"""

import sys, os

# check dependencies
#

# coz the c++ projects now are using ice version that does not include python support,
# a newer version is required to run this application, so when both version of ice are
# installed, I will switch the current context to the new ice runtime temporarily for 
# this session.
#try :
#    os.environ['path'] = os.environ['path'].lower().replace(
#            '%s\\bin' %(os.environ['ICE_ROOT'].lower()), '%s\\bin' %(os.environ['ICE32'].lower()), 1)
   
#    sys.path.append(os.path.join(os.environ['ICE32'], 'python'))
#except KeyError, ex:
#    print ex.message
#    sys.exit(1)

try:
    py = os.path.join(os.environ['ICE_ROOT'], 'python')
    if not os.path.exists(py):
        print 'python not supported by current ICE version.'
        sys.exit(1)

    sys.path.append(py);

except KeyError, ex:
    print ex.message
    sys.exit(1)


# if ice modules not present, compile it automatically
if not os.path.exists('TianShanIce') or not os.path.exists('ChannelOnDemand'):
    print 'Compiling Ice files...'

    import shutil

    icePath = os.environ['ICE_ROOT']

    try:
        zqPath = os.environ['ZQPROJSPATH']
    except KeyError, ex:
        print ex.message
        sys.exit(1)

    path  = os.path.join(zqPath, 'TianShan\\Ice')
    path2 = os.path.join(zqPath, 'TianShan\\ChannelOnDemand')
    path3 = os.path.join(zqPath, 'TianShan\\ContentStore\\ICE')
    path4 = os.path.join(icePath, 'slice')

    depends = ('TianShanIce.ice',
               'TsStorage.ice',
               'TsEvents.ice',
               'TsApplication.ice',
               'TsStreamer.ice',
               'TsSRM.ice',
               'TsTransport.ice',
               'TsStorageMediaCluster.ice',
               'ChannelOnDemand.ice',
               'ZQIceVersion.ICE')
    
    try:
        for file in depends[:-3]:
            shutil.copyfile(os.path.join(path, file), os.path.join('.', file))

        shutil.copyfile(
            os.path.join(path4, depends[-1]), os.path.join('.', depends[-1]))

        shutil.copyfile(
            os.path.join(path2, depends[-2]), os.path.join('.', depends[-2]))

        shutil.copyfile(
            os.path.join(path3, depends[-3]), os.path.join('.', depends[-3]))

        compiler = os.path.join(icePath, 'bin\\slice2py.exe')
        params = '-I %s ' %(os.path.join(icePath, 'slice'))

        command = '%s %s %s' %(compiler, params, ' '.join(depends))
        os.system(command)
    except IOError, ex:
        print 'compile ice files error! Has to be compiled manually. (%s)' %(ex.filename)
        sys.exit(1)
 

#------------------------------------------------------------------
# can not suppress the warning message while loading the deprecated
# ice file, will not be used until a newer version is adopted.
#------------------------------------------------------------------
#
# load ice files dynamically instead of copy them over and compile.
#
#import Ice

#try:
#    icePath = os.environ['ICE_ROOT']
#    zqPath = os.environ['ZQPROJSPATH']

#    path = os.path.join(zqPath, 'TianShan\\Ice')
#    path2 = os.path.join(zqPath, 'TianShan\\ChannelOnDemand')

#    params = '--all -I%s -I%s' %(os.path.join(icePath, 'slice'), path)

#    Ice.loadSlice('%s %s %s' %(params, os.path.join(path, 'TsStorage.ice'), 
#                               os.path.join(path2, 'ChannelOnDemand.ice')))

#except (RuntimeError, KeyError), ex:
#    print ex.message
#    sys.exit(1)

try:
    from getopt import GetoptError
    from Queue import Queue
    import thread

    from TianShanIce import InvalidParameter, InvalidStateOfArt
    from TianShanIce.Storage import NoResourceException 

    from time import sleep
    from Utils import *
    from ContentUploader import ContentUploader, StreamUploader, FileUploader, IngestsimUploader, MulticastClient
    from EventSubscriber import EventSubscriber 
    from Managers import *
except ImportError, ex:
    print __file__, '--> import not completed! ', str(ex)
    sys.exit(1)


def commandHandler(args):
    """ handles variant requests other than provision.

    args    -- command map generated from the command-line options
    output  -- There is output to the console by default, no output if 
               this parameter is set to False.
    """

    if ('list' in args['action'] and \
        ('content' in args['category'] or 'store' in args['category'])) \
        or 'destroy' in args['action'] or 'property' in args['action'] \
        or 'cancel' in args['action']:
        contentMgr = ContentManager(args['endpoint'])

    if ('list' in args['action'] and 'channel' in args['category']) \
        or 'publish' in args['action'] \
        or 'remove' in args['action']  \
        or 'edit' in args['action']:
        channelMgr = ChannelManager()

    failed = list()

    # command list
    if 'list' in args['action']:
        if 'content' in args['category']:
            if len(args['category']) > 1: 
                return contentMgr.listContents(args['category'][-1])
            else: return contentMgr.listContents()
        elif 'channel' in args['category']:
            if len(args['category']) > 1: 
                return channelMgr.listChannels(args['category'][-1])
            else: return channelMgr.listChannels()
        elif 'store' in args['category']:
            return contentMgr.listContentStoreProperties()

    # destroy content
    elif 'destroy' in args['action'] or 'destroy2' in args['action']:
        force = True if 'destroy2' in args['action'] else False

        if len(args['destroy']) == 1 and ('*' in args['destroy'][0] or '?' in args['destroy'][0]):
            args['destroy'] = contentMgr.listContents(args['destroy'][0])
        for content in args['destroy']: 
            try: 
                contentMgr.destroyContent(content, force)
            except TianShanIce.InvalidStateOfArt, ex: 
                Logger.logger('main').error(ex.message)
                failed.append(content)
                continue

    # list content properties
    elif 'property' in args['action']:
        for content in args['property'][0]:
            try:
                contentMgr.listProperties(content, args['property'][1]) 
            except TianShanIce.InvalidStateOfArt, ex:
                Logger.logger('main').error(ex.message)
                failed.append(content)
                continue

    # cancel content provision
    elif 'cancel' in args['action']:
        try:
            contentMgr.cancelProvision(args['name'])
        except TianShanIce.InvalidStateOfArt, ex:
            Logger.logger('main').error(ex.message)
            failed.append(args['name'])

    # publish channel and append item
    elif 'publish' in args['action']:
        # for channel only
        channel = channelMgr.openChannel(args['publish'][0], True)
        Logger.logger('main').info('Publising channel (%s).' %(args['publish'][0]))
        # channel name and item name
        if len(args['publish']) > 1: 
            try:
                [channelMgr.insertItem(channel, item) for item in args['publish'][-1]]
            except TianShanIce.InvalidParameter:
                failed.append(item)

    # remove channel or item
    elif 'remove' in args['action']:
        channel = channelMgr.openChannel(args['remove'][0], False)
        # channel name and item name
        if len(args['remove']) > 1:
            try:
                [channelMgr.removeItem(channel, item) for item in args['remove'][-1]]
            except TianShanIce.InvalidParameter:
                failed.append(item)
        else: channelMgr.destroyChannel(channel)

    # edit a program
    elif 'edit' in args['action']:
        channel = channelMgr.openChannel(args['edit'][0], False)
        try:
            channelMgr.editItem(channel, args['edit'][1][0], args['edit'][1][1])
        except TianShanIce.InvalidParameter:
            failed.append(item)

    if failed: 
        raise TianShanIce.InvalidParameter('main', 0, \
            'Operation failed on following items: (%s).' %(','.join(failed)))


def doProvision(args):
    # get my instance number in case multi-instances are running
    id = getInstanceCount()
    if not id: sys.exit(1)

    sys.stdout.write("%d Instance(s) running...\n\n" %(id))

    # threads to be initialized 
    threads = list()

    if config['StateChangeTopic'] or config['ProgressTopic']:
        # event subscriber 
        eventSubscriber = EventSubscriber(args['channel'])
        eventSubscriber.start()
        threads.append(eventSubscriber)

    if config['ProvisionMode'] == 'passive':
        if 'book' in args:
            ContentUploader.setBookOnly()

        # source = multicast stream
        if args['streaming']:
            # Ingestsim or native class
            if config['UploadModule'] == 'Ingestsim':
                source = '%s:%d' %(args['source'][0], args['source'][1])
                ingestsimUploader = IngestsimUploader(source, args['contentName'], args['timing'])

                ingestsimUploader.start()

                threads.append(ingestsimUploader)
            # Multicast client and content uploader
            elif config['UploadModule'] == 'Native':
                dataQueue = Queue()

                streamUploader = StreamUploader(args['contentName'], timing=args['timing'], queue=dataQueue)

                multicastClient = MulticastClient(args['source'][0], args['source'][1], queue=dataQueue)
                multicastClient.bindToLocal(config["LocalAddress"])
                multicastClient.addToGroup()

                multicastClient.start()
                streamUploader.start()

                threads.extend([multicastClient,streamUploader])
        # source = local disk file 
        else:
            fileUploader = FileUploader(args['source'], args["contentName"], args["count"], args['bitrate'])
            fileUploader.start()

            threads.append(fileUploader)

    if config['ProvisionMode'] == 'active':
        contentMgr = ContentManager(args['endpoint'])

        (num, interval) = (0, args['start'])

        (totalTime, average) = [0.0] * 2

        (min, max) = [(0.0, '')] * 2 

        while num < args['count']:
            if args['count'] == 1:
                generatedName = args['contentName']
            else:
                generatedName = genFilename(args['contentName'], num) 

            try:
                tm = None
                if 'specific' in args:
                    tm = args['specific']

                export = True if 'export' in args else False
                t = contentMgr.provisionActive(generatedName, \
                        args['source'], \
                        interval, \
                        args['interval'],\
                        export,\
						args['sourceType'], \
                        tm, \
                        maxbitrate=args['bitrate'])

                interval += args['interval'] + config['ProvisionInterval']

                totalTime += t

                if not min[0] or not max[0]:
                    min = max = (t, generatedName)

                if t < min[0]:
                    min = (t, generatedName)
                if t > max[0]:
                    max = (t, generatedName)

            except (NoResourceException, InvalidParameter, InvalidStateOfArt), ex:
                Logger.logger('main').error(ex.message)
                break
            except Ice.Exception, ex:
                Logger.logger('main').error(ex.ice_name())
                break
            
            num += 1

        if num:
            average = totalTime / num
        else:
            average = totalTime

        Logger.logger('main').info(\
                '\n\ttotal: (%.3f ms)\
                 \n\tsessions: (%d)\
                 \n\tmin: [%.3f ms, %s]\
                 \n\tmax: [%.3f ms, %s]\
                 \n\taverage: (%.3f ms)'\
                %(totalTime, num, min[0], min[1], max[0], max[1], average))	

    return threads


def initialize():
    # try to load arguments from configuration file
    try:
        loadConfig()
    except IOError, ex:
        sys.stderr.write("Failed to load configuration!\n")
        # no configuration file specified, quit
        shutdown()

    # setting up Logger. handlers
    Logger.setup()

    # if command line arguments provided
    if sys.argv[1:]:
        try:
            args = parseArgs(sys.argv[1:])
        except (GetoptError, IOError), ex :
            Logger.logger('main').error('Invalid arguments: ' + ex.args[0])
            summary()

        # option help
        if 'help' in args['action']: usage()
        # option list or option destroy
        try:
            if 'provision' not in args['action']: commandHandler(args); shutdown()
        except (Ice.ConnectionRefusedException, Ice.ConnectionLostException, 
                Ice.ConnectFailedException, Ice.SocketException):
            Logger.logger('main').error('Failed connecting to the remote server.')
            shutdown()
        except (TianShanIce.InvalidStateOfArt, TianShanIce.ServerError, TianShanIce.InvalidParameter), ex:
            Logger.logger('main').error('Failed to accomplish request: ' + ex.message)
            shutdown()

    # must specify parameters now
    else:
        sys.stderr.write('missing arguments!\n')
        summary()

    return args


def main():
    args = initialize()

    threads = list()
    try:
        threads = doProvision(args) 
    except (Ice.ConnectFailedException, Ice.ConnectionLostException), ex:
        Logger.logger('main').error('Ice connection error:' + os.strerror(ex.error))
        shutdown()
    except (NoResourceException, TianShanIce.InvalidParameter, TianShanIce.InvalidStateOfArt), ex:
        Logger.logger('main').error(ex.message)
        shutdown(threads)
    except (Ice.SocketException, Ice.TimeoutException), ex:
        Logger.logger('main').error("Ice communication error.")
        shutdown()
    except KeyboardInterrupt:
        Logger.logger('main').warning('User Interrupt...')
        shutdown()

    try:
        while threads:
            for th in threads: 
                try:
                    th.join(100)
                except thread.error:
                    Logger.logger('main').debug('release lock error.')
                    raise KeyboardInterrupt

                if not th.isAlive(): threads.remove(th)
    except KeyboardInterrupt:
        Logger.logger('main').warning('User Interrupt...')

    Logger.logger('main').info('main program exiting...')
    shutdown(threads)

if __name__ == '__main__': 
    main()


   

# vim ts=4
