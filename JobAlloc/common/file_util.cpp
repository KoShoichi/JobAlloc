#include "frame_public.h"
#include "file_util.h"
#include "string_util.h"
#include "string_array.h"


void FileUtil::parse_file_name(const char *full_file_name, char *path_name, char *base_name, char *ext_name)
{
    int full_file_len = 0;
    if (path_name != NULL) path_name[0] = 0;
    if (base_name != NULL) base_name[0] = 0;
    if (ext_name != NULL) ext_name[0] = 0;
    if (full_file_name == NULL) 
        return;

    //确保full_file_name中字符串长度必然大于0，之后pEnd和pHead必然不在同一位置
    if ((full_file_len=(int)strlen(full_file_name)) <= 0) 
        return;

    char *pHead = (char *)full_file_name;
    char *pEnd = pHead + full_file_len;
    char *pCur = pEnd - 1;

    //读扩展名
    int ext_len = 0;
    while(1) {
        if (pCur <= pHead)  break;
        if (*pCur == '\\' || *pCur == '/') break;
        if (*pCur == '.') {
            ++pCur;
            ext_len = (int)(pEnd-pCur);
            if (ext_len>0 && ext_name!=NULL)
                strncpy(ext_name, pCur, ext_len);
            break;
        }
        --pCur;
    }
    if (ext_name != NULL) ext_name[ext_len] = '\0';

    //读文件名，不包含扩展名
    pEnd = pEnd - ext_len;
    if (ext_len > 0) --pEnd; //扩展名大于0，那么还需要减去一个点号
    pCur = pEnd - 1;
    int file_len = 0;
    while(1) {
        if (*pCur == '\\' || *pCur == '/') {
            ++pCur;
            file_len = (int)(pEnd-pCur);
            if (file_len>0 && base_name!=NULL)
                strncpy(base_name, pCur, file_len);
            break;
        }
        if (pCur == pHead) {
            //到头了都没有找到路径分隔符，那么整个都属于base_name
            file_len = (int)(pEnd-pCur);
            if (file_len>0 && base_name!=NULL)
                strncpy(base_name, pCur, file_len);
            break;
        }
        --pCur;
    }
    if (base_name != NULL) base_name[file_len] = '\0';

    //读路径名
    pEnd = pHead + full_file_len - ext_len - file_len;
    if (ext_len > 0) --pEnd; //扩展名大于0，那么还需要减去一个点号
    int path_len = (int)(pEnd - pHead);
    if (path_name != NULL) {
        strncpy(path_name, pHead, path_len);
        path_name[path_len] = '\0';
    }
}

int FileUtil::ensure_path(const char *_path)
{    
    char path_name[MAX_PATH] = {0};
    char base_name[MAX_PATH] = {0};
    char ext_name[MAX_PATH] = {0};

    if (_path == NULL)
        return -1;

    //判断目录或者文件是否存在，如果不存在，那么递归判断父目录是否存在，直到最上层
    if (strlen(_path) == 0)
        return 0;

    if (exists(_path) == true)
        return 0;

    parse_file_name(_path, path_name, base_name, ext_name);
    clear_path_end_slash(path_name);
    int ret = ensure_path(path_name);
    if (ret != 0)
        return ret;

    //有扩展名则认为是文件，只需要确保之前的路径存在即可
    if (strlen(ext_name) > 0)
        return 0;

    if (strlen(base_name) <= 0)
        return 0;

    if (mkdir(_path, 0777) != 0)
        return -1;

    return 0;
}

bool FileUtil::exists(const char *_name)
{
    return (access(_name,0) == 0);
}

void FileUtil::clear_path_end_slash(char *path_name)
{
    if (path_name == NULL) return;

    int _len = (int)strlen(path_name);
    if (_len <= 0) return;

    char *pEnd = path_name + _len - 1;
    while(pEnd > path_name) {
        if (*pEnd == '\\' || *pEnd == '/') {
            *pEnd = '\0';
        }
        else
            return;

        --pEnd;
    }
}

