#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "Hmac_sha1.h"

/* Filler bytes: */
#define IPAD_BYTE	0x36
#define OPAD_BYTE	0x5c
#define ZERO_BYTE	0x00

void HMAC_SHA1_Init(HMAC_SHA1_CTX *ctx) {
	memset(&(ctx->key[0]), ZERO_BYTE, HMAC_SHA1_BLOCK_LENGTH);
	memset(&(ctx->ipad[0]), IPAD_BYTE, HMAC_SHA1_BLOCK_LENGTH);
	memset(&(ctx->opad[0]), OPAD_BYTE, HMAC_SHA1_BLOCK_LENGTH);
	ctx->keylen = 0;
	ctx->hashkey = 0;
}

void HMAC_SHA1_UpdateKey(HMAC_SHA1_CTX *ctx, unsigned char *key, unsigned int keylen) {

	/* Do we have anything to work with?  If not, return right away. */
	if (keylen < 1)
		return;

	/*
	 * Is the total key length (current data and any previous data)
	 * longer than the hash block length?
	 */
	if (ctx->hashkey !=0 || (keylen + ctx->keylen) > HMAC_SHA1_BLOCK_LENGTH) {
		/*
		 * Looks like the key data exceeds the hash block length,
		 * so that means we use a hash of the key as the key data
		 * instead.
		 */
		if (ctx->hashkey == 0) {
			/*
			 * Ah, we haven't started hashing the key
			 * data yet, so we must init. the hash
			 * monster to begin feeding it.
			 */

			/* Set the hash key flag to true (non-zero) */
			ctx->hashkey = 1;

			/* Init. the hash beastie... */
			SHA1_Init(&ctx->shactx);

			/* If there's any previous key data, use it */
			if (ctx->keylen > 0) {
				SHA1_Update(&ctx->shactx, &(ctx->key[0]), ctx->keylen);
			}

			/*
			 * Reset the key length to the future true
			 * key length, HMAC_SHA1_DIGEST_LENGTH
			 */
			ctx->keylen = HMAC_SHA1_DIGEST_LENGTH;
		}
		/* Now feed the latest key data to the has monster */
		SHA1_Update(&ctx->shactx, key, keylen);
	} else {
		/*
		 * Key data length hasn't yet exceeded the hash
		 * block length (HMAC_SHA1_BLOCK_LENGTH), so theres
		 * no need to hash the key data (yet).  Copy it
		 * into the key buffer.
		 */
		memcpy(&(ctx->key[ctx->keylen]), key, keylen);
		ctx->keylen += keylen;
	}
}

void HMAC_SHA1_EndKey(HMAC_SHA1_CTX *ctx) {
	unsigned char	*ipad, *opad, *key;
	unsigned int	i;

	/* Did we end up hashing the key? */
	if (ctx->hashkey) {
		memset(&(ctx->key[0]), ZERO_BYTE, HMAC_SHA1_BLOCK_LENGTH);
		/* Yes, so finish up and copy the key data */
		SHA1_Final(&(ctx->key[0]), &ctx->shactx);
		/* ctx->keylen was already set correctly */
	}
	/* Pad the key if necessary with zero bytes */
	if ((i = HMAC_SHA1_BLOCK_LENGTH - ctx->keylen) > 0) {
		memset(&(ctx->key[ctx->keylen]), ZERO_BYTE, i);
	}

	ipad = &(ctx->ipad[0]);
	opad = &(ctx->opad[0]);

	/* Precompute the respective pads XORed with the key */
	key = &(ctx->key[0]);
	for (i = 0; i < ctx->keylen; i++, key++) {
		/* XOR the key byte with the appropriate pad filler byte */
		*ipad++ ^= *key;
		*opad++ ^= *key;
	}
}

void HMAC_SHA1_StartMessage(HMAC_SHA1_CTX *ctx) {
	SHA1_Init(&ctx->shactx);
	SHA1_Update(&ctx->shactx, &(ctx->ipad[0]), HMAC_SHA1_BLOCK_LENGTH);
}

