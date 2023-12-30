#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "base_64.h"
#include <malloc.h>
#include "ZQ_common_conf.h"
//#ifdef ZQ_OS_LINUX
//#define _snprintf snprintf
//#endif

/* ---- Base64 Encoding/Decoding Table --- */
static const char table64[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char ultouc(unsigned long ulnum)
{
#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:810) /* conversion may lose significant bits */
#endif

	return (unsigned char)(ulnum & (unsigned long) 0xFF);

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif
}
static void decodeQuantum(unsigned char *dest, const char *src)
{
  const char *s, *p;
  unsigned long i, v, x = 0;

  for(i = 0, s = src; i < 4; i++, s++) {
    v = 0;
    p = table64;
    while(*p && (*p != *s)) {
      v++;
      p++;
    }
    if(*p == *s)
      x = (x << 6) + v;
    else if(*s == '=')
      x = (x << 6);
  }

  dest[2] = ultouc(x & 0xFFUL);
  x >>= 8;
  dest[1] = ultouc(x & 0xFFUL);
  x >>= 8;
  dest[0] = ultouc(x & 0xFFUL);
}

bool base64Decode(const char *src,
                            unsigned char **outptr, size_t *outlen)
{
  size_t length = 0;
  size_t equalsTerm = 0;
  size_t i;
  size_t numQuantums;
  unsigned char lastQuantum[3];
  size_t rawlen = 0;
  unsigned char *newstr;

  *outptr = 0;
  *outlen = 0;

  while((src[length] != '=') && src[length])
    length++;
  /* A maximum of two = padding characters is allowed */
  if(src[length] == '=') {
    equalsTerm++;
    if(src[length+equalsTerm] == '=')
      equalsTerm++;
  }
  numQuantums = (length + equalsTerm) / 4;

  /* Don't allocate a buffer if the decoded length is 0 */
  if(numQuantums == 0)
    return true;

  rawlen = (numQuantums * 3) - equalsTerm;

  /* The buffer must be large enough to make room for the last quantum
  (which may be partially thrown out) and the zero terminator. */
  newstr = (unsigned char*)malloc(rawlen+4);
  if(!newstr)
    return false;

  *outptr = newstr;

  /* Decode all but the last quantum (which may not decode to a
  multiple of 3 bytes) */
  for(i = 0; i < numQuantums - 1; i++) {
    decodeQuantum(newstr, src);
    newstr += 3; src += 4;
  }

  /* This final decode may actually read slightly past the end of the buffer
  if the input string is missing pad bytes.  This will almost always be
  harmless. */
  decodeQuantum(lastQuantum, src);
  for(i = 0; i < 3 - equalsTerm; i++)
    newstr[i] = lastQuantum[i];

  newstr[i] = '\0'; /* zero terminate */

  *outlen = rawlen; /* return size of decoded data */

  return true;
}

bool base64Encode(const char *inputbuff, size_t insize,
                            char **outptr, size_t *outlen)
{
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;
  char *output;
  char *base64data;
  char *convbuf = 0;

  const char *indata = inputbuff;

  *outptr = NULL;
  *outlen = 0;

  if(0 == insize)
    insize = strlen(indata);

  base64data = output = (char*)malloc(insize*4/3+4);
  if(NULL == output)
    return false;

  if(convbuf)
    indata = (char *)convbuf;

  while(insize > 0) {
    for(i = inputparts = 0; i < 3; i++) {
      if(insize > 0) {
        inputparts++;
        ibuf[i] = (unsigned char) *indata;
        indata++;
        insize--;
      }
      else
        ibuf[i] = 0;
    }

    obuf[0] = (unsigned char)  ((ibuf[0] & 0xFC) >> 2);
    obuf[1] = (unsigned char) (((ibuf[0] & 0x03) << 4) | \
                               ((ibuf[1] & 0xF0) >> 4));
    obuf[2] = (unsigned char) (((ibuf[1] & 0x0F) << 2) | \
                               ((ibuf[2] & 0xC0) >> 6));
    obuf[3] = (unsigned char)   (ibuf[2] & 0x3F);

    switch(inputparts) {
    case 1: /* only one byte read */
      snprintf(output, 5, "%c%c==",
               table64[obuf[0]],
               table64[obuf[1]]);
      break;
    case 2: /* two bytes read */
      snprintf(output, 5, "%c%c%c=",
               table64[obuf[0]],
               table64[obuf[1]],
               table64[obuf[2]]);
      break;
    default:
      snprintf(output, 5, "%c%c%c%c",
               table64[obuf[0]],
               table64[obuf[1]],
               table64[obuf[2]],
               table64[obuf[3]] );
      break;
    }
    output += 4;
  }
  *output = '\0';
  *outptr = base64data; /* return pointer to new data, allocated memory */

  if(convbuf)
    free(convbuf);

  *outlen = strlen(base64data); /* return the length of the new data */

  return true;
}
/* ---- End of Base64 Encoding ---- */
