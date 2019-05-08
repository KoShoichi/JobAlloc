#include "frame_public.h"
#include <algorithm>
#include "string_util.h"
#include "string_array.h"

#ifndef WIN32
#include <iconv.h>
#endif

void StringUtil::ltrim(char *buf)
{
	int len = (int)strlen(buf);
	if (len <= 0) return;
	char *ptr_a = buf;
	char *ptr_b = &buf[len+1]; //'\0'之后那个位置，那么复制的时候可以把'\0'一起向前移
	while(1) {
		if (*ptr_a == ' ' || *ptr_a == '\t' || *ptr_a == '\n' || *ptr_a == '\r') {
			++ptr_a;
			continue;
		}
		else
			break;
	}
	memcpy((void *)buf, (void *)ptr_a, ptr_b-ptr_a);
}

void StringUtil::rtrim(char *buf)
{
	int len = (int)strlen(buf);
	if (len <= 0) return;
	char *ptr = &buf[len-1];
	while(1) {
		if (ptr == buf) return;
		if (*ptr == '\0') return;
		if (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
			--ptr;
			continue;
		}
		else {
			*(++ptr) = '\0';
			return;
		}
	}
}

void StringUtil::trim(char *buf)
{
	rtrim(buf);
	ltrim(buf);
}

const char *StringUtil::split(const char *buf, char delimiter, char *first_buf)
{
    const char *ptr_a = buf;
    const char *ptr_b = ptr_a;	

    while(*ptr_b != '\0') {
        if (*ptr_b != delimiter) {
            ++ptr_b;
            continue;
        }

        if (ptr_b == ptr_a) {
            if (first_buf != NULL)
                first_buf[0] = '\0';            
        }
        else {
            if (first_buf != NULL) {
                strncpy(first_buf, ptr_a, ptr_b-ptr_a);
                first_buf[ptr_b-ptr_a] = '\0';
            }
        }
        return ++ptr_b;
    }
    if (ptr_b > ptr_a) {
        if (first_buf != NULL) {
            strncpy(first_buf, ptr_a, ptr_b-ptr_a);
            first_buf[ptr_b-ptr_a] = '\0';
        }        
    }
    return NULL;
}

CStringArray * StringUtil::split(const char *buf, char delimiter, CStringArray *pStringArray/*=NULL*/)
{
    CStringArray *ret_array = pStringArray;
    if (ret_array == NULL)
        ret_array = new CStringArray;

    const char *ptr_a = buf;
    const char *ptr_b = ptr_a;	

    while(*ptr_b != '\0') {
        if (*ptr_b != delimiter) {
            ++ptr_b;
            continue;
        }
        ret_array->push_back(ptr_a, ptr_b);
        ptr_a = ptr_b+1;
        ptr_b = ptr_a;
    }
    if (ptr_b > ptr_a) {
        ret_array->push_back(ptr_a, ptr_b);
    }
    return ret_array;
}

CStringArray *StringUtil::splitby0(const char *buf, int buf_size)
{
	CStringArray *ret_array = new CStringArray;

	const char *ptr_a = buf;
	const char *ptr_b = ptr_a;	

	while(ptr_b < (buf+buf_size)) {
		if (*ptr_b != '\0') {
			++ptr_b;
			continue;
		}

		if (ptr_b == ptr_a)
			break;
		else			
			ret_array->push_back(ptr_a, ptr_b);

		ptr_a = ptr_b+1;
		ptr_b = ptr_a;
	}
	if (ptr_b > ptr_a) {
		ret_array->push_back(ptr_a, ptr_b);
	}
	return ret_array;
}

void StringUtil::strupr(const char *buf, char *out_buf)
{
    std::string &str = std::string(buf);
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    strcpy(out_buf, str.c_str());
}

void StringUtil::strlwr(const char *buf, char *out_buf)
{
    std::string str = buf;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    strcpy(out_buf, str.c_str());
}

const char * StringUtil::find_first_digit(const char *buf)
{
    const char *ptr_b = buf;	

    while(*ptr_b != '\0') {
        if (*ptr_b <= '0' || *ptr_b >= '9') {
            ++ptr_b;
            continue;
        }
        return ptr_b;
    }    
    return NULL;
}

int StringUtil::sprintf_cat(char *buf, const char *fmt, ...)
{
    int ret = -1;
    char *p = &buf[strlen(buf)];
    va_list args;
    va_start(args, fmt);
    ret = vsprintf(p, fmt, args);
    va_end(args);
    return ret;
}


#ifdef WIN32
void StringUtil::utf8_to_gb(char* src, char* dst, int len)
{
    int ret = 0;
    WCHAR* strA;
    int i= MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (i <= 0) {
        printf("ERROR.");
        return;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_ACP, 0, strA, -1, NULL, 0, NULL, NULL);
    if (len >= i) {
        ret = WideCharToMultiByte(CP_ACP, 0, strA, -1, dst, i, NULL, NULL);
        dst[i] = 0;
    }
    if (ret <= 0) {
        free(strA);
        return;
    }

    free( strA );
}

void StringUtil::gb_to_utf8(char* src, char* dst, int len)
{
    int ret = 0;
    WCHAR* strA;
    int i= MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    if (i <= 0) {
        printf("ERROR.");
        return;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_ACP, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_UTF8, 0, strA, -1, NULL, 0, NULL, NULL);
    if (len >= i) {
        ret = WideCharToMultiByte(CP_UTF8, 0, strA, -1, dst, i, NULL, NULL);
        dst[i] = 0;
    }

    if (ret <= 0) {
        free(strA);
        return;
    }
    free(strA);
}
#else   //Linux
// starkwong: In iconv implementations, inlen and outlen should be type of size_t not uint, which is different in length on Mac
void StringUtil::utf8_to_gb(char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;
    char* inbuf = src;
    char* outbuf = dst;
    iconv_t cd;
    cd = iconv_open("GBK", "UTF-8");
    if (cd != (iconv_t)-1) {
        ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
        if (ret != 0) {
            printf("iconv failed err: %s\n", strerror(errno));
        }

        iconv_close(cd);
    }
}

void StringUtil::gb_to_utf8(char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;
    char* inbuf = src;
    char* outbuf2 = NULL;
    char* outbuf = dst;
    iconv_t cd;

    // starkwong: if src==dst, the string will become invalid during conversion since UTF-8 is 3 chars in Chinese but GBK is mostly 2 chars
    if (src == dst) {
        outbuf2 = (char*)malloc(len);
        memset(outbuf2, 0, len);
        outbuf = outbuf2;
    }

    cd = iconv_open("UTF-8", "GBK");
    if (cd != (iconv_t)-1) {
        ret = iconv(cd, &inbuf, &inlen, &outbuf, &outlen);
        if (ret != 0)
            printf("iconv failed err: %s\n", strerror(errno));

        if (outbuf2 != NULL) {
            strcpy(dst, outbuf2);
            free(outbuf2);
        }

        iconv_close(cd);
    }
}



#endif
