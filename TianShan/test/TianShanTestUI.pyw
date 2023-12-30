#!/usr/bin/python
#
#
#   TianShanTestUI.py
#       The GUI front end for TianShanTest      
#
#   Author: fei huang
#   Date:   Dec 12th, 2006
#
#

import wx

import os, sys
from getopt import GetoptError

from TianShanTest import doProvision, commandHandler

import Ice, TianShanIce
from TianShanIce.Storage import NoResourceException

from Utils import config, parseArgs, loadConfig, shutdown, Logger


class ProvisionPanel(wx.Panel):
    """ Panel for Provisioin """

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        # button to start and stop provision
        self.startBtn = wx.Button(self, wx.ID_ANY, 'Start')
        self.stopBtn  = wx.Button(self, wx.ID_ANY, 'Stop')

        # source entry
        sourceBox = wx.StaticBox(self, wx.ID_ANY, 'Source')

        self.streamRadio = wx.RadioButton(self, wx.ID_ANY, 'Stream', style=wx.RB_GROUP)
        self.streamHost = wx.TextCtrl(self, wx.ID_ANY) 
        self.streamPort = wx.SpinCtrl(self, wx.ID_ANY)
        self.streamPort.SetRange(1025, 99999) 

        self.localRadio = wx.RadioButton(self, wx.ID_ANY, 'Local   ') 
        self.localText = wx.TextCtrl(self, wx.ID_ANY)
        browseBtn = wx.Button(self, wx.ID_ANY, 'Browse', size=(80, -1))

        # content name entry
        contentLabel = wx.StaticText(self, wx.ID_ANY, 'Content:')
        self.contentText = wx.TextCtrl(self, wx.ID_ANY)

        # list of available channels
        channelList = commandHandler(parseArgs(['-l', 'channel'])).keys()
        channelLabel = wx.StaticText(self, wx.ID_ANY, 'Channels:')
        self.channelChoice = wx.Choice(self, wx.ID_ANY, choices=channelList) 

        # number of provisions to be performed
        countLabel = wx.StaticText(self, wx.ID_ANY, 'Count:')
        self.countSpin = wx.SpinCtrl(self, wx.ID_ANY)
        self.countSpin.SetRange(1, 999)
        self.countSpin.SetValue(1)

        # timing selection
        timingLabel = wx.StaticText(self, wx.ID_ANY, 'Timing:')
        self.timingSpin = wx.SpinCtrl(self, wx.ID_ANY, min=5, max=60)
        self.timingSpin.SetRange(5, 60)
        self.timingSpin.SetValue(30)

        # groups
        self.sourceGroup = {\
                self.streamRadio: (self.streamHost, self.streamPort, timingLabel, self.timingSpin),
                self.localRadio: (self.localText, browseBtn, countLabel, self.countSpin)}

        for ctrl in self.sourceGroup[self.localRadio]:
            [item.Enable(False) for item in self.sourceGroup[self.localRadio]]

        if not config['StateChangeTopic'] and not config['ProgressTopic']:
            channelLabel.Enable(False); self.channelChoice.Enable(False)
        self.stopBtn.Enable(False)

        # static sizer
        staticBoxSizer = wx.StaticBoxSizer(sourceBox, wx.VERTICAL)

        # source sizer
        sourceSizer = wx.FlexGridSizer(cols=3, hgap=5, vgap=5)
        sourceSizer.AddGrowableCol(1)

        sourceSizer.Add(self.streamRadio, 0, wx.ALIGN_CENTER|wx.ALL)
        sourceSizer.Add(self.streamHost, 0, wx.EXPAND|wx.ALIGN_CENTER)
        sourceSizer.Add(self.streamPort, 0, wx.ALIGN_CENTER)

        sourceSizer.Add(self.localRadio, 0, wx.ALIGN_CENTER|wx.ALL)
        sourceSizer.Add(self.localText, 0, wx.EXPAND|wx.ALIGN_CENTER|wx.TOP, 5)
        sourceSizer.Add(browseBtn, wx.LEFT)

        staticBoxSizer.Add(sourceSizer, 0, wx.EXPAND|wx.LEFT|wx.ALL, 5)

        # parameters Sizer
        paramSizer = wx.FlexGridSizer(cols=2, rows=2, hgap=10, vgap=10)
        paramSizer.AddGrowableCol(1)

        paramSizer.Add(contentLabel, 0, wx.ALIGN_RIGHT|wx.ALIGN_CENTER_VERTICAL)
        paramSizer.Add(self.contentText, 0, wx.EXPAND)

        paramSizer.Add(channelLabel, 0, wx.ALIGN_RIGHT|wx.ALIGN_CENTER_VERTICAL)
        paramSizer.Add(self.channelChoice, 0, wx.EXPAND)

        paramSizer.Add(countLabel, 0, wx.ALIGN_RIGHT|wx.ALIGN_CENTER_VERTICAL)
        paramSizer.Add(self.countSpin, 0, wx.EXPAND)

        paramSizer.Add(timingLabel, 0, wx.ALIGN_RIGHT|wx.ALIGN_CENTER_VERTICAL)
        paramSizer.Add(self.timingSpin, 0, wx.EXPAND)

        # button Sizer
        buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
        buttonSizer.Add((20,20), 1)
        buttonSizer.Add(self.startBtn)
        buttonSizer.Add((20,20), 1)
        buttonSizer.Add(self.stopBtn)
        buttonSizer.Add((20,20), 1)

        # main Sizer
        mainSizer = wx.BoxSizer(wx.VERTICAL)

        mainSizer.Add(staticBoxSizer, 0, wx.EXPAND|wx.ALL, 10)
        mainSizer.Add(paramSizer, 0, wx.EXPAND|wx.ALL, 10)
        mainSizer.Add((20,20))
        mainSizer.Add(buttonSizer, 0, wx.EXPAND|wx.BOTTOM, 10)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)
        mainSizer.SetSizeHints(self)

        # events
        self.Bind(wx.EVT_BUTTON, self.OnBrowse, browseBtn)
        self.Bind(wx.EVT_BUTTON, self.OnStart,  self.startBtn)
        self.Bind(wx.EVT_BUTTON, self.OnStop,  self.stopBtn)

        self.Bind(wx.EVT_RADIOBUTTON, self.OnSourceSelect, self.streamRadio)
        self.Bind(wx.EVT_RADIOBUTTON, self.OnSourceSelect, self.localRadio)

        # provision threads 
        self.threads = list()

    def OnBrowse(self, event):
        dlg = wx.FileDialog(
                self, message='Choose a file', defaultDir=os.getcwd(),
                defaultFile='', wildcard='mpeg file (*.mpg;*.mpeg)|*.mpg;*.mpeg)',
                style=wx.OPEN|wx.FILE_MUST_EXIST|wx.CHANGE_DIR)

        if dlg.ShowModal() == wx.ID_OK:
            self.localText.WriteText(dlg.GetPath())

        dlg.Destroy()

    def OnSourceSelect(self, event):
        selected = event.GetEventObject()

        for ctrl in self.sourceGroup:
            for item in self.sourceGroup[ctrl]:            
                if ctrl is selected: item.Enable(True) 
                else: item.Enable(False)

    def OnStart(self, event):
        input = list()
        
        input.append('-s')
        if self.streamRadio.GetValue():
            input.append('%s:%d' %(self.streamHost.GetValue(), self.streamPort.GetValue()))
            input.append('-t')
            input.append(self.timingSpin.GetValue())
        elif self.localRadio.GetValue():
            input.append(self.localText.GetValue())
            input.append('-c')
            input.append(self.countSpin.GetValue())

        input.append('-n')
        input.append(self.contentText.GetValue())

        if config['StateChangeTopic'] or config['ProgressTopic']:
            input.append('-C')
            input.append(self.channelChoice.GetStringSelection())
        
        try:
            args = parseArgs(input)
        except GetoptError, ex:
            dlg = wx.MessageDialog(self, ex.args[0], 'Error occured!', wx.OK|wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            return False

        btn = event.GetEventObject()
        btn.Enable(False)
        self.stopBtn.Enable(True)

        try:
            self.threads = doProvision(args)  
        except (Ice.ConnectFailedException, Ice.ConnectionLostException), ex:
            dlg = wx.MessageDialog(self, 'Ice connection error: ' + os.strerror(ex.error),
                    'Error occured!', wx.OK|wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            shutdown(self.threads, exit=False)
        except (NoResourceException, TianShanIce.InvalidParameter, TianShanIce.InvalidStateOfArt), ex:
            dlg = wx.MessageDialog(self, ex.message, 'Error occured!', wx.OK|wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            shutdown(self.threads, exit=False)
        except Ice.SocketException:
            dlg = wx.MessageDialog(self, 'Another instance might be running, stop it first.', 
                    'Error occured!', wx.OK|wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            shutdown(self.threads, exit=False)

    def OnStop(self, event):
        pressed = event.GetEventObject()
        pressed.Enable(False)
        self.startBtn.Enable(True)

        shutdown(self.threads, exit=False)

class ChannelPanel(wx.Panel):
    """ Panel for channel manipulation """

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        # maintain the channel list
        self.channelList = dict()

        # buttons
        self.addBtn = wx.Button(self, wx.ID_ANY, 'Add')
        self.editBtn = wx.Button(self, wx.ID_ANY, 'Edit')
        removeBtn = wx.Button(self, wx.ID_ANY, 'Remove')
        refreshBtn = wx.Button(self, wx.ID_ANY, 'Refresh')

        # channel tree control
        self.channelTree = wx.TreeCtrl(self, wx.ID_ANY, style=wx.TR_DEFAULT_STYLE|wx.SUNKEN_BORDER)
        self.buildChannelTree()

        # button sizer
        buttonSizer = wx.BoxSizer(wx.VERTICAL)
        buttonSizer.Add((20,20), 1)
        buttonSizer.Add(self.addBtn)
        buttonSizer.Add((20,20), 1)
        buttonSizer.Add(self.editBtn)
        buttonSizer.Add((20,20), 1)
        buttonSizer.Add(removeBtn)
        buttonSizer.Add((20,20), 1)
        buttonSizer.Add(refreshBtn)
        buttonSizer.Add((20,20), 1)
        
        # main sizer
        mainSizer = wx.FlexGridSizer(cols=2)
        mainSizer.AddGrowableCol(1)
        mainSizer.AddGrowableRow(0)
        mainSizer.Add(buttonSizer, 0, 
                wx.ALL|wx.EXPAND|wx.ALIGN_CENTER_VERTICAL, 20)
        mainSizer.Add(self.channelTree, 0, 
                wx.ALL|wx.EXPAND|wx.ALIGN_CENTER_VERTICAL, 20)

        # main
        self.SetSizer(mainSizer)
        mainSizer.Fit(self)
        mainSizer.SetSizeHints(self)

        # events
        self.Bind(wx.EVT_BUTTON, self.OnAdd, self.addBtn)
        self.Bind(wx.EVT_BUTTON, self.OnEdit, self.editBtn)
        self.Bind(wx.EVT_BUTTON, self.OnRemove, removeBtn)
        self.Bind(wx.EVT_BUTTON, self.OnRefresh, refreshBtn)

        self.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelectChanged, self.channelTree)

    def buildChannelTree(self):
        self.channelTree.DeleteAllItems()

        root = self.channelTree.AddRoot('Channels')

        self.channelList = commandHandler(parseArgs(['-l', 'channel']))
        for channel in self.channelList: 
            child = self.channelTree.AppendItem(root, channel)

            for item in self.channelList[channel]:
                item = self.channelTree.AppendItem(child, item)

        self.channelTree.SelectItem(root)
        self.editBtn.Enable(False)
        self.channelTree.Expand(root)

    def OnAdd(self, event):
        item = self.channelTree.GetSelection() 
        label = self.channelTree.GetItemText(item)

        # a channel selected, add item
        if label in self.channelList:
            dlg = wx.TextEntryDialog(self, 'Please enter the Content Name:')
            if dlg.ShowModal() == wx.ID_OK:
                commandHandler(parseArgs(['-a', label, dlg.GetValue()]))

                # add item to the control
                self.channelTree.InsertItem(item, self.channelTree.GetLastChild(item), dlg.GetValue())

                # refresh my channel list
                self.channelList = commandHandler(parseArgs(['-l', 'channel']))

            dlg.Destroy()

        # add channel
        elif item == self.channelTree.GetRootItem():
            dlg = wx.TextEntryDialog(self, 'Please enter the Channel Name:')
            if dlg.ShowModal() == wx.ID_OK:
                commandHandler(parseArgs(['-a', dlg.GetValue()]))

                # append channel to the control
                self.channelTree.InsertItem(item, self.channelTree.GetLastChild(item), dlg.GetValue())

                # refresh my channel list
                self.channelList = commandHandler(parseArgs(['-l', 'channel']))

            dlg.Destroy()

    def OnEdit(self, event):
        item = self.channelTree.GetSelection()
        label = self.channelTree.GetItemText(item)

        # must be program
        dlg = wx.TextEntryDialog(self, 'Please enter the new name:')
        if dlg.ShowModal() == wx.ID_OK:
            channelItem = self.channelTree.GetItemParent(item) 
            channelName = self.channelTree.GetItemText(channelItem)

            commandHandler(parseArgs(['-e', channelName, label, dlg.GetValue()]))

            # refresh the control
            self.channelTree.SetItemText(item, dlg.GetValue())

        dlg.Destroy()

    def OnRemove(self, event):
        item = self.channelTree.GetSelection()
        label = self.channelTree.GetItemText(item)

        input = list()
        # A channel selected
        if label in self.channelList:
            msg = 'Sure to delete channel and all program within it?'
            dlg = wx.MessageDialog(self, msg, 'Warning!', wx.OK|wx.CANCEL|wx.ICON_WARNING)

            if dlg.ShowModal() == wx.ID_OK:
                if self.channelTree.ItemHasChildren(item):
                    cookie = 0
                    (child, cookie) = self.channelTree.GetFirstChild(item)

                    children = list()
                    while child:
                        children.append(self.channelTree.GetItemText(child)) 
                        (child, cookie) = self.channelTree.GetNextChild(item, cookie)

                    # perform deletion
                    commandHandler(parseArgs(['-r', label, ','.join(children)]))

                    # delete children for this channel
                    self.channelTree.DeleteChildren(item)
                # empty channel
                else:  
                    commandHandler(parseArgs(['-r', label]))

                    self.channelTree.Delete(item)

                # refresh my channel list
                self.channelList = commandHandler(parseArgs(['-l', 'channel']))

            dlg.Destroy()

        # selected the root item
        elif item == self.channelTree.GetRootItem():
            msg = 'Sure to delete ALL channels and items?'
            dlg = wx.MessageDialog(self, msg, 'Warning!', wx.OK|wx.CANCEL|wx.ICON_WARNING)
            if dlg.ShowModal() == wx.ID_OK:
                for channel in self.channelList:
                    commandHandler(parseArgs(['-r', channel]))

                # delete all channels from the control
                self.channelTree.DeleteChildren(self.channelTree.GetRootItem())

                # refresh my channel list
                self.channelList = commandHandler(parseArgs(['-l', 'channel']))

            dlg.Destroy()

        # selected an item
        else:
            msg = 'Sure to delete this program?'
            dlg = wx.MessageDialog(self, msg, 'Warning!', wx.OK|wx.CANCEL|wx.ICON_WARNING)
            if dlg.ShowModal() == wx.ID_OK:
                channelItem = self.channelTree.GetItemParent(item)
                channelName = self.channelTree.GetItemText(channelItem)

                commandHandler(parseArgs(['-r', channelName, label]))

                # delete from control
                self.channelTree.Delete(item)

                # refresh my channel list
                self.channelList = commandHandler(parseArgs(['-l', 'channel']))
            
            dlg.Destroy()

    def OnRefresh(self, event):
        self.buildChannelTree()

    def OnSelectChanged(self, event):
        item = event.GetItem()
        if not item: event.Skip()

        label = self.channelTree.GetItemText(item)
        
        if label in self.channelList or item == self.channelTree.GetRootItem():
            self.editBtn.Enable(False)
            self.addBtn.Enable(True)
        else: # item selected
            self.editBtn.Enable(True)
            self.addBtn.Enable(False)
    

class ContentPanel(wx.Panel):
    """ Panel for content manipulation """

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

        # controls
        searchBtn = wx.Button(self, wx.ID_ANY, 'Search')
        self.destroyBtn = wx.Button(self, wx.ID_ANY, 'Destroy')

        patternLabel = wx.StaticText(self, wx.ID_ANY, 'Pattern:')
        self.patternText = wx.TextCtrl(self, wx.ID_ANY, '')

        contentLabel = wx.StaticText(self, wx.ID_ANY, 'Contents:')
        self.contentBox = wx.ListCtrl(self, wx.ID_ANY, size=(100, 160), 
                            style=wx.BORDER_SUNKEN|wx.LC_REPORT|wx.LC_SORT_ASCENDING)
        self.contentBox.InsertColumn(0, 'ContentName')
        self.buildContentList()

        # pattern sizer
        patternSizer = wx.FlexGridSizer(cols=2, rows=2, vgap=1)
        patternSizer.AddGrowableCol(1)
        patternSizer.Add((0, 0), 0)
        patternSizer.Add(patternLabel, 0, 
                wx.TOP|wx.LEFT|wx.RIGHT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_LEFT, 10)
        patternSizer.Add(searchBtn, 0, wx.LEFT|wx.RIGHT, 10)
        patternSizer.Add(self.patternText, 0, 
                wx.EXPAND|wx.LEFT|wx.BOTTOM|wx.RIGHT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_LEFT, 10)

        # content sizer
        contentSizer = wx.FlexGridSizer(cols=2, rows=2, vgap=1)
        contentSizer.AddGrowableCol(1)
        contentSizer.Add((0, 0), 0)
        contentSizer.Add(contentLabel, 0, 
                wx.TOP|wx.LEFT|wx.RIGHT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_LEFT, 10)
        contentSizer.Add(self.destroyBtn, 0, wx.LEFT|wx.RIGHT, 10)
        contentSizer.Add(self.contentBox, 0, 
                wx.EXPAND|wx.LEFT|wx.BOTTOM|wx.RIGHT|wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_LEFT, 10)

        # main sizer
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(patternSizer, 0, wx.ALL|wx.EXPAND, 10)
        mainSizer.Add(contentSizer, 0, wx.ALL|wx.EXPAND, 10)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)
        mainSizer.SetSizeHints(self)

        # init state
        self.destroyBtn.Enable(False)

        # events
        self.Bind(wx.EVT_BUTTON, self.OnSearch, searchBtn) 
        self.Bind(wx.EVT_BUTTON, self.OnDestroy, self.destroyBtn) 
        
        self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnItemSelected, self.contentBox)

    def buildContentList(self, pattern='*'):
        self.contentBox.DeleteAllItems()

        contents = commandHandler(parseArgs(['-l', 'content', pattern]))
        
        iter = enumerate(contents)
        for idx, content in iter:
            self.contentBox.InsertStringItem(idx, content)

    def searchContent(self):
        if not self.patternText.IsEmpty():
            self.buildContentList(self.patternText.GetValue())
        else: self.buildContentList()

    def OnSearch(self, event):
        self.searchContent()
        self.destroyBtn.Enable(False)
        
    def OnDestroy(self, event):
        dlg = wx.MessageDialog(self, 'Are you sure to destroy the content?', 
                        'Confirm', wx.OK|wx.CANCEL|wx.ICON_WARNING)
        if dlg.ShowModal() == wx.ID_OK:
            items = list()

            idx = self.contentBox.GetFirstSelected()
            while idx != -1:
                items.append(self.contentBox.GetItemText(idx))
                idx = self.contentBox.GetNextSelected(idx)

            commandHandler(parseArgs(['-d', ','.join(items)]))

            # refresh the control
            self.searchContent()

            self.destroyBtn.Enable(False)

        dlg.Destroy()

    def OnItemSelected(self, event):
        if not self.destroyBtn.IsEnabled():
            self.destroyBtn.Enable(True)


class ConfigPanel(wx.Panel):
    """ test configuration """

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, wx.ID_ANY)

