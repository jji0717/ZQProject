
#ifndef __SHA1_H__
#define __SHA1_H__

//#include <string.h>

/* Define this if your machine is LITTLE_ENDIAN, otherwise #undef it: */
// #define LITTLE_ENDIAN 20061122 It'll be defined in makefile
// 20061122 Modify it _BIG_ENDIAN_ definition in makefile, not use LITTLE_ENDIAN

/* Make sure you define these types for your architecture: */
typedef unsigned int  sha1_quadbyte;	/* 4 byte type */
typedef unsigned char sha1_byte;	/* single byte type */

/*
 * Be sure to get the above definitions right.  For instance, on my
 * x86 based FreeBSD box, I define LITTLE_ENDIAN and use the type
 * "unsigned long" for the quadbyte.  On FreeBSD on the Alpha, however,
 * while I still use LITTLE_ENDIAN, I must define the quadbyte type
 * as "unsigned int" instead.
 */

#define SHA1_BLOCK_LENGTH	64
#define SHA1_DIGEST_LENGTH	20

/* The SHA1 structure: */
typedef struct _SHA_CTX {
	sha1_quadbyte	state[5];
	sha1_quadbyte	count[2];
	sha1_byte	buffer[SHA1_BLOCK_LENGTH];
} SHA_CTX;

#ifndef NOPROTO
void SHA1_Init(SHA_CTX *context);
void SHA1_Update(SHA_CTX *context, sha1_byte *data, unsigned int len);
void SHA1_Final(sha1_byte digest[SHA1_DIGEST_LENGTH], SHA_CTX* context);
#else
void SHA1_Init();
void SHA1_Update();
void SHA1_Final();
#endif


#endif