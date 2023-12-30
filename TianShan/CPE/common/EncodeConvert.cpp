#include "EncodeConvert.h"

#ifdef ZQ_OS_LINUX

#include "iconv.h"

#ifndef PATH_MAX
#define PATH_MAX  512
#endif

static bool str2wstr(const std::string& str, std::wstring& wstr) {
	setlocale(LC_ALL, "");

	wchar_t wstr_[PATH_MAX] = {0};	
	int result = mbstowcs(wstr_, str.c_str(), str.length());

	if(result < 0) { 
		return false;
	}

	wstr = wstr_;
	return true;
}

static bool wstr2str(const std::wstring& wstr, std::string& str) {
	setlocale(LC_ALL, "");

	char str_[PATH_MAX] = {0};
	int result = wcstombs(str_, wstr.c_str(), PATH_MAX);

	if(result < 0) {
		return false;
	}
	str = str_;
	return true;
}

static bool convert(const char* from, const char* to, char* in, size_t in_len, char* out, size_t out_len) {
	if(!from || !to || !in || !out) {
		return false;
	}

	iconv_t conv = 0;
	if((conv = iconv_open(to, from)) == (iconv_t)-1) {
		return false;
	}
	if(iconv(conv, &in,  &in_len, &out, &out_len) == (size_t)-1) {
		return false;
	}
	return true;
}

static bool convert(const char* from, const char* to, const std::string& in, std::string& out) {
	if(!from || !to || !in.length()) {
		return false;
	}

	size_t in_len = in.length();

	char* src = new char[in_len+1];
	memset(src, 0, sizeof src);
	strncpy(src, in.data(), in_len); 

	char dest[PATH_MAX] = {0};
	bool result = convert(from, to, src, in_len, dest, PATH_MAX);
	
	if(result) {
		out = dest;
	}

	delete []src;
	return result;
}

static void print_hex(const char* s, size_t len) {
	size_t i = 0;
	for(; i < len; ++i) {
		printf("%X%X", (s[i]>>4)&0x0f, s[i]&0x0f);
		if(i <= (len - 1)) {
			printf(" ");
		}
	}
	printf("\n");
}

#endif


bool EncodeConvert::ansi_to_unicode(const std::string& ansi, std::wstring& unicode)
{
	if (ansi.empty())
		return false;

#ifdef ZQ_OS_MSWIN
	int nSize = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, 0, 0);
	if (!nSize)
		return false;

	std::auto_ptr<wchar_t> buf(new wchar_t[nSize]);	
	MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, buf.get(), nSize);

	unicode = buf.get();
	return true;
#else

	std::string dest;
	if(convert("gb2312", "utf-16le", ansi, dest)) {
		str2wstr(dest, unicode);	
		return true;
	}

	return false;

#endif
}

bool EncodeConvert::unicode_to_utf8(const std::wstring& unicode, std::string& utf8)
{
	if (unicode.empty())
		return false;

#ifdef ZQ_OS_MSWIN
	int nSize = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, 0, 0, 0, 0);
	if (!nSize)
		return false;

	std::auto_ptr<char> buf(new char[nSize]);	
	WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, buf.get(), nSize, 0, 0);

	utf8 = buf.get();
	return true;
#else

	std::string src;
	if(!wstr2str(unicode, src)) {
		return false;
	}
	if(!convert("utf-16le", "utf-8", src, utf8)) {
		return false;
	}

	return false;

#endif

}

bool EncodeConvert::utf8_to_unicode(const std::string& utf8, std::wstring& unicode)
{
	if (utf8.empty())
		return false;

#ifdef ZQ_OS_MSWIN
	int nSize = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, 0, 0);
	if (!nSize)
		return false;

	std::auto_ptr<wchar_t> buf(new wchar_t[nSize]);	
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buf.get(), nSize);

	unicode = buf.get();

	return true;
#else

	std::string dest;
	if(convert("utf-8", "utf-16le", utf8, dest)) {
		str2wstr(dest, unicode);	
		return true;
	}

	return false;

#endif
}

bool EncodeConvert::unicode_to_ansi(const std::wstring& unicode, std::string& ansi)
{
	if (unicode.empty())
		return false;

#ifdef ZQ_OS_MSWIN
	int nSize = WideCharToMultiByte(CP_ACP, 0, unicode.c_str(), -1, 0, 0, 0, 0);
	if (!nSize)
		return false;

	std::auto_ptr<char> buf(new char[nSize]);	
	WideCharToMultiByte(CP_ACP, 0, unicode.c_str(), -1, buf.get(), nSize, 0, 0);

	ansi = buf.get();
	return true;
#else

	std::string src;
	if(!wstr2str(unicode, src)) {
		return false;
	}
	if(!convert("utf-16le", "gb2312", src, ansi)) {
		return false;
	}	
	return true;

#endif
}

bool EncodeConvert::utf8_to_ansi(const std::string& utf8, std::string& ansi)
{
#ifdef ZQ_OS_MSWIN
	std::wstring unicode;
	if (!utf8_to_unicode(utf8, unicode))
		return false;

	if (!unicode_to_ansi(unicode, ansi))
		return false;

	return true;

#else

	if(!convert("utf-8", "gb2312", utf8, ansi)) {
		return false;
	}
	return true;

#endif
}

//ANSI to utf8
bool EncodeConvert::ansi_to_utf8(const std::string& ansi, std::string& utf8)
{
#ifdef ZQ_OS_MSWIN
	std::wstring unicode;

	if (!ansi_to_unicode(ansi,unicode))
		return false;

	if (!unicode_to_utf8(unicode,utf8))
		return false;

	return true;
#else

	if(!convert("gb2312", "utf-8", ansi, utf8)) {
		return false;
	}

	return false;

#endif
}


// vim: ts=4 sw=4 nu bg=dark
