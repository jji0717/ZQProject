#include <linux/module.h>

#include "RTFLib.h"

#include "RTFMain.h"

#ifdef TEST_FUNC
#include "RTFMainTest.h"
#endif

// local function prototypes **************************************************

static void module_err( char *pString );
static void module_log( RTF_APP_SESSION_HANDLE hAppSession, char *pString );

// library startup ************************************************************

int init_module()
{
  BOOL retval = FALSE;
  int status = 0;

  DBG("RTF Module: Calling rtfInitializeLibrary\n");

  retval = rtfInitializeLibrary( module_alloc, module_free, module_log, MAX_SESSION_COUNT,
			MAX_INPUT_BUFFER_BYTES, MAX_OUTPUT_BUFFER_BYTES, SES_FAIL_THRESHOLD, module_err );

  if( retval != TRUE )
  {
    printk("RTF Module intiialization error!\n");
    status = -1;
  }
  else
  {
    printk("RTF Module Initialized with fail session threshold %d\n", SES_FAIL_THRESHOLD);

#ifdef TEST_FUNC
	{
		char *argv[ 16 ];
		int argc = 0;

		argv[ argc++ ] = "module_test";	// application name
		argv[ argc++ ] = "S";			// serial execution (otherwise "P")
		argv[ argc++ ] = "test_clip_1";	// path to input stream
										// (outputs go to same path, with extensions)

		status = module_test( argc, argv );
		if( status != 0 )
		{
			printk( "module_test returned error\n" );
		}
	}
#endif

  }

  return status;
}

// library shutdown ***********************************************************

void cleanup_module()
{
  BOOL retval;

  DBG("RTF Module: Calling rtfShutdownLibrary\n");

  retval = rtfShutdownLibrary();

  if( retval != TRUE )
  {
    printk("RTF Module Shutdown Error!\n");
  }
  else
  {
    printk("RTF Module unloaded successfully\n");
  }
}

void *module_alloc( int bytes )
{
	return kmalloc( bytes, GFP_KERNEL );
}

void module_free( void *ptr )
{
	kfree( ptr );
}

// local functions ************************************************************

static void module_err( char *pString )
{
	printk( "module_err: %s\n", pString );
}

static void module_log( RTF_APP_SESSION_HANDLE hAppSession, char *pString )
{
	printk( "module_log: %s\n", pString );
}
