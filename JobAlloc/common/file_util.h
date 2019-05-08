#pragma once
#ifndef _FILE_UTIL_H
#define _FILE_UTIL_H

class CStringArray;

class DLL_API FileUtil
{
public:
    //path_nameĩβ����·���ָ���
    static void parse_file_name(const char *full_file_name, char *path_name, char *base_name, char *ext_name);
    //ȥ��path_nameĩβ��·���ָ���
    static void clear_path_end_slash(char *path_name);
    //�ж��ļ���Ŀ¼�Ƿ����
    static bool exists(const char *_name);
    //ȷ��Ŀ¼���ڣ����������Զ�����
    static int ensure_path(const char *_path);
    //���ַ����в��������ļ����г��ֵ��ַ�ȥ�������滻
    static void validate_filename_string(const char *string_buf, char *filename_buf, char replace_char=0);

    //����Ŀ¼�µ��ļ���browse_next����<0����ʾ����������file_name���ص�������·��
    static int browse_init(const char *_path, const char *file_spec);
    static long browse_next(char *file_name);
    static void browse_end();

    //����Ŀ¼�µ���Ŀ¼��������ȱ���������Ŀ¼�����б�,��β����"\\"
    static CStringArray *list_subdirs(const char *_path, const char *file_spec, bool is_full_name);
};





#endif