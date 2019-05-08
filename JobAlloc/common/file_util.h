#pragma once
#ifndef _FILE_UTIL_H
#define _FILE_UTIL_H

class CStringArray;

class DLL_API FileUtil
{
public:
    //path_name末尾包含路径分隔符
    static void parse_file_name(const char *full_file_name, char *path_name, char *base_name, char *ext_name);
    //去除path_name末尾的路径分隔符
    static void clear_path_end_slash(char *path_name);
    //判断文件或目录是否存在
    static bool exists(const char *_name);
    //确保目录存在，不存在则自动生成
    static int ensure_path(const char *_path);
    //将字符串中不允许在文件名中出现的字符去除或者替换
    static void validate_filename_string(const char *string_buf, char *filename_buf, char replace_char=0);

    //遍历目录下的文件，browse_next返回<0即表示遍历结束，file_name返回的是完整路径
    static int browse_init(const char *_path, const char *file_spec);
    static long browse_next(char *file_name);
    static void browse_end();

    //返回目录下的子目录，不做深度遍历，返回目录名称列表,结尾不含"\\"
    static CStringArray *list_subdirs(const char *_path, const char *file_spec, bool is_full_name);
};





#endif