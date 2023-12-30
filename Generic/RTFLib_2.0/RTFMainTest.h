// include file for kernel-mode library test loop

#ifndef _RTF_MAIN_TEST_H
#define _RTF_MAIN_TEST_H 1

// consitional complilation flags

// #define DO_COMBINED_FWD_REV			1

// configuration constants

#define TRICK_SPEED_COUNT			2

#ifdef DO_COMBINED_FWD_REV
#define SESSION_FLAGS				0
#define OUTPUT_FILE_COUNT			( 1 + TRICK_SPEED_COUNT )
#else
#define SESSION_FLAGS				RTF_SES_SEPARATE_DIRECTIONS
#define OUTPUT_FILE_COUNT			( 1 + ( 2 * TRICK_SPEED_COUNT ) )
#endif

#define INPUT_FILE_BUFFER_COUNT		256
#define INPUT_FILE_BUFFER_BYTES		(8 * 1024)
#define OUTPUT_FILE_BUFFER_BYTES	(64 * 1024)

// test loop function prototype

int module_test( int argc, char* argv[] );

#endif // #ifndef _RTF_MAIN_TEST_H
