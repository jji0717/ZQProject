

#ifndef _IMEMORY_ALLOCATOR_
#define _IMEMORY_ALLOCATOR_


class IMemAlloc
{
public:
	virtual ~IMemAlloc(){}
	/// allocate a memory block from the buffer pool of the engine
	///@param[in] the size in byte about to allocate
	///@return pointer to the newly allocated buffer, NULL if the allocation failed
	virtual void* alloc(size_t size) =0;
	
	/// free a memory block to the buffer pool of the engine
	///@param[in] pointer to the memory block allocated via alloc()
	///@sa alloc()
	virtual void free(void* ptr) =0;
};


#endif
