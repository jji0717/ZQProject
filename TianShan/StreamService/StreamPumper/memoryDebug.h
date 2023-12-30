
#ifdef _DEBUG_MEM_LEAK_
	#ifndef _AHEI_DEBUG_MEMORY_H__
	#define _AHEI_DEBUG_MEMORY_H__

		#if defined _DEBUG || defined DEBUG

				#undef new
				extern void _RegDebugNew( void );
				extern void* __cdecl operator new( size_t, const char*, int );
				extern void __cdecl operator delete( void*, const char*, int);
				#define new new(__FILE__, __LINE__)		
				#define REG_DEBUG_NEW _RegDebugNew();

		#else

				#define REG_DEBUG_NEW
		#endif//_DEBUG || defined DEBUG
	#endif//_AHEI_DEBUG_MEMORY_H__
	
#else//_DEBUG_MEM_LEAK_

	#define REG_DEBUG_NEW

#endif//_DEBUG_MEM_LEAK_

