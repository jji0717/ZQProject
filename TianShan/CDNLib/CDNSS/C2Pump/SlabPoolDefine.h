#ifndef SLAB_POOL_DEFINE_H
#define SLAB_POOL_DEFINE_H

#include "SlabPool.h"

#ifdef ENABLE_SLABPOOL
extern SlabPool gSlabPool;
    #define SLABPOOL_DEFINE(TYPE) \
        void* operator new(size_t size) throw(std::bad_alloc)  \
        { \
            return sh.getSlab(); \
        } \
        void operator delete(void* p) \
        { \
            return sh.freeSlab(p); \
        } \
        static SlotHolder sh;
    
    #define SLOTHOLDER_INIT(TYPE) SlotHolder TYPE::sh(gSlabPool, sizeof(TYPE));
#else
    #define SLABPOOL_DEFINE(TYPE)
    #define SLOTHOLDER_INIT(TYPE)
#endif

#endif //SLAB_POOL_DEFINE_H
