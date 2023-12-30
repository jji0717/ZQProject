#ifndef HEADER_HMAC_SHA1_H
#define HEADER_HMAC_SHA1_H

#include "sha1.h"

#define HMAC_SHA1_DIGEST_LENGTH	20
#define HMAC_SHA1_BLOCK_LENGTH	64

/* The HMAC_SHA1 structure: */
typedef struct{
	unsigned char	ipad[HMAC_SHA1_BLOCK_LENGTH];
	unsigned char	opad[HMAC_SHA1_BLOCK_LENGTH];
	SHA_CTX	shactx;
	unsigned char	key[HMAC_SHA1_BLOCK_LENGTH];
	unsigned int	keylen;
	unsigned int	hashkey;
} HMAC_SHA1_CTX;

#ifndef NOPROTO
void HMAC_SHA1_Init(HMAC_SHA1_CTX *ctx);
void HMAC_SHA1_UpdateKey(HMAC_SHA1_CTX *ctx, unsigned char *key, unsigned int keylen);
void HMAC_SHA1_EndKey(HMAC_SHA1_CTX *ctx);
void HMAC_SHA1_StartMessage(HMAC_SHA1_CTX *ctx);
void HMAC_SHA1_UpdateMessage(HMAC_SHA1_CTX *ctx, unsigned char *data, unsigned int datalen);
void HMAC_SHA1_EndMessage(unsigned char *out, HMAC_SHA1_CTX *ctx);
void HMAC_SHA1_Done(HMAC_SHA1_CTX *ctx);
void hmac_sha1(unsigned char *text, unsigned int text_len,
               unsigned char *key,  unsigned int key_len,
               unsigned char *digest);
#else
void HMAC_SHA1_Init();
void HMAC_SHA1_UpdateKey();
void HMAC_SHA1_EndKey();
void HMAC_SHA1_StartMessage();
void HMAC_SHA1_UpdateMessage();
void HMAC_SHA1_EndMessage();
void HMAC_SHA1_Done();
#endif


#endif





