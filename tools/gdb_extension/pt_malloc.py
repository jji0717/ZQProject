# -*- encoding:utf-8 -*-
# malloc.py is an extension for gdb to inspect memory allocation and free
#
import sys
import struct
from contextlib import contextmanager
import os.path

#in pthon 2.6, this is only what I can get
from optparse import OptionParser

import gdb


#redefine long as int in higher python version
if sys.version_info[0] == 2:
    if sys.version_info[1] >= 7:
        long = int
elif sys.version_info[0] == 3:
    long = int

MIN_CHUNKSIZE = 32
ALIGNMENT = 16
ALIGNMENT_MASK = ALIGNMENT - 1
HEAPSIZE = 64 * 1024 * 1024
HEAP_MASK = HEAPSIZE - 1

symbol = None

class SymbolInitializer(object):
    '''
    load symbol and find basic type for analyse
    '''
    def __init__(self):
        self._symbol_loaded = False

    @property
    def symbolLoaded(self):
        return self._symbol_loaded

    def loadSymbol(self):
        pass

class Pt2mallocInitializer(SymbolInitializer):
    '''
    pt2malloc(used in glibc) symbol initializer
    '''
    type_mchunkptr = None
    type_long = None
    type_heapinfo = None
    type_mstate = None
    mstate_size = 0

    def __init__(self):
        super(Pt2mallocInitializer, self).__init__()

    def cast2Mstate(self, val):
        '''
        cast type of val to mstate -- that is struct malloc_state*
        '''
        return val.cast(self.type_mstate)
    
    def cast2Long(self, v):
        '''
        cast type of v to long, which can also be used as pointer address
        '''
        return v.cast(self.type_long)
    
    def cast2HeapInfo(self, v):
        '''
        cast type of v to heapinfo* -- that is struct _heap_info*
        '''
        return v.cast(self.type_heapinfo)
    
    def cast2Pt2ChunkPtr(self, v):
        '''
        cast type of v to mchunkptr -- that is struct malloc_chunk*, this type is also identical
        to mfastbinptr
        '''
        return v.cast(self.type_mchunkptr)

    @property
    def mstateSize(self):
        return self.mstate_size

    def loadSymbol(self):
        '''
        load libc's symbol and lookup some useful type
        '''
        if self._symbol_loaded:
            return True
        objs = gdb.objfiles()
        bFound = False
        for obj in objs:
            name = os.path.basename(obj.filename)
            if name.startswith("libc") and name.endswith(".debug"):
                bFound = True
        if not bFound:
            print("debug symbol of libc is not found")
            return False
        self.type_mchunkptr = gdb.lookup_type("mchunkptr")
        self.type_long = gdb.lookup_type("long")
        self.type_heapinfo = gdb.lookup_type("struct _heap_info").pointer()
        self.type_mstate = gdb.lookup_type("struct malloc_state").pointer()
        self.mstate_size = gdb.parse_and_eval("sizeof(struct malloc_state)").cast(self.type_long)
        self._symbol_loaded = True
        global symbol
        symbol = self
        return True


class Chunk(object):
    
    @property
    def address(self):
        assert False
        return 0

    @property
    def userPtr(self):
        assert False
        return 0
    
    @property
    def chunkPtr(self):
        assert False
        return 0


