package dataqueue

import (
	"container/list"
	"errors"
	"fmt"
	"io"
	"se1/dataqueue/logger"
	"sync"
	"sync/atomic"
	"time"
)

var (
	errInvalidData       = errors.New("invalid data passed in")
	errInvalidBufId      = errors.New("invalid buffer id passed in")
	errInvalidBufferUser = errors.New("invalid buffer user")
	errTimedOut          = errors.New("operation timed out")
)

const (
	PageSizeShift = 12
	PageSize      = 1 << PageSizeShift
)

func RoundToPageBondary(size int) int {
	return (size + PageSize - 1) >> PageSizeShift << PageSizeShift
}

type CondValue struct {
	chSignal   chan bool
	expectSize int
}

type PoorCond struct {
	records map[int]CondValue
	locker  *sync.Mutex
	size    int
	bFilled bool
	lastId  int
}

func NewPoorCond(locker *sync.Mutex) *PoorCond {
	cond := &PoorCond{}
	cond.records = make(map[int]CondValue, 128)
	cond.locker = locker
	cond.Reset()
	return cond
}

func (cond *PoorCond) Reset() {
	cond.lastId = 10
	cond.size = 0
	cond.bFilled = false
}

// 调用这个函数之前一定要先lock住传入到PoorCond里的locker
func (cond *PoorCond) Wait(timeout time.Duration, expectSize int) error {
	if cond.bFilled || cond.size >= expectSize {
		return nil
	}
	cond.lastId++
	reqId := cond.lastId
	_, ok := cond.records[reqId]
	if ok {
		panic(fmt.Errorf("logic error, id[%d] has already been registered", reqId))
	}
	chSignal := make(chan bool, 2)
	cond.records[reqId] = CondValue{chSignal, expectSize}
	cond.locker.Unlock()
	//记得要先从map里面删除chSignal,然后再关闭chSignal
	defer close(chSignal)
	defer func() {
		cond.locker.Lock()
		delete(cond.records, reqId)
	}()

	timer := time.NewTimer(timeout)
	defer timer.Stop()
	for {
		select {
		case <-timer.C:
			{
				return errTimedOut
			}
		case <-chSignal:
			{
				return nil
			}
		}
	}

}

// 行为和CondVar的broadcast一直，只是当前这个需要判断expectSize与size
func (cond *PoorCond) Signal(size int, filled bool) {
	if cond.bFilled {
		panic("can not signal an already filled PoorCond")
	}
	cond.bFilled = filled
	cond.size = size
	records := cond.records
	cond.records = make(map[int]CondValue, 128)
	if filled {
		for _, v := range records {
			v.chSignal <- true
		}
	} else {
		for k, v := range records {
			if v.expectSize <= size {
				v.chSignal <- true
			} else {
				cond.records[k] = v
			}
		}
	}
}

type Buffer struct {
	buf        []byte
	n          *PoorCond
	dataSize   int32
	recyclable bool
	users      int32
	filled     bool
	locker     sync.Mutex
	id         int64
}

func NewBuffer(size int) *Buffer {
	size = RoundToPageBondary(size)
	b := &Buffer{}
	b.buf = make([]byte, size)
	b.n = NewPoorCond(&b.locker)
	return b
}

func (b *Buffer) Capacity() int {
	return cap(b.buf)
}

func (b *Buffer) Len() int {
	return int(b.dataSize)
}

func (b *Buffer) Recyclable() bool {
	return b.recyclable
}

func (b *Buffer) SetRecyclable() {
	b.recyclable = true
}

func (b *Buffer) SetDataSize(newSize int) {
	b.locker.Lock()
	defer b.locker.Unlock()
	atomic.StoreInt32(&b.dataSize, int32(newSize))
	b.n.Signal(newSize, false)
}

func (b *Buffer) setDataFilled(newSize int) {
	b.locker.Lock()
	defer b.locker.Unlock()
	atomic.StoreInt32(&b.dataSize, int32(newSize))
	b.filled = true
	b.n.Signal(newSize, true)
}

