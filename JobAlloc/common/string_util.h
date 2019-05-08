#pragma once
#ifndef STRING_UTIL_H
#define STRING_UTIL_H


#define STR0(s) (strlen((s))>0 ? (s) : "0")
//注意：d只能是数组，不能是指针
#define Strncpy(d, s) strncpy(d, s, sizeof(d)-1); d[sizeof(d)-1]=0

class CStringArray;
class DLL_API StringUtil
{
	public:
		static void ltrim(char *buf);
		static void rtrim(char *buf);
		static void trim(char *buf);

		static const char   *split(const char *buf, char delimiter, char *first_buf);	//first_buf可以为NULL
		static CStringArray *split(const char *buf, char delimiter, CStringArray *pStringArray=NULL);
		static CStringArray *splitby0(const char *buf, int buf_size);


		static void strupr(const char *buf, char *out_buf);
		static void strlwr(const char *buf, char *out_buf);


		static const char *find_first_digit(const char *buf);

		static int sprintf_cat(char *buf, const char *fmt, ...);

		/**
		 * UTF-8 to GBK
		 * @param src [in]
		 * @param dst [out]
		 * @param len [in] The most bytes which starting at dst, will be written.
		 *
		 */
		static void utf8_to_gb(char* src, char* dst, int len);

		/**
		 * GBK to UTF-8
		 *
		 * @param src [in]
		 * @param dst [out]
		 * @param len [in] The most bytes which starting at dst, will be written.
		 */
		static void gb_to_utf8(char* src, char* dst, int len);

};
#endif
