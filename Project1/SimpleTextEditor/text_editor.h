/*
 * 简易文本编辑器 - 头文件
 * 支持基本文本编辑功能
 */

#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* 常量定义 */
#define MAX_LINE_LENGTH     4096    /* 每行最大字符数，提升上限以减少限制感 */
#define MAX_LINES           1000    /* 最大行数 */
#define MAX_FILENAME        256     /* 文件名最大长度 */
#define BUFFER_SIZE         4096    /* 缓冲区大小 */

/* 字符统计结构体 */
typedef struct {
    int letter_count;       /* 英文字母数 */
    int digit_count;        /* 数字个数 */
    int space_count;        /* 空格个数 */
    int total_count;        /* 总字符数 */
    int punctuation_count;  /* 标点符号数 */
    int other_count;        /* 其他字符数 */
    int chinese_count;      /* 中文字符数 */
} CharStatistics;

/* 文本缓冲区结构体 */
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH + 1];  /* 文本行数组 */
    int line_count;                               /* 当前行数 */
    int modified;                                 /* 是否被修改 */
    char filename[MAX_FILENAME];                  /* 当前文件名 */
} TextBuffer;

/* 搜索结果结构体 */
typedef struct {
    int line;       /* 行号 */
    int column;     /* 列号 */
} SearchResult;

/* ========================== 函数声明 ========================== */

/* 初始化和清理函数 */
void buffer_init(TextBuffer *buf);
void buffer_clear(TextBuffer *buf);

/* 缓冲区查询函数 */
int get_line_count(const TextBuffer *buf);
const char* get_line(const TextBuffer *buf, int line_num);
const char* get_filename(const TextBuffer *buf);
int is_modified(const TextBuffer *buf);

/* 文本输入功能 */
int insert_line(TextBuffer *buf, int line_num, const char *text);

/* 文件操作功能 */
int file_open(TextBuffer *buf, const char *filename);
int file_save(TextBuffer *buf, const char *filename);
int file_save_current(TextBuffer *buf);

/* 字符统计功能 */
CharStatistics count_characters(const TextBuffer *buf);
int is_letter(char c);
int is_digit_char(char c);
int is_space_char(char c);
int is_punctuation(char c);

/* UTF-8 辅助函数 */
int utf8_strlen_chars(const char *s);

/* 子串查找功能 */
int find_substring_count(TextBuffer *buf, const char *substr);
SearchResult* find_all_occurrences(TextBuffer *buf, const char *substr, int *count);

/* 子串插入功能 */
int insert_substring(TextBuffer *buf, int line, int col, const char *substr);
int insert_at_position(TextBuffer *buf, int pos, const char *substr);

/* 子串修改功能 */
int replace_at_position(TextBuffer *buf, int line, int col, int len, const char *newstr);
int replace_char(TextBuffer *buf, int line, int col, const char *newchar_utf8);
int replace_all(TextBuffer *buf, const char *oldstr, const char *newstr);

/* 子串删除功能 */
int delete_substring(TextBuffer *buf, const char *substr);
int delete_at_position(TextBuffer *buf, int line, int col, int len);
int delete_line(TextBuffer *buf, int line_num);


char* trim_string(char *str);
int get_total_length(const TextBuffer *buf);

#endif /* TEXT_EDITOR_H */