func (b *Buffer) IsFilled() bool {
	b.locker.Lock()
	defer b.locker.Unlock()
	return b.filled
}

func (b *Buffer) GetId() int64 {
	return b.id
}

func (b *Buffer) SetId(id int64) {
	b.id = id
}

func (b *Buffer) Reset() {
	if b.users != 0 {
		panic("CAN NOT reset an in-used Buffer")
	}
	b.n = NewPoorCond(&b.locker)
	b.dataSize = 0
	b.recyclable = false
	b.filled = false
	b.id = 0
}

func (b *Buffer) Users() int {
	return int(atomic.LoadInt32(&b.users))
}

func (b *Buffer) IncUser() int {
	return int(atomic.AddInt32(&b.users, 1))
}

func (b *Buffer) DecUser() int {
	return int(atomic.AddInt32(&b.users, -1))
}

func (b *Buffer) Wait(timeout time.Duration, expectSize int) error {
	return b.n.Wait(timeout, expectSize)
}

// 注意BufferUser的操作过程中
// write 针对buf进行操作
// read 针对data进行操作
type BufferUser struct {
	buf          *Buffer
	bRead        bool
	pool         *ResourcePool
	rangeStart   int
	rangeSize    int
	pos          int
	minWantBytes int
}

func NewBufferUser(buf *Buffer, bRead bool, pool *ResourcePool) *BufferUser {
	return NewRangedBufferUser(buf, bRead, pool, 0, buf.Capacity())
}

func NewRangedBufferUser(buf *Buffer, bRead bool, pool *ResourcePool, rangeStart, rangeSize int) *BufferUser {
	buf.IncUser() // add reference to Buffer
	return &BufferUser{buf, bRead, pool, rangeStart, rangeSize, 0, 10}
}

func (bu *BufferUser) Attach(buf *Buffer, bRead bool, pool *ResourcePool) {
	bu.AttachWithRange(buf, bRead, pool, 0, buf.Capacity())
}

func (bu *BufferUser) AttachWithRange(buf *Buffer, bRead bool, pool *ResourcePool, rangeStart, rangeSize int) {
	if bu.buf != nil {
		bu.Close()
	}
	buf.IncUser()
	bu.buf = buf
	bu.bRead = bRead
	bu.pool = pool
	bu.pos = 0
	bu.rangeStart = rangeStart
	bu.rangeSize = rangeSize
}

func (bu *BufferUser) Close() {
	buf := bu.buf
	bu.buf = nil
	if buf == nil {
		return
	}
	buf.DecUser()
	bu.pool.PutBU(bu)
}

func (bu *BufferUser) SetMinWantedBytes(size int) {
	if size < 1 {
		size = 1
	}
	bu.minWantBytes = size
}

func (bu *BufferUser) SpaceLeft() int {
	buf := bu.buf
	if buf == nil {
		return 0
	}
	left := bu.rangeSize - bu.pos
	if left < 0 {
		left = 0
	}
	return left
}

func (bu *BufferUser) DataLeft() int {
	if bu.buf == nil {
		return 0
	}
	left := bu.buf.Len() - bu.pos - bu.rangeStart
	if left < 0 {
		left = 0
	}
	return left
}

func (bu *BufferUser) Seek(pos int) int {
	if bu.buf == nil {
		return 0
	}
	if pos < 0 {
		pos = 0
	}
	bu.pos = pos
	return pos
}

func (bu *BufferUser) Advance(delta int) int {
	if bu.buf == nil {
		return 0
	}
	bu.pos += delta
	if bu.pos < 0 {
		bu.pos = 0
	}
	return bu.pos
}

func (bu *BufferUser) Buffer() []byte {
	if bu.buf == nil {
		return nil
	}
	start := bu.rangeSize + bu.pos
	end := bu.rangeStart + bu.rangeSize
	if bu.buf.Capacity() < end {
		end = bu.buf.Capacity()
	}
	return bu.buf.buf[start:end]
}

