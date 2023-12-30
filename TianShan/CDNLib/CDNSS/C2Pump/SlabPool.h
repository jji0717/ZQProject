#ifndef ZQ_SLAB_POOL_H
#define ZQ_SLAB_POOL_H
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <list>
#include <string>
#include <Pointer.h>

union Slab
{
    union Slab* next;
    //char data[1];
    char* data;
};

struct Chunk
{
    struct Chunk* next;
    void*   alloc;
};

class Slot : public virtual ZQ::common::SharedObject
{
public:
    typedef ZQ::common::Pointer<Slot> Ptr;
    Slot(size_t size, size_t align = 8, std::string name = "");
    virtual ~Slot();

    size_t  size() {return _size;}
    size_t  totalCount() {return _totalCount;}
    size_t  usedCount() {return _usedCount;}
    size_t  availableCount() {return _availableCount;}

    Slab*   getSlab();
    void    free(void* p);

private:
    size_t roundUp(size_t size) {
        return (((size) + _align) & ~(_align - 1));
    }

    bool refill(size_t n);
    void* chunkAlloc(size_t size);
    void addSlab(Slab* slab);

private:
    size_t  _align;
    size_t  _size;
    std::string _name;
    Slab*   _slabs;
    Chunk*  _chunks;
    size_t  _totalCount;
    size_t  _usedCount;
    size_t  _availableCount;
    size_t  _increment;
    boost::recursive_mutex _mutex;
};
typedef boost::unordered_map<size_t, Slot::Ptr> Slots;

class SlabPool;
class SlotHolder : public virtual ZQ::common::SharedObject
{
public:
    typedef ZQ::common::Pointer<SlotHolder> Ptr;
    SlotHolder(SlabPool& pool, size_t size);
    virtual ~SlotHolder();

    void* getSlab();
    void freeSlab(void* p);
private:
    void init();

private:
    SlabPool&   _pool;
    size_t      _size;
    Slot::Ptr   _slotPtr;
};

class SlabPool : public virtual ZQ::common::SharedObject
{
public:
    typedef ZQ::common::Pointer<SlabPool> Ptr;
    SlabPool(bool memStartAlign = false, size_t align = 8);
    virtual ~SlabPool();

    Slot::Ptr getSlot(size_t size, std::string name = "");

private:
    size_t roundUp(size_t size) {
        return (((size) + _align) & ~(_align - 1));
    }

private:
    bool    _bMemStartAlign;
    size_t  _align;

    Slots   _slots;
    boost::recursive_mutex _mutex;
};
#endif