void HMAC_SHA1_UpdateMessage(HMAC_SHA1_CTX *ctx, unsigned char *data, unsigned int datalen) {
	SHA1_Update(&ctx->shactx, data, datalen);
}

void HMAC_SHA1_EndMessage(unsigned char *out, HMAC_SHA1_CTX *ctx) {
	unsigned char	buf[HMAC_SHA1_DIGEST_LENGTH];
	SHA_CTX		*c = &ctx->shactx;

	SHA1_Final(&(buf[0]), c);
	SHA1_Init(c);
	SHA1_Update(c, &(ctx->opad[0]), HMAC_SHA1_BLOCK_LENGTH);
	SHA1_Update(c, buf, HMAC_SHA1_DIGEST_LENGTH);
	SHA1_Final(out, c);
}

void HMAC_SHA1_Done(HMAC_SHA1_CTX *ctx) {
	/* Just to be safe, toast all context data */
	memset(&(ctx->ipad[0]), ZERO_BYTE, HMAC_SHA1_BLOCK_LENGTH);
	memset(&(ctx->ipad[0]), ZERO_BYTE, HMAC_SHA1_BLOCK_LENGTH);
	memset(&(ctx->key[0]), ZERO_BYTE, HMAC_SHA1_BLOCK_LENGTH);
	ctx->keylen = 0;
	ctx->hashkey = 0;
} 

void hmac_sha1(unsigned char *text, unsigned int text_len,
               unsigned char *key,  unsigned int key_len,
               unsigned char *digest)
{
	HMAC_SHA1_CTX	ctx;
	
	HMAC_SHA1_Init(&ctx);
	
	HMAC_SHA1_UpdateKey(&ctx, key, key_len);
	HMAC_SHA1_EndKey(&ctx);
	
	HMAC_SHA1_StartMessage(&ctx);
	HMAC_SHA1_UpdateMessage(&ctx, text, text_len);
	HMAC_SHA1_EndMessage(digest, &ctx); 
}

/*
#include <string>
int main(int argc, char* argv[])
{
	char* key =  ("c27ab9228f148f89d9e6bb2ce4ac67f002d0c5e8debf90b646351ab16484");
  char* text = "GET\napplication/cdmi-object\n1365664824796\n/aqua/rest/cdmi/MyContainerCDMI_HL006/%e6%b5%8b%e8%af%95%e4%b8%ad%e6%96%8744.txt";
//	char* key = "key";
//   char* text = "The quick brown fox jumps over the lazy dog";

	printf("data: [%s]\n", text);
	for(int i = 0; i< strlen(text); i++)
	{
		printf("0x%02x,", *(text + i));
	}
	printf("\nkey:[%s]\n", key);

	for(int i = 0; i< strlen(key); i++)
	{
		printf("0x%02x,", *(key + i));
	}
	printf("\n");

//	char text[256] =  ("456");
//	char key[256] = "123";

	int keyLength = strlen(key);
	unsigned char digest[21]="";
//	hmac_sha1((unsigned char*)text,strlen(text), (unsigned char*)key, keyLength, digest);

	hmac_sha1((unsigned char*)text,strlen(text), (unsigned char*)key, strlen(key), digest);
//    printf("length(%d)str(%s)\n",strlen((char*)digest), digest);

	printf("hmac_sha1 result:\n");
	for(int i = 0; i< strlen((char*)digest); i++)
	{
		printf("0x%02x,", *(digest + i));
	}
	printf("\n");

	char* outptr;
	size_t outlen;
	bool ret =  base64Encode((char*)digest, strlen((char*)digest), &outptr, &outlen);

	std::string base64;
	if(outptr && outlen > 0)
	{
		base64.append(outptr, outlen);
        delete outptr;
		outptr = NULL;
	}
	
	printf("%s\n", base64.c_str());
	return 0;
}
*/