func (bu *BufferUser) Data() []byte {
	if bu.buf == nil {
		return nil
	}
	start := bu.rangeStart + bu.pos
	end := bu.rangeStart + bu.rangeSize
	if bu.buf.Len() < end {
		end = bu.buf.Len()
	}
	return bu.buf.buf[start:end]
}

func (bu *BufferUser) Wait(timeout time.Duration, expectSize int) error {
	if expectSize < 0 {
		panic("expect size must >= 0")
	}
	if bu.buf == nil {
		return errInvalidBufferUser
	}
	err := bu.buf.Wait(timeout, bu.rangeStart+bu.pos+expectSize)
	if bu.buf.IsFilled() {
		if bu.buf.Len() <= bu.rangeStart+bu.pos {
			return io.EOF
		}
	}
	return err
}

// ResourcePool 用于提供一个中央管理的buffer池
// 其存取动作暂时不考虑lockfree
type ResourcePool struct {
	bufs    *list.List
	buPool  sync.Pool
	bufSize int
	inUse   int
	total   int
	locker  sync.Mutex
}

func NewResourcePool(bufCount, bufSize int) *ResourcePool {
	rp := &ResourcePool{}

	if bufCount < 4 {
		bufCount = 4
	}
	rp.bufSize = RoundToPageBondary(bufSize)
	rp.inUse = 0
	rp.total = 0
	rp.bufs = list.New()
	rp.addBuffers(bufCount)

	rp.buPool.New = func() interface{} {
		return &BufferUser{}
	}
	return rp
}

func (rp *ResourcePool) Close() {
	if rp.bufs.Len() != rp.total {
		panic("there are some in-use buffer")
	}
	rp.bufs = nil
}

func (rp *ResourcePool) GetBU(buf *Buffer, bRead bool) *BufferUser {
	bu := rp.buPool.Get().(*BufferUser)
	bu.Attach(buf, bRead, rp)
	return bu
}

func (rp *ResourcePool) GetBUWithRange(buf *Buffer, bRead bool, rangeStart, rangeSize int) *BufferUser {
	bu := rp.buPool.Get().(*BufferUser)
	bu.AttachWithRange(buf, bRead, rp, rangeStart, rangeSize)
	return bu
}

func (rp *ResourcePool) PutBU(bu *BufferUser) {
	rp.buPool.Put(bu)
}

func (rp *ResourcePool) addBuffers(n int) {
	for i := 0; i < n; i++ {
		buf := NewBuffer(rp.bufSize)
		rp.bufs.PushBack(buf)
		rp.total++
	}
}

func (rp *ResourcePool) GetBuffer() *Buffer {
	rp.locker.Lock()
	defer rp.locker.Unlock()
	if rp.bufs.Len() == 0 {
		rp.addBuffers(16)
	}
	buf := rp.bufs.Remove(rp.bufs.Front()).(*Buffer)
	buf.Reset()
	return buf
}

func (rp *ResourcePool) GetBufferN(n int) []*Buffer {
	rp.locker.Lock()
	defer rp.locker.Unlock()
	if rp.bufs.Len() < n {
		delta := n - rp.bufs.Len()
		if delta < 16 {
			delta = 16
		}
		rp.addBuffers(delta)
	}
	bufs := make([]*Buffer, n)
	for i := 0; i < n; i++ {
		buf := rp.bufs.Remove(rp.bufs.Front()).(*Buffer)
		buf.Reset()
		bufs = append(bufs, buf)
	}
	return bufs
}

func (rp *ResourcePool) PutBuffer(b *Buffer) {
	rp.locker.Lock()
	defer rp.locker.Unlock()
	rp.bufs.PushBack(b)
}

func (rp *ResourcePool) PutBufferN(bufs []*Buffer) {
	rp.locker.Lock()
	rp.locker.Unlock()
	for _, b := range bufs {
		rp.bufs.PushBack(b)
	}
}

