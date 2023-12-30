#ifndef _hmac_md5_
#define _hmac_md5_
#include "md5.h"
void hmac_md5(uint8* text, uint32 text_len, uint8* key, uint32 key_len, uint8* digest);
#endif


