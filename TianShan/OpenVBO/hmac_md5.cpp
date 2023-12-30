#include "hmac_md5.h"

#define MAX_KEY_LEN (64)

void hmac_md5(uint8* text, uint32 text_len, uint8* key, uint32 key_len, uint8* digest)
{
    uint8 k_ipad[MAX_KEY_LEN+1];    /* inner padding -
                                  * key XORd with ipad
                                  */
    uint8 k_opad[MAX_KEY_LEN+1];    /* outer padding -
                                  * key XORd with opad
                                  */
    uint32 i;
    // if key is longer than 64 bytes reset it to key=MD5(key)
    if (key_len > MAX_KEY_LEN)
	{
		ZQ::common::md5 keymd5;
		keymd5.Update(key, key_len);
		keymd5.Finalize();
		memcpy(key, keymd5.Digest(), MD5_BYTE_LEN);
		key_len = MD5_BYTE_LEN;
	}

    /*
     * the HMAC_MD5 transform looks like:
     *
     * MD5(K XOR opad, MD5(K XOR ipad, text))
     *
     * where K is an n byte key
     * ipad is the byte 0x36 repeated 64 times
     * opad is the byte 0x5c repeated 64 times
     * and text is the data being protected
     */

    // start out by storing key in pads
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    // XOR key with ipad and opad values
    for (i=0; i<MAX_KEY_LEN; i++)
	{
            k_ipad[i] ^= 0x36;
            k_opad[i] ^= 0x5c;
    }
    
	// perform inner MD5
	{
		ZQ::common::md5 md5;
		md5.Update(k_ipad, MAX_KEY_LEN);     // start with inner pad
		md5.Update(text, text_len); // then text of datagram
		md5.Finalize();
		memcpy(digest, md5.Digest(), MD5_BYTE_LEN);
	}

    // perform outer MD5
	{
		ZQ::common::md5 md5;
		md5.Update(k_opad, MAX_KEY_LEN);     // start with inner pad
		md5.Update(digest, MD5_BYTE_LEN);     // then text of datagram
		md5.Finalize();

		memcpy(digest, md5.Digest(), MD5_BYTE_LEN);
	}
}



