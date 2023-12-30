#include "SlabPool.h"
#include <TimeUtil.h>

Slot::Slot(size_t size, size_t align /* = 8 */, std::string name /* = "" */)
: _align(align), _size(size), _name(name), _slabs(NULL), _chunks(NULL),
_totalCount(0), _usedCount(0), _availableCount(0), _increment(4)
{
}

Slot::~Slot()
{
    Chunk* r = _chunks;
    while(r)
    {
        Chunk* tmp = r;
        r = r->next;
        delete tmp;
    }
}

void* Slot::chunkAlloc(size_t size)
{
    boost::recursive_mutex::scoped_lock splk(_mutex);

    char* r = (char*)malloc(size);
    if (!r)
        return NULL; // out of memory
    // add chunk to chunk list
    Chunk* ck = (Chunk*)malloc(sizeof(Chunk));
    ck->alloc = r;
    ck->next = _chunks;
    _chunks = ck;
    return r;
}

bool Slot::refill(size_t n)
{
    boost::recursive_mutex::scoped_lock splk(_mutex);

    size_t allocSize = _size * n;
    char* start = (char*)chunkAlloc(allocSize);
    if (!start)
        return false;  // out of memory

    for (size_t i = 0; i < n; i++)
    {
        Slab* tmp = (Slab*)start;
        addSlab(tmp);
        start += _size;
    }
    return true;
}

void Slot::addSlab(Slab* slab)
{
    boost::recursive_mutex::scoped_lock splk(_mutex);

    if (!_slabs)
    {
        slab->next = NULL;
        _slabs = slab;
    }else{
        slab->next = _slabs->next;
        _slabs = slab;
    }
    _totalCount++;
    _availableCount++;
}

void Slot::free(void* p)
{
    boost::recursive_mutex::scoped_lock splk(_mutex);

    Slab* tmp = (Slab*)p;
    tmp->next = _slabs;
    _slabs = tmp;
}

Slab* Slot::getSlab()
{
    boost::recursive_mutex::scoped_lock splk(_mutex);

    if (_slabs)
    {
        Slab* r = _slabs;
        _slabs = _slabs->next;

        _availableCount--;
        _usedCount++;
        return r;
    }else{
        size_t refillNum = 2 * _totalCount + _increment;
        _increment <<= 1;
        if (!refill(refillNum))
        {
            return NULL;
        }
        return getSlab();
    }
}

SlotHolder::SlotHolder(SlabPool& pool, size_t size)
: _pool(pool), _size(size), _slotPtr(NULL)
{
    init();
}

SlotHolder::~SlotHolder()
{

}

void SlotHolder::init()
{
    _slotPtr = _pool.getSlot(_size);
}

void* SlotHolder::getSlab()
{
    return _slotPtr->getSlab();
}

void SlotHolder::freeSlab(void* p)
{
    return _slotPtr->free(p);
}

SlabPool::SlabPool(bool memStartAlign /* = false */, size_t align /* = 8 */)
: _bMemStartAlign(memStartAlign), _align(align)
{
    //_slots.reserve(100000);
}

SlabPool::~SlabPool()
{
    _slots.clear();
}

Slot::Ptr SlabPool::getSlot(size_t size, std::string name)
{
    boost::recursive_mutex::scoped_lock splk(_mutex);

    size_t slotSize = roundUp(size);
    Slots::iterator it = _slots.find(slotSize);

    if (it != _slots.end())
    {
        return it->second;
    }else{
        Slot::Ptr slotPtr = new Slot(size, _align, name);
        _slots.insert(Slots::value_type(std::make_pair(slotSize, slotPtr)));
        return slotPtr;
    }
}
