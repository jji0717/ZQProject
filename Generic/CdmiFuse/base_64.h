#ifndef HEADER__BASE64_H
#define HEADER__BASE64_H

bool base64Encode(const char *inputbuff, size_t insize, char **outptr, size_t *outlen);

bool base64Decode(const char *src, unsigned char **outptr, size_t *outlen);

#endif /* HEADER__BASE64_H */
