// include file for kernel-mode library main routine

#ifndef _RTF_MAIN_H
#define _RTF_MAIN_H 1

// conditional compilation switches

#define DEBUG						1
#define TEST_FUNC					1

// configuration constants

#define SES_FAIL_THRESHOLD		1
#define MAX_SESSION_COUNT		8
#define MAX_INPUT_BUFFER_BYTES	( 8 * 1024 )
#define MAX_OUTPUT_BUFFER_BYTES	( 64 * 1024 )

// macros

#ifdef DEBUG
#define  DBG(x...) printk(x)
#else
#define  DBG(x...)
#endif

// function prototypes

int init_module();
void cleanup_module();
void *module_alloc( int bytes );
void module_free( void *ptr );

#endif // #ifndef _RTF_MAIN_H