class Pt2Chunk(Chunk):
    def __init__(self, chunkAddress):
        self.chunk = symbol.cast2Pt2ChunkPtr(chunkAddress).dereference()

    def __cmp__(self, rhs):
        '''
        compare two chunk according to its address
        '''
        if self.chunk.address < rhs.chunk.address:
            return -1
        elif self.chunk.address == rhs.chunk.address:
            return 0
        else:
            return 1

    @property
    def address(self):
        return symbol.cast2Long(self.chunk.address)

    @property
    def userPtr(self):
        #hard code for x64 size_t which should be 8 bytes
        return int(symbol.cast2Long(self.chunk.address) + 2 * 8)

    @property
    def chunkPtr(self):
        return long(symbol.cast2Long(self.chunk.address))

    @property
    def size(self):
        return long(self.chunk["size"]) & (~7)
    
    @property
    def prevSize(self):
        return long(self.chunk["prev_size"])
    
    @property
    def prevInUse(self):
        return self.chunk["size"] & 0x01 > 0
    
    @property
    def isMapped(self):
        return self.chunk["size"] & 0x02 > 0
    
    @property
    def isInMainHeap(self):
        return self.chunk["size"] & 0x04 == 0
    
    @property
    def next(self):
        '''
        next return Pt2Chunk instance at current address + current size
        '''
        return Pt2Chunk(self.address + self.size)

    @property
    def fwd(self):
        return Pt2Chunk(self.chunk["fd"].address)

    @property
    def bck(self):
        return Pt2Chunk(self.chunk["bk"].address)
    
    @property
    def fwd_nextsize(self):
        return Pt2Chunk(self.chunk["fd_nextsize"].address)
    
    @property
    def bck_nextsize(self):
        return Pt2Chunk(self.chunk["bk_nextsize"].address)
    
    def chunkAtOffset(self, offset):
        nextAddr = symbol.cast2Long(self.chunk.address) + offset
        return Pt2Chunk(gdb.Value(nextAddr))

__HEAP_MAXSIZE = 64 * 1024 * 1024
__HEAP_MASK = __HEAP_MAXSIZE - 1

class Arena(object):
    def __init__(self):
        self._allSize = 0
        self._allocated = []
        self._allocatedSize = 0
        self._freed = []
        self._freedSize = 0
        self._scanned = False
        self._isMainArena = False

    @property
    def address(self):
        assert False
        return 0

    @property
    def size(self):
        assert False
        return 0

    @property
    def allocatedSize(self):
        if not self._scanned:
            self.scanArena()
        return self._allocatedSize

    @property
    def freedSize(self):
       if not self._scanned:
           self.scanArena()
       return self._freedSize

    @property
    def next(self):
        assert False
        return None
    
    def getInusedChunk(self):
        if not self._scanned:
            self.scanArena()
        return self._allocated
    
    def getFreedChunk(self):
        if not self._scanned:
            self.scanArena()
        return self._freed

    @property
    def isMainArena(self):
        return self._isMainArena

    @staticmethod
    def getMainArena():
        assert False
        pass


