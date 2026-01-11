/*
 * 简易文本编辑器 - 核心功能实现
 */

#include "text_editor.h"

static void trim_line_endings(char *line) {
    if (!line) return;
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
}

/* ========================== 初始化和清理函数 ========================== */

/*
 * 初始化文本缓冲区
 */
void buffer_init(TextBuffer *buf) {
    if (buf == NULL) return;
    
    memset(buf->lines, 0, sizeof(buf->lines));
    buf->line_count = 0;
    buf->modified = 0;
    memset(buf->filename, 0, sizeof(buf->filename));
}

/*
 * 清空文本缓冲区
 */
void buffer_clear(TextBuffer *buf) {
    if (buf == NULL) return;
    buffer_init(buf);
}

/* ========================== 缓冲区查询函数 ========================== */

int get_line_count(const TextBuffer *buf) {
    return buf ? buf->line_count : 0;
}

const char* get_line(const TextBuffer *buf, int line_num) {
    if (!buf || line_num < 0 || line_num >= buf->line_count) return NULL;
    return buf->lines[line_num];
}

const char* get_filename(const TextBuffer *buf) {
    return buf ? buf->filename : NULL;
}

int is_modified(const TextBuffer *buf) {
    return buf ? buf->modified : 0;
}

/* ========================== 字符分类函数 ========================== */

static int is_cjk_char(int cp) {
    return (cp >= 0x4E00 && cp <= 0x9FFF) ||  /* CJK Unified Ideographs */
           (cp >= 0x3400 && cp <= 0x4DBF) ||  /* Extension A */
           (cp >= 0x20000 && cp <= 0x2A6DF) || /* Extension B */
           (cp >= 0x2A700 && cp <= 0x2B73F) || /* Extension C-D */
           (cp >= 0x2B740 && cp <= 0x2B81F) ||
           (cp >= 0x2B820 && cp <= 0x2CEAF) ||
           (cp >= 0xF900 && cp <= 0xFAFF);    /* Compatibility */
}

static int is_cjk_punctuation_cp(int cp) {
    return (cp >= 0x3000 && cp <= 0x303F) || /* CJK Symbols and Punctuation */
           (cp >= 0xFE30 && cp <= 0xFE4F) || /* CJK Compatibility Forms */
           (cp >= 0xFF00 && cp <= 0xFF65);   /* Fullwidth forms including punctuation */
}

static int is_fullwidth_space(int cp) {
    return cp == 0x3000; /* IDEOGRAPHIC SPACE */
}

static int is_fullwidth_digit(int cp) {
    return cp >= 0xFF10 && cp <= 0xFF19;
}

static int is_fullwidth_letter(int cp) {
    return (cp >= 0xFF21 && cp <= 0xFF3A) || (cp >= 0xFF41 && cp <= 0xFF5A);
}

int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_digit_char(char c) {
    return c >= '0' && c <= '9';
}

int is_space_char(char c) {
    return c == ' ' || c == '\t';
}

int is_punctuation(char c) {
    return (c >= '!' && c <= '/') ||
           (c >= ':' && c <= '@') ||
           (c >= '[' && c <= '`') ||
           (c >= '{' && c <= '~');
}

/* ========================== UTF-8 辅助函数 ========================== */
static int utf8_char_length(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; /* 非法字节按单字节处理，避免死循环 */
}

int utf8_strlen_chars(const char *s) {
    if (s == NULL) return 0;
    int count = 0;
    for (int i = 0; s[i] != '\0'; ) {
        int len = utf8_char_length((unsigned char)s[i]);
        i += len;
        count++;
    }
    return count;
}

static int utf8_byte_offset(const char *s, int char_index) {
    if (s == NULL || char_index < 0) return -1;
    int i = 0;
    int idx = 0;
    while (s[i] != '\0' && idx < char_index) {
        int len = utf8_char_length((unsigned char)s[i]);
        i += len;
        idx++;
    }
    if (idx == char_index) {
        return i;
    }
    return -1;
}

static int utf8_char_index_from_byte(const char *s, int byte_pos) {
    if (s == NULL || byte_pos < 0) return -1;
    int idx = 0;
    int i = 0;
    while (s[i] != '\0' && i < byte_pos) {
        int len = utf8_char_length((unsigned char)s[i]);
        i += len;
        idx++;
    }
    if (i == byte_pos) return idx;
    return -1;
}

/* ========================== 字符统计功能 ========================== */