type queueElement struct {
	buf *Buffer
}

type DataQueue struct {
	// 为何DataQueue会有两个sync.Mutex
	// 而不是一个sync.RWMutex呢 ？
	// 因为DataQueue在一个时间点上只允许一个人写
	// 但是允许多个人读。 而且，即便是在写的时候
	// 也允许人读，只是读的部分不能是写的部分而已
	locker      sync.Mutex
	writeLocker sync.Mutex
	pool        *ResourcePool
	eles        []*Buffer
	idFirst     int64
	head        int
	tail        int
	users       int
	queueId     int
}

func NewDataQueue(pool *ResourcePool, queueSize int) *DataQueue {
	dq := &DataQueue{}
	dq.pool = pool
	dq.eles = make([]*Buffer, queueSize)
	copy(dq.eles, pool.GetBufferN(queueSize))
	dq.idFirst = 10
	dq.head = -1
	dq.tail = -1
	dq.users = 0
	return dq
}

func (dq *DataQueue) Write(prefix, data []byte) (int, error) {
	if data == nil {
		return 0, errInvalidData
	}
	size := len(data)
	if prefix != nil {
		size += len(prefix)
	}

	dq.writeLocker.Lock()
	defer dq.writeLocker.Unlock()

	var buf *Buffer
	if dq.head < 0 {
		dq.head = 0
	}
	buf = dq.eles[dq.head]
	if buf.Len()+size > buf.Capacity() {
		// 数据过大， 无法在当前的buffer中保存下来
		// 直接跳到下一个buffer
		buf = dq.advanceQueue()
	}
	pos := buf.Len()
	if prefix != nil {
		space := buf.buf[pos:]
		copy(space, prefix)
		pos += len(prefix)
	}
	space := buf.buf[pos:]
	copy(space, data)
	newSize := buf.Len() + size
	if newSize == buf.Capacity() {
		buf.setDataFilled(newSize)
	} else {
		buf.SetDataSize(buf.Len() + size)
	}
	return size, nil
}

// 变换当前的Queue.head到下一个buffer位置
// 如果下一个buffer被占用，那么申请一块新的buffer。
// 同时标识被占用的buffer为可回收的。
func (dq *DataQueue) removeBufferAt(idx int) *Buffer {
	if idx >= len(dq.eles) || idx < 0 {
		panic("index out of bounds")
	}
	buf := dq.eles[idx]
	dq.eles = append(dq.eles[0:idx], dq.eles[idx+1:]...)
	return buf
}

func (dq *DataQueue) insertBufferAt(idx int) {
	// 获取一个新的Buffer
	newBuf := dq.pool.GetBuffer()
	if cap(dq.eles) < len(dq.eles)+1 {
		dq.eles = append(dq.eles, nil) //enlarge eles
	} else {
		dq.eles = dq.eles[:len(dq.eles)+1]
	}
	copy(dq.eles[idx:], dq.eles[idx+1:])
	dq.eles[idx] = newBuf
}

func (dq *DataQueue) advanceQueue() *Buffer {
	dq.locker.Lock()
	defer dq.locker.Unlock()
	for {
		nextIdx := (dq.head + 1) % (len(dq.eles))
		nextBuf := dq.eles[nextIdx]
		if nextBuf.Users() != 0 {
			// 当前这个Buffer正在被读取，不能使用
			nextBuf.SetRecyclable()
			dq.insertBufferAt(nextIdx)
			dq.head = nextIdx
			if dq.tail >= 0 && nextIdx == dq.tail {
				dq.tail = (dq.tail + 1) % len(dq.eles)
			}
			break
		} else if nextBuf.Recyclable() {
			buf := dq.removeBufferAt(nextIdx)
			dq.pool.PutBuffer(buf)
			if dq.tail >= 0 && nextIdx == dq.tail {
				dq.tail = (dq.tail) % len(dq.eles)
			}
			continue
		} else {
			dq.head = nextIdx
			if nextIdx == dq.tail && dq.tail > 0 {
				dq.tail = (dq.tail + 1) % len(dq.eles)
			}
			break
		}
	}
	dq.idFirst++
	return dq.eles[dq.head]
}