class PtArena(Arena):
    def __init__(self, arena, isMain = False):
        super(PtArena, self).__init__()
        self.arena = arena
        self._isMainArena = isMain

    def __cmp__(self, rhs):
        if self.address < rhs.address:
            return -1
        elif self.address == rhs.address:
            return 0
        else:
            return 1

    @property
    def address(self):
        return symbol.cast2Long(self.arena.address)

    
    @property
    def size(self):
        '''
        arena size, include those non-contiguos zone
        '''
        return symbol.cast2Long(self.arena["system_mem"])

    @property
    def contiguous(self):
        return self.arena["flags"] & 0x02 == 0

    def sbrkBaseAsChunk(self):
        assert self._isMainArena
        sbrk = gdb.parse_and_eval("mp_")
        return symbol.cast2Pt2ChunkPtr(sbrk["sbrk_base"])

    def topAsChunk(self):
        '''
        return top as chunkPtr
        '''
        return symbol.cast2Pt2ChunkPtr(self.arena["top"])

    @property
    def next(self):
        '''
        next arena
        '''
        return PtArena(self.arena["next"].dereference())

    @property
    def fastbins(self):
        try:
            fb = self.arena["fastbinsY"]
        except ValueError:
            fb = self.arena["fastbins"]#in centos 5, this var is named as fastbins
        return fb

    @property
    def normalbins(self):
        return self.arena["bins"]

    @staticmethod
    def getMainArena():
        return PtArena(gdb.parse_and_eval("main_arena"), True)

    @staticmethod
    def getHeapArea(addr):
        hp = symbol.cast2HeapInfo(gdb.Value(addr))
        begin = addr + gdb.parse_and_eval("sizeof(struct _heap_info)")
        end = addr + hp["size"]
        if symbol.cast2Long(hp["prev"]) == 0:
            #main heap for current arena
            begin = begin + symbol.mstateSize
            misalign = (begin + 2 * 8) & ALIGNMENT_MASK 
            if misalign > 0:
                begin += (ALIGNMENT - misalign)
        return hp, symbol.cast2Pt2ChunkPtr(begin), symbol.cast2Pt2ChunkPtr(end)

    def scanArena(self):
        if self._scanned:
            return
        if self._isMainArena:
            self._scanMainArena()
        else:
            self._scanNonMainArena()
        self._freed.sort()
        self._allocated.sort()
        self._scanned = True
        alloced = []
        lenAll = len(self._allocated)
        lenFree = len(self._freed)
        iAll = 0
        iFree = 0
        while (iAll < lenAll) and (iFree < lenFree):
            if self._allocated[iAll] < self._freed[iFree]:
                alloced.append(self._allocated[iAll])
                self._allocatedSize += self._allocated[iAll].size
                iAll = iAll + 1
            elif self._allocated[iAll] == self._freed[iFree]:
                iAll = iAll + 1
                iFree = iFree + 1
            else:
                iFree = iFree + 1
        while iAll < lenAll:
            alloced.append(self._allocated[iAll])
            self._allocatedSize += self._allocated[iAll].size
            iAll = iAll + 1
        self._allocated = alloced
        self._scanned = True

    def _scanMainArena(self):
        if(not self.contiguous):
            print("main arena has non-contiguous memory layout, chunks statictics may not accurate")
        self._scanFreedChunks()
        top = self.topAsChunk()
        bottom = self.sbrkBaseAsChunk()
        self._scanAllChunks(bottom, top)

    def _scanNonMainArena(self):
        self._scanFreedChunks()
        hp = self.topAsChunk()
        firstHeap = True
        while hp != None and symbol.cast2Long(hp) != 0:
            # scan all heaps
            hpInfo, begin, end = PtArena.getHeapArea(symbol.cast2Long(hp)&(~HEAP_MASK))
            if firstHeap:
                end = hp
                firstHeap = False
            self._scanAllChunks(begin, end)
            hp = hpInfo["prev"]
    
    def _scanAllChunks(self, start, end):
        ckEnd = Pt2Chunk(end)
        ck = Pt2Chunk(start)
        if ck.size < MIN_CHUNKSIZE:
            return
        while ck < ckEnd:
            self._allocated.append(ck)
            ck = ck.next
            if ck >= ckEnd:
                break
            if ck.size < MIN_CHUNKSIZE:
                break

    def _scanFreedChunks(self):
        fastbins = self.fastbins
        for i in range(0, 10):
            ck = fastbins[i]
            if symbol.cast2Long(ck) == 0:
                continue
            while symbol.cast2Long(ck) != 0:
                chk = Pt2Chunk(ck)
                self._freedSize += chk.size
                self._freed.append(chk)
                ck = ck["fd"]
        normalbins = self.normalbins
        for i in range(0, 254, 2):
            #head should be at offset of bin[x] - 2* sizeof(SIZE_T)
            head = symbol.cast2Long(normalbins[i].address) - 2 * 8
            bk = normalbins[i + 1]
            while symbol.cast2Long(bk) != head:
                chk = Pt2Chunk(bk)
                self._freedSize += chk.size
                self._freed.append(chk)
                bk = bk["bk"]


def ptr2symbol(addr):
    proc = gdb.selected_inferior()
    if not proc:
        return ""
    vtblData = proc.read_memory(addr, 8)#x64
    vtbl = struct.unpack("@q", vtblData)
    if (vtbl is None) or (vtbl == 0):
        return ""
    cmd = "info symbol 0x%x" % (vtbl)
    try:
        symbol = gdb.execute(cmd, False, True)
    except gdb.error:
        return "" #is this good ?
    if symbol.startswith("No symbol matches"):
        return ""
    if symbol.endswith("\n"):
        return symbol[:-1]
    return symbol