/* UTF-8 读取下一个 code point */
static int utf8_next_codepoint(const unsigned char *p, int *advance) {
    if (p == NULL || advance == NULL) return -1;
    unsigned char c = *p;
    if (c < 0x80) { *advance = 1; return c; }
    if ((c & 0xE0) == 0xC0) {
        *advance = 2;
        return ((c & 0x1F) << 6) | (p[1] & 0x3F);
    }
    if ((c & 0xF0) == 0xE0) {
        *advance = 3;
        return ((c & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    }
    if ((c & 0xF8) == 0xF0) {
        *advance = 4;
        return ((c & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
    }
    *advance = 1; /* 非法字节，按单字节处理 */
    return c;
}

CharStatistics count_characters(const TextBuffer *buf) {
    CharStatistics stats = {0, 0, 0, 0, 0, 0, 0};
    
    if (buf == NULL) return stats;
    
    for (int i = 0; i < buf->line_count; i++) {
        const unsigned char *p = (const unsigned char *)buf->lines[i];
        while (*p != '\0') {
            int advance = 1;
            int cp = utf8_next_codepoint(p, &advance);
            if (advance <= 0) advance = 1;
            stats.total_count++;

            if (cp >= 0 && cp < 0x80) {
                char c = (char)cp;
                if (is_letter(c)) {
                    stats.letter_count++;
                } else if (is_digit_char(c)) {
                    stats.digit_count++;
                } else if (is_space_char(c)) {
                    stats.space_count++;
                } else if (is_punctuation(c)) {
                    stats.punctuation_count++;
                } else {
                    stats.other_count++;
                }
            } else if (is_fullwidth_space(cp)) {
                stats.space_count++;
            } else if (is_fullwidth_digit(cp)) {
                stats.digit_count++;
            } else if (is_fullwidth_letter(cp)) {
                stats.letter_count++;
            } else if (is_cjk_char(cp)) {
                stats.chinese_count++;
            } else if (is_cjk_punctuation_cp(cp)) {
                stats.punctuation_count++;
            } else {
                /* 其他特殊符号、表情等 */
                stats.other_count++;
            }
            p += advance;
        }
    }
    
    return stats;
}

/*
 * 在指定位置插入一行
 */
int insert_line(TextBuffer *buf, int line_num, const char *text) {
    if (buf == NULL || text == NULL) return -1;
    if (line_num < 0 || line_num > buf->line_count) return -1;
    if (buf->line_count >= MAX_LINES) return -1;
    
    /* 移动后面的行 */
    for (int i = buf->line_count; i > line_num; i--) {
        strcpy_s(buf->lines[i], MAX_LINE_LENGTH + 1, buf->lines[i - 1]);
    }
    
    /* 插入新行 */
    strncpy_s(buf->lines[line_num], sizeof(buf->lines[line_num]), text, _TRUNCATE);
    buf->line_count++;
    buf->modified = 1;
    
    return 0;
}

/* ========================== 文件操作功能 ========================== */

/*
 * 打开文件并读取内容到缓冲区
 */
int file_open(TextBuffer *buf, const char *filename) {
    FILE *fp = NULL;
    char line[MAX_LINE_LENGTH + 2];
    
    if (buf == NULL || filename == NULL) return -1;
    
    if (fopen_s(&fp, filename, "r") != 0 || fp == NULL) {
        return -1;
    }
    
    /* 清空当前缓冲区 */
    buffer_clear(buf);
    
    /* 读取文件内容 */
    while (fgets(line, sizeof(line), fp) != NULL && buf->line_count < MAX_LINES) {
        trim_line_endings(line);
        
        /* 截断过长的行 */
        size_t len = strlen(line);
        if (len > MAX_LINE_LENGTH) {
            line[MAX_LINE_LENGTH] = '\0';
        }
        
        strncpy_s(buf->lines[buf->line_count], sizeof(buf->lines[buf->line_count]), line, _TRUNCATE);
        buf->line_count++;
    }
    
    fclose(fp);
    
    /* 保存文件名 */
    strncpy_s(buf->filename, sizeof(buf->filename), filename, _TRUNCATE);
    buf->modified = 0;
    
    
    return 0;
}

/*
 * 保存缓冲区内容到指定文件
 */
int file_save(TextBuffer *buf, const char *filename) {
    FILE *fp = NULL;
    
    if (buf == NULL || filename == NULL) return -1;
    
    if (fopen_s(&fp, filename, "w") != 0 || fp == NULL) {
        return -1;
    }
    
    /* 写入所有行 */
    for (int i = 0; i < buf->line_count; i++) {
        fprintf(fp, "%s\n", buf->lines[i]);
    }
    
    fclose(fp);
    
    /* 更新文件名 */
    strncpy_s(buf->filename, sizeof(buf->filename), filename, _TRUNCATE);
    buf->modified = 0;
    
    return 0;
}

/*
 * 保存到当前文件
 */
int file_save_current(TextBuffer *buf) {
    if (buf == NULL) return -1;
    
    if (buf->filename[0] == '\0') {
        return -1;
    }
    
    return file_save(buf, buf->filename);
}

/* ========================== 子串查找功能 ========================== */

static void build_lps(const char *pattern, size_t m, int *lps) {
    size_t len = 0;
    lps[0] = 0;

    for (size_t i = 1; i < m; ) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = (int)len;
            i++;
        } else if (len != 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }
}

static int kmp_count_line(const char *text, const char *pattern, const int *lps, size_t m) {
    size_t n = strlen(text);
    size_t i = 0, j = 0;
    int count = 0;

    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
            if (j == m) {
                count++;
                j = (size_t)lps[j - 1];
            }
        } else if (j != 0) {
            j = (size_t)lps[j - 1];
        } else {
            i++;
        }
    }

    return count;
}

static int kmp_collect_line(const char *text, const char *pattern, const int *lps, size_t m,
                            int line_idx, SearchResult *results, int start_offset, int max_results) {
    size_t n = strlen(text);
    size_t i = 0, j = 0;
    int count = 0;

    while (i < n) {
        if (text[i] == pattern[j]) {
            i++;
            j++;
            if (j == m) {
                if (start_offset + count < max_results) {
                    int byte_pos = (int)(i - j);
                    int char_pos = utf8_char_index_from_byte(text, byte_pos);
                    if (char_pos < 0) char_pos = byte_pos; /* fallback */
                    results[start_offset + count].line = line_idx;
                    results[start_offset + count].column = char_pos;
                }
                count++;
                j = (size_t)lps[j - 1];
            }
        } else if (j != 0) {
            j = (size_t)lps[j - 1];
        } else {
            i++;
        }
    }

    return count;
}

/*
 * 统计子串在文本中出现的次数
 */
int find_substring_count(TextBuffer *buf, const char *substr) {
    int count = 0;

    if (buf == NULL || substr == NULL || substr[0] == '\0') return 0;

    size_t substr_len = strlen(substr);
    int *lps = (int*)malloc(sizeof(int) * substr_len);
    if (lps == NULL) return 0;

    build_lps(substr, substr_len, lps);

    for (int i = 0; i < buf->line_count; i++) {
        count += kmp_count_line(buf->lines[i], substr, lps, substr_len);
    }

    free(lps);
    return count;
}

/*
 * 获取所有出现位置
 */
SearchResult* find_all_occurrences(TextBuffer *buf, const char *substr, int *count) {
    if (buf == NULL || substr == NULL || count == NULL) return NULL;

    *count = find_substring_count(buf, substr);
    if (*count == 0) return NULL;

    SearchResult *results = (SearchResult*)malloc(sizeof(SearchResult) * (*count));
    if (results == NULL) {
        *count = 0;
        return NULL;
    }

    size_t substr_len = strlen(substr);
    int *lps = (int*)malloc(sizeof(int) * substr_len);
    if (lps == NULL) {
        free(results);
        *count = 0;
        return NULL;
    }

    build_lps(substr, substr_len, lps);

    int idx = 0;
    for (int i = 0; i < buf->line_count && idx < *count; i++) {
        idx += kmp_collect_line(buf->lines[i], substr, lps, substr_len, i, results, idx, *count);
    }

    free(lps);
    return results;
}

/* ========================== 子串插入功能 ========================== */

/*
 * 在指定行和列位置插入子串
 */
int insert_substring(TextBuffer *buf, int line, int col, const char *substr) {
    if (buf == NULL || substr == NULL) return -1;
    if (line < 0 || line >= buf->line_count) return -1;

    int byte_col = utf8_byte_offset(buf->lines[line], col);
    if (byte_col < 0) return -1;

    size_t line_len = strlen(buf->lines[line]);
    size_t substr_len = strlen(substr);

    /* 检查插入后是否超长（按字节限制） */
    if (line_len + substr_len > MAX_LINE_LENGTH) {
        return -1;
    }

    char temp[MAX_LINE_LENGTH + 1];
    strncpy_s(temp, sizeof(temp), buf->lines[line], byte_col);
    temp[byte_col] = '\0';
    strcat_s(temp, sizeof(temp), substr);
    strcat_s(temp, sizeof(temp), buf->lines[line] + byte_col);

    strcpy_s(buf->lines[line], MAX_LINE_LENGTH + 1, temp);
    buf->modified = 1;

    return 0;
}

/*
 * 在全局位置插入子串（按字符计），位置基于现有文本行及换行符顺序
 */
int insert_at_position(TextBuffer *buf, int pos, const char *substr) {
    if (buf == NULL || substr == NULL || pos < 0) return -1;

    /* 空缓冲区时，直接作为第一行插入 */
    if (buf->line_count == 0) {
        return insert_line(buf, 0, substr);
    }

    int cumulative = 0;
    for (int i = 0; i < buf->line_count; ++i) {
        int line_chars = utf8_strlen_chars(buf->lines[i]);
        /* 命中当前行 */
        if (pos <= cumulative + line_chars) {
            int col = pos - cumulative;
            return insert_substring(buf, i, col, substr);
        }

        cumulative += line_chars;

        /* 处理行与行之间的换行分隔（视为 1 个位置） */
        if (i != buf->line_count - 1) {
            if (pos == cumulative) {
                return insert_substring(buf, i + 1, 0, substr);
            }
            cumulative += 1;
        }
    }

    /* 允许在文本末尾追加 */
    int last_line = buf->line_count - 1;
    int tail_col = utf8_strlen_chars(buf->lines[last_line]);
    if (pos == cumulative) {
        return insert_substring(buf, last_line, tail_col, substr);
    }

    return -1;
}

/* ========================== 子串修改功能 ========================== */

/*
 * 在指定位置替换指定长度的内容
 */
int replace_at_position(TextBuffer *buf, int line, int col, int len, const char *newstr) {
    if (buf == NULL || newstr == NULL) return -1;
    if (line < 0 || line >= buf->line_count) return -1;
    if (len < 0) return -1;

    int line_chars = utf8_strlen_chars(buf->lines[line]);
    if (col < 0 || col >= line_chars) return -1;
    if (col + len > line_chars) len = line_chars - col;

    int byte_start = utf8_byte_offset(buf->lines[line], col);
    int byte_end = utf8_byte_offset(buf->lines[line], col + len);
    if (byte_start < 0) return -1;
    if (byte_end < 0) byte_end = (int)strlen(buf->lines[line]);

    size_t line_len = strlen(buf->lines[line]);
    size_t newstr_len = strlen(newstr);

    /* 检查替换后是否超长（按字节限制） */
    if (line_len - (size_t)(byte_end - byte_start) + newstr_len > MAX_LINE_LENGTH) {
        return -1;
    }

    char temp[MAX_LINE_LENGTH + 1];
    strncpy_s(temp, sizeof(temp), buf->lines[line], byte_start);
    temp[byte_start] = '\0';
    strcat_s(temp, sizeof(temp), newstr);
    strcat_s(temp, sizeof(temp), buf->lines[line] + byte_end);

    strcpy_s(buf->lines[line], MAX_LINE_LENGTH + 1, temp);
    buf->modified = 1;

    return 0;
}

/*
 * 替换单个字符
 */
int replace_char(TextBuffer *buf, int line, int col, const char *newchar_utf8) {
    if (buf == NULL || newchar_utf8 == NULL) return -1;
    if (line < 0 || line >= buf->line_count) return -1;

    int line_chars = utf8_strlen_chars(buf->lines[line]);
    if (col < 0 || col >= line_chars) return -1;

    int byte_start = utf8_byte_offset(buf->lines[line], col);
    int byte_end = utf8_byte_offset(buf->lines[line], col + 1);
    if (byte_start < 0) return -1;
    if (byte_end < 0) byte_end = (int)strlen(buf->lines[line]);

    size_t line_len = strlen(buf->lines[line]);
    size_t new_len = strlen(newchar_utf8);
    if (line_len - (size_t)(byte_end - byte_start) + new_len > MAX_LINE_LENGTH) return -1;

    char temp[MAX_LINE_LENGTH + 1];
    strncpy_s(temp, sizeof(temp), buf->lines[line], byte_start);
    temp[byte_start] = '\0';
    strcat_s(temp, sizeof(temp), newchar_utf8);
    strcat_s(temp, sizeof(temp), buf->lines[line] + byte_end);

    strcpy_s(buf->lines[line], MAX_LINE_LENGTH + 1, temp);
    buf->modified = 1;

    return 0;
}

/*
 * 替换所有匹配的子串
 */
int replace_all(TextBuffer *buf, const char *oldstr, const char *newstr) {
    int count = 0;

    if (buf == NULL || oldstr == NULL || newstr == NULL) return -1;
    if (oldstr[0] == '\0') return 0;

    size_t oldlen = strlen(oldstr);
    size_t newlen = strlen(newstr);

    for (int i = 0; i < buf->line_count; i++) {
        char temp[MAX_LINE_LENGTH + 1] = "";
        const char *p = buf->lines[i];
        const char *last = p;
        size_t temp_len = 0;

        while ((p = strstr(p, oldstr)) != NULL) {
            size_t prefix_len = (size_t)(p - last);
            if (temp_len + prefix_len + newlen > MAX_LINE_LENGTH) {
                /* 超长，停止本行替换，尽量保留到限制 */
                strncat_s(temp, sizeof(temp), last, MAX_LINE_LENGTH - temp_len);
                temp_len = MAX_LINE_LENGTH;
                break;
            }

            strncat_s(temp, sizeof(temp), last, prefix_len);
            temp_len += prefix_len;

            strcat_s(temp, sizeof(temp), newstr);
            temp_len += newlen;

            count++;
            p += oldlen;
            last = p;
        }

        if (temp_len < MAX_LINE_LENGTH && last != NULL) {
            size_t remain = strlen(last);
            if (temp_len + remain > MAX_LINE_LENGTH) remain = MAX_LINE_LENGTH - temp_len;
            strncat_s(temp, sizeof(temp), last, remain);
        }

        if (strcmp(buf->lines[i], temp) != 0) {
            strcpy_s(buf->lines[i], MAX_LINE_LENGTH + 1, temp);
            buf->modified = 1;
        }
    }

    if (count > 0) {
        /* 若有替换但未超长覆盖，modified 已在上方设定，此处保持一致 */
        buf->modified = 1;
    }

    return count;
}

/* ========================== 子串删除功能 ========================== */

/*
 * 删除所有匹配的子串
 */
int delete_substring(TextBuffer *buf, const char *substr) {
    if (buf == NULL || substr == NULL || substr[0] == '\0') return -1;
    
    /* 用空字符串替换实现删除 */
    return replace_all(buf, substr, "");
}

/*
 * 删除指定位置的字符
 */
int delete_at_position(TextBuffer *buf, int line, int col, int len) {
    if (buf == NULL) return -1;
    if (line < 0 || line >= buf->line_count) return -1;
    if (len < 0) return -1;

    int line_chars = utf8_strlen_chars(buf->lines[line]);
    if (col < 0 || col >= line_chars) return -1;
    if (col + len > line_chars) len = line_chars - col;

    int byte_start = utf8_byte_offset(buf->lines[line], col);
    int byte_end = utf8_byte_offset(buf->lines[line], col + len);
    if (byte_start < 0) return -1;
    if (byte_end < 0) byte_end = (int)strlen(buf->lines[line]);

    size_t line_len = strlen(buf->lines[line]);
    memmove(buf->lines[line] + byte_start,
            buf->lines[line] + byte_end,
            line_len - byte_end + 1);

    buf->modified = 1;

    return 0;
}

/*
 * 删除指定行
 */
int delete_line(TextBuffer *buf, int line_num) {
    if (buf == NULL) return -1;
    if (line_num < 0 || line_num >= buf->line_count) return -1;
    
    /* 移动后面的行 */
    for (int i = line_num; i < buf->line_count - 1; i++) {
        strcpy_s(buf->lines[i], MAX_LINE_LENGTH + 1, buf->lines[i + 1]);
    }
    
    buf->line_count--;
    buf->modified = 1;
    
    return 0;
}

/*
 * 去除字符串首尾空白
 */
char* trim_string(char *str) {
    if (str == NULL) return NULL;
    
    /* 去除尾部空白 */
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end-- = '\0';
    }
    
    /* 去除首部空白 */
    char *start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    return str;
}

/*
 * 获取文本总长度
 */
int get_total_length(const TextBuffer *buf) {
    int total = 0;
    
    if (buf == NULL) return 0;
    
    for (int i = 0; i < buf->line_count; i++) {
        total += (int)strlen(buf->lines[i]);
    }
    
    return total;
}