class LogWindow(wx.TextCtrl):
    """ redirects standard stream handler output to the text control """

    def __init__(self, parent):
        wx.TextCtrl.__init__(self, parent, wx.ID_ANY, '', 
                style=wx.TE_MULTILINE|wx.TE_READONLY|wx.HSCROLL)

    def write(self, message):
        self.AppendText(message)

    def flush(self): pass

class TianShanTestFrame(wx.Frame):
    """ Main frame of TianShanTest """

    def __init__(self):
        wx.Frame.__init__(self, None, wx.ID_ANY, "TianShanTest")

        # initialize
        try:
            loadConfig()
        except IOError, ex:
            dlg = wx.MessageDialog(self, 'Failed to load configuration!',
                    'Error occured!', wx.OK|wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()


        # splitter
        splitter = wx.SplitterWindow(
                self, wx.ID_ANY, style=wx.CLIP_CHILDREN|wx.SP_LIVE_UPDATE|wx.SP_3D)

        # text area that shows log info
        log = LogWindow(splitter)

        # upper pane
        self.nb = wx.Notebook(splitter, wx.ID_ANY, style=wx.CLIP_CHILDREN)

        self.provisionPanel = ProvisionPanel(self.nb)
        channelPanel = ChannelPanel(self.nb)
        contentPanel = ContentPanel(self.nb)
        configPanel = ConfigPanel(self.nb)
        self.nb.AddPage(self.provisionPanel, 'Provision')
        self.nb.AddPage(channelPanel, 'Channels')
        self.nb.AddPage(contentPanel, 'Contents')
        self.nb.AddPage(configPanel, 'Config')

        splitter.SplitHorizontally(self.nb, log, -150)

        # events
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)

        # logge 
        Logger.setup(log)

    def OnCloseWindow(self, event):
        shutdown(self.provisionPanel.threads, exit=True)
        self.Destroy()

class TianShanTestApp(wx.App):
    def OnInit(self):
        self.frame = TianShanTestFrame()
        self.frame.Show()
        
        return True

if __name__ == '__main__':
    app = TianShanTestApp(redirect=False)
    app.MainLoop()