class MemCenter(object):
    def __init__(self):
        self._arenas = []
        self.symbolLoader = None
    @property
    def arenas(self):
        if len(self._arenas) == 0:
            print("memory chunk parsing, it may take very long time (depend on how many chunks your process owned)")
            self.collect()
        return self._arenas

    def findArena(self, addr):
        ar = None
        for arena in self.arenas:
            if addr == arena.address:
                ar = arena
                break
        return ar

    def loadSymbol(self, method="ptmalloc"):
        if self.symbolLoader == None:
            if method == "ptmalloc":
                self.symbolLoader = Pt2mallocInitializer()
            else:
                print("not recognized malloc method[%s]" % (method))
                return False
        if self.symbolLoader.symbolLoaded:
            return True
        if not self.symbolLoader.symbolLoaded:
            self.symbolLoader.loadSymbol()
        if (self.symbolLoader is None) or (not self.symbolLoader.symbolLoaded):
            print("failed to load symbol for method [%s]" % (method))
            return False
        return True

    def collect(self):
        if not self.loadSymbol():
            print("debug symbol is not loaded")
            return
        mainArena = PtArena.getMainArena()
        if not mainArena:
            print("no main arena is found")
            return
        self._arenas.append(mainArena)
        arena = mainArena.next
        while arena != mainArena:
            self._arenas.append(arena)
            arena = arena.next

mc = MemCenter()
_heapUsage = '''
'''

@contextmanager
def turnoffPagination():
    gdb.execute("set pagination off")
    yield
    gdb.execute("set pagination on")

def readNumber(n):
    if n is None:
        return 0
    n = n.strip().upper()
    trail2factor = {"KI": 1024,
            "MI": 1024 * 1024,
            "GI": 1024 * 1024 * 1024,
            "TI": 1024 * 1024 * 1024,
            "K": 1000,
            "M": 1000 * 1000,
            "G": 1000 * 1000 * 1000,
            "T": 1000 * 1000 * 1000 }
    for k in trail2factor.keys():
        if not n.endswith(k):
            continue
        n = n[0:-len(k)]
        base = float(n)
        return long(base * trail2factor[k])
    return long(n, base=0)


class HeapCommand(gdb.Command):
    '''
    heap command set is used to diagnose system memroy manager
    '''
    def __init__(self):
        super(HeapCommand, self).__init__("heap", gdb.COMMAND_USER, gdb.COMPLETE_NONE, True)

    def invoke(self, argument, from_tty):
        if not from_tty:
            return
        self.usage()

    def usage(self):
        print(_heapUsage)