void FileUtil::validate_filename_string(const char *string_buf, char *filename_buf, char replace_char/*=0*/)
{
    //文件名中不能出现 \/:*?"<>|
    const char *ptr_a = string_buf;
    char *ptr_b = filename_buf;
    while(*ptr_a != '\0') {
        if (*ptr_a == '\\' || *ptr_a == '/' || *ptr_a == ':' || *ptr_a == '*' || *ptr_a == '?' || *ptr_a == '"' || *ptr_a == '<' || *ptr_a == '>' || *ptr_a == '|') {
            if (replace_char != 0) {
                *ptr_b = replace_char;
                ++ptr_a;
                ++ptr_b;
            }
            else {
                ++ptr_a;
            }
        }
        else {
            *ptr_b = *ptr_a;
            ++ptr_a;
            ++ptr_b;
        }
    }
    *ptr_b = '\0';
    StringUtil::trim(filename_buf);
}

static char g_szOriginPath[MAX_PATH] = {0};
static char g_szBrowsePath[MAX_PATH] = {0};
static char g_szFileSpec[MAX_PATH] = {0};
static long g_hBrowseFile = -1; 

int FileUtil::browse_init(const char *_path, const char *file_spec)
{
    browse_end();

    //把程序原路径保存下来
    _getcwd(g_szOriginPath, MAX_PATH);    

    //先把_path转换为绝对路径  
    if (_fullpath(g_szBrowsePath, _path, MAX_PATH) == NULL)
        return -1;  

    //判断目录是否存在
    if (_chdir(g_szBrowsePath) != 0)
        return -1;

    //如果目录的最后一个字母不是'\',则在最后加上一个'\'  
    int len = (int)strlen(g_szBrowsePath);  
    if (g_szBrowsePath[len-1] != '\\')  
        strcat(g_szBrowsePath,"\\");

    strcpy(g_szFileSpec, file_spec);
    return 0;  
}

long FileUtil::browse_next(char *file_name)
{
    _finddata_t fileinfo;

    do {
        if (g_hBrowseFile != -1) {
            if (_findnext(g_hBrowseFile, &fileinfo) != 0)
                return -1;
        }
        else {
            g_hBrowseFile = (long)_findfirst(g_szFileSpec, &fileinfo);
            if (g_hBrowseFile == -1) return -1;
        }
    } while(fileinfo.attrib & _A_SUBDIR);	//是目录的话就继续寻找下一个

    if (file_name != NULL) {
        strcpy(file_name, g_szBrowsePath);
        strcat(file_name, fileinfo.name);
    }
    return g_hBrowseFile;
}

void FileUtil::browse_end()
{
    if (g_hBrowseFile != -1) {
        _findclose(g_hBrowseFile);        
        g_hBrowseFile = -1;
    }
    if (g_szOriginPath[0] != 0) {
        _chdir(g_szOriginPath);
        g_szOriginPath[0] = 0;
    }
}

CStringArray * FileUtil::list_subdirs(const char *_path, const char *file_spec, bool is_full_name)
{
    char szOriginPath[MAX_PATH] = {0};
    char szBrowsePath[MAX_PATH] = {0};
    long hBrowseFile = -1;
    CStringArray *retArray = new CStringArray;

    //把程序原路径保存下来
    _getcwd(szOriginPath, MAX_PATH);    

    //先把_path转换为绝对路径  
    if (_fullpath(szBrowsePath, _path, MAX_PATH) == NULL)
        return retArray;  

    //判断目录是否存在
    if (_chdir(szBrowsePath) != 0)
        return retArray;

    _finddata_t fileinfo;

    hBrowseFile = (long)_findfirst(file_spec, &fileinfo);
    if (hBrowseFile == -1) {
        if (szOriginPath[0] != 0) {
            _chdir(szOriginPath);
        }
        return retArray;
    }

    while(hBrowseFile >= 0) {
        if (_findnext(hBrowseFile, &fileinfo) != 0)
            break;

        if (fileinfo.attrib & _A_SUBDIR) {
            if (strcmp(fileinfo.name, "..") == 0)
                continue;

            if (is_full_name) {
                char buf[MAX_PATH] = {0};
                strcpy(buf, szBrowsePath);
                strcat(buf, "\\");
                strcat(buf, fileinfo.name);
                retArray->push_back(buf);
            }
            else {
                retArray->push_back(fileinfo.name);
            }            
        }
    }
    if (hBrowseFile != -1) {
        _findclose(hBrowseFile);
    }
    if (szOriginPath[0] != 0) {
        _chdir(szOriginPath);
    }
    return retArray;    
}