func (dq *DataQueue) Read(id int64) (*BufferUser, error) {
	dq.locker.Lock()
	dq.locker.Unlock()
	if dq.head < 0 {
		dq.head = 0
		dq.tail = 0
		buf := dq.eles[dq.head]
		buf.SetId(dq.idFirst)
		return dq.pool.GetBU(buf, true), nil
	} else {
		if dq.tail < 0 {
			dq.tail = dq.head //直接读取最新的数据，貌似说得通 ^_^
		}
		availEles := int64((dq.head + len(dq.eles) - dq.tail) % len(dq.eles))
		leftEdge := dq.idFirst - availEles + 1
		rightEdge := dq.idFirst + 1
		if id > rightEdge {
			return nil, errInvalidBufId
		} else if id < leftEdge {
			logger.InfoF("DataQueue[%04d] read, id[%d] head[%d/%d] tail[%d], adjust id to idFirst", dq.queueId, id, dq.head, dq.tail)
			id = dq.idFirst
		}
		if id != rightEdge {
			idx := (dq.head + int(dq.idFirst-id)) % len(dq.eles)
			logger.InfoF("DataQueue[%04d] read, id[%d] head[%d/%d] tail[%d], idx[%d]", dq.queueId, id, dq.head, dq.tail, idx)
			buf := dq.eles[idx]
			buf.SetId(dq.idFirst)
			return dq.pool.GetBU(buf, true), nil
		} else {
			// 想要读取位于Head的下一个buf
			for {
				nextIdx := (dq.head + 1) % len(dq.eles)
				nextBuf := dq.eles[nextIdx]
				if nextBuf.Users() != 0 {
					if nextIdx != dq.tail {

						logger.InfoF("DataQueue[%04d] read, probeNext id[%d] head[%d/%d] tail[%d], idx[%d]", dq.queueId, id, dq.head, dq.tail, nextIdx)
						break //当前的buf已经被用作head的下一个buf
					}
					nextBuf.SetRecyclable()
					dq.insertBufferAt(nextIdx)
					if dq.tail >= 0 {
						dq.tail = (dq.tail + 1) % len(dq.eles)
					}

					logger.InfoF("DataQueue[%04d] read, probeNext id[%d] head[%d/%d] tail[%d], add newBuffer at idx[%d]", dq.queueId, id, dq.head, dq.tail, nextIdx)
					break
				} else if nextBuf.Recyclable() {
					buf := dq.removeBufferAt(nextIdx)
					dq.pool.PutBuffer(buf)
					if dq.tail >= 0 && nextIdx == dq.tail {
						dq.tail = (dq.tail) % len(dq.eles)
					}

					logger.InfoF("DataQueue[%04d] read, probeNext id[%d] head[%d/%d] tail[%d], remove Buffer at idx[%d]", dq.queueId, id, dq.head, dq.tail, nextIdx)
					continue
				} else {
					if nextIdx == dq.tail && dq.tail > 0 {
						dq.tail = (dq.tail + 1) % len(dq.eles)
					}
					logger.InfoF("DataQueue[%04d] read, probeNext id[%d] head[%d/%d] tail[%d], idx[%d]", dq.queueId, id, dq.head, dq.tail, nextIdx)
					break
				}
			}
			nextIdx := (dq.head + 1) % len(dq.eles)
			buf := dq.eles[nextIdx]
			buf.SetId(dq.idFirst + 1)
			return dq.pool.GetBU(buf, true), nil
		}
	}
}

type QueueUser struct {
	dq      *DataQueue
	id      int64
	bReader bool
}

func NewQueueUser(dq *DataQueue, bReader bool) *QueueUser {
	qu := &QueueUser{}
	qu.bReader = bReader
	qu.dq = dq
	return qu
}