class HeapShowCommand(gdb.Command):
    '''
    heap show display heap's attribute in this process
    you can use heap stat -h to get help information
    '''
    def __init__(self):
        super(HeapShowCommand, self).__init__("heap show", gdb.COMMAND_USER, gdb.COMPLETE_NONE)
        self.dont_repeat()

    def invoke(self, argument, from_tty):
        parser = OptionParser()
        parser.add_option("-f", "--full", dest="fulldetail", help="print detail heap statistics", action="store_true", default=False)
        parser.add_option("--min", "--min", dest="minsize", help="filter out heap by minimum size")
        parser.add_option("--max", "--max", dest="maxsize", help="filter out heap by maximum size")
        parser.add_option("-s", "--sortby", dest="sortby", help="sort heap by column you specified" )
        parser.add_option("-r", "--reverse", dest="reversesort", help="reverse sort", action="store_true", default=False)
        (options, args) = parser.parse_args(gdb.string_to_argv(argument))
        heapAddr = 0
        if len(args) > 0:
            heapAddr = readNumber(args[0])
        minsize = readNumber(options.minsize)
        maxsize = readNumber(options.maxsize)
        try:
            self.showHeaps(heapAddr, minsize, maxsize, options.fulldetail, options.sortby, options.reversesort )
        except KeyboardInterrupt:
            pass

    def showHeaps(self, heapAddr, minSize, maxSize, fulldetail, sortedby, reverse):
        if maxSize <= 0:
            maxSize = 1024 * 1024 * 1024 * 1024 
        print("processing with heap %x min %x max %x" % (heapAddr, minSize, maxSize))
        heaps = []
        sortfunc = None
        if (sortedby is not None) and  len(sortedby) > 0:
            if sortedby == "allocated_size":
                fulldetail = True
                sortfunc = lambda x: x.allocatedSize
            elif sortedby == "heap_size":
                sortfunc = lambda x: x.size
            elif sortedby == "freed_size":
                fulldetail = True
                sortfunc = lambda x: x.freedSize
            else:
                print("wrong")
                sortfunc = lambda x: x

        with turnoffPagination():
            for heap in mc.arenas:
                if heapAddr != 0:
                    if heapAddr != heap.address:
                        continue
                size = heap.size
                if size < minSize or size > maxSize:
                    continue
                heaps.append(heap)

            if sortfunc is not None:
                heaps.sort(key=sortfunc, reverse=reverse)

            for heap in heaps:
                if heap.isMainArena and not heap.contiguous:
                    sys.stdout.write("*")
                if fulldetail:
                    print("heap[0x%x]\t size[% 16d] allocated[% 10d] freed[% 10d]" % (heap.address, heap.size, heap.allocatedSize, heap.freedSize))
                else:
                    print("heap[0x%x]\t size[% 16d] " % (heap.address, heap.size))


class HeapFltCommand(gdb.Command):
    def __init__(self):
        super(HeapFltCommand, self).__init__("heap flt", gdb.COMMAND_USER, gdb.COMPLETE_NONE)
        self.dont_repeat()
    
    def invoke(self, argument, from_tty):
        parser = OptionParser()
        parser.add_option("-f", "--full", dest="fulldetail", help="print detail chunk information including symbol if present", action="store_true", default=False)
        parser.add_option("-m", "--memoryheap", dest="heap", help="specify the memory heap you want to take action on")
        parser.add_option("-n", "--freed", dest="freed", help="search on freed chunks", action="store_true", default=False )
        (options, args) = parser.parse_args(gdb.string_to_argv(argument))
        if len(args) == 0:
            print("\t Please specify a chunk size")
            return
        sizeMin = readNumber(args[0])
        if len(args)>1:
            sizeMax = readNumber(args[1])
        else:
            sizeMax = sizeMin
        heapAddr = readNumber(options.heap)
        try:
            self.filt(heapAddr, sizeMin, sizeMax, options.fulldetail, options.freed)
        except KeyboardInterrupt:
            pass


    def filt(self, heapAddr, sizeMin, sizeMax, full, searchFreed):
        if sizeMin > sizeMax:
            sizeMin, sizeMax = sizeMax, sizeMin
        if heapAddr != 0:
            heap = mc.findArena(heapAddr)
            if heap is None:
                print("heap [0x%x] is not found" % (heapAddr))
                return
            self.filterChunk(heap, sizeMin, sizeMax, full, searchFreed)
        else:
            for heap in mc.arenas:
                self.filterChunk(heap, sizeMin, sizeMax, full, searchFreed)

    def filterChunk(self, heap, sizeMin, sizeMax, full, searchFreed):
        chunks = heap.getFreedChunk() if searchFreed else heap.getInusedChunk()
        for chk in chunks:
            size = chk.size
            if sizeMin <= size <= sizeMax:
                if full:
                    print("Chunk[0x%x] size[%d] UserPtr[0x%x] %s" % (chk.address, size, chk.userPtr, ptr2symbol(chk.userPtr)))
                else:
                    print("Chunk[0x%x] size[%d] UserPtr[0x%x]" % (chk.address, size, chk.userPtr))



heapCmd = HeapCommand()
heapStatCmd = HeapShowCommand()
heapFltCmd = HeapFltCommand()

# vim: ts=4 sw=4 sts=4 noet
