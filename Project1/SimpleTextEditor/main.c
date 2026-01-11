/*
 * 简易文本编辑器 - 主程序
 * 
 * 功能列表:
 * 1. 新建/输入文本内容
 * 2. 打开/保存文本文件
 * 3. 字符统计
 * 4. 子串查找
 * 5. 子串插入
 * 6. 子串修改
 * 7. 子串删除
 * 8. 插件管理
 */

#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include "text_editor.h"
#include "plugin_manager.h"

/* 全局文本缓冲区 */
static TextBuffer g_buffer;

/* 函数声明 */
void menu_input_text(void);
void menu_open_file(void);
void menu_save_file(void);
void menu_statistics(void);
void menu_find_substring(void);
void menu_insert_substring(void);
void menu_modify_substring(void);
void menu_delete_substring(void);
void menu_display_text(void);
void menu_plugins(void);
int confirm_exit(void);

/* UI 辅助函数 */
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause_screen(void) {
    printf("\n按回车键继续...");
    getchar();
}

/* UI 日志回调 */
void ui_log_func(const char* msg) {
    printf("%s", msg);
}

static void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

static bool read_line_prompt(const char *prompt, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }
    if (prompt) {
        printf("%s", prompt);
    }
    if (!fgets(buffer, buffer_size, stdin)) {
        return false;
    }
    trim_newline(buffer);
    return true;
}

static bool read_int_range(const char *prompt, int min_value, int max_value, int *out_value) {
    char input[64];
    if (!out_value) {
        return false;
    }
    if (!read_line_prompt(prompt, input, sizeof(input))) {
        return false;
    }
    errno = 0;
    char *endptr = NULL;
    long value = strtol(input, &endptr, 10);
    if (endptr == input || *endptr != '\0' || errno == ERANGE) {
        return false;
    }
    if (value < min_value || value > max_value) {
        return false;
    }
    *out_value = (int)value;
    return true;
}

static bool read_yes_no(const char *prompt) {
    char input[8];
    if (!read_line_prompt(prompt, input, sizeof(input))) {
        return false;
    }
    return input[0] == 'y' || input[0] == 'Y';
}

void display_statistics(CharStatistics *stats) {
    if (stats == NULL) return;
    
    printf("\n========== 字符统计结果 ==========\n");
    printf("英文字母数: %d\n", stats->letter_count);
    printf("中文字符数: %d\n", stats->chinese_count);
    printf("数字个数:   %d\n", stats->digit_count);
    printf("空格个数:   %d\n", stats->space_count);
    printf("标点符号数: %d\n", stats->punctuation_count);
    printf("其他字符数: %d\n", stats->other_count);
    printf("总字符数:   %d\n", stats->total_count);
    printf("==================================\n");
}

void display_text(TextBuffer *buf) {
    if (buf == NULL) {
        printf("错误: 缓冲区为空\n");
        return;
    }
    
    int count = get_line_count(buf);
    const char *filename = get_filename(buf);
    int modified = is_modified(buf);

    printf("\n========== 当前文本内容 ==========\n");
    if (filename && filename[0] != '\0') {
        printf("文件: %s%s\n", filename, modified ? " [已修改]" : "");
    }
    printf("共 %d 行\n", count);
    printf("----------------------------------\n");
    
    if (count == 0) {
        printf("(空文档)\n");
    } else {
        for (int i = 0; i < count; i++) {
            const char *line = get_line(buf, i);
            if (line) {
                printf("%3d | %s\n", i + 1, line);
            }
        }
    }
    
    printf("==================================\n");
}

void display_menu(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║         简易文本编辑器 v1.0              ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║  1. 新建/输入文本内容                    ║\n");
    printf("║  2. 打开文本文件                         ║\n");
    printf("║  3. 保存文本文件                         ║\n");
    printf("║  4. 统计字符信息                         ║\n");
    printf("║  5. 查找子串出现次数                     ║\n");
    printf("║  6. 在指定位置插入子串                   ║\n");
    printf("║  7. 修改指定位置字符/子串                ║\n");
    printf("║  8. 删除指定子串                         ║\n");
    printf("║  9. 显示当前文本                         ║\n");
    printf("║ 10. 插件管理                             ║\n");
    printf("║  0. 退出系统                             ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf("请输入选项 (0-10): ");
}

/*
 * 从控制台逐行输入文本 (UI 实现)
 */
int input_text_ui(TextBuffer *buf) {
    char line[MAX_LINE_LENGTH + 2];
    
    if (buf == NULL) return -1;
    
    printf("\n请逐行输入文本内容（输入空行结束）:\n");
    printf("提示: 支持大小写英文字母、数字、标点符号及空格\n");
    printf("--------------------------------------------------\n");
    
    while (buf->line_count < MAX_LINES) {
        printf("第%d行: ", buf->line_count + 1);
        
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        
        trim_newline(line);
        size_t len = strlen(line);
        
        /* 空行结束输入 */
        if (len == 0) {
            break;
        }
        
        /* 检查行长度 */
        if (len > MAX_LINE_LENGTH) {
            printf("警告: 行长度超过%d字符，将被截断\n", MAX_LINE_LENGTH);
            line[MAX_LINE_LENGTH] = '\0';
        }
        
        /* 复制到缓冲区 */
        insert_line(buf, buf->line_count, line);
    }
    
    printf("--------------------------------------------------\n");
    printf("输入完成，共输入 %d 行\n", buf->line_count);
    
    return buf->line_count;
}

/*
 * 菜单: 新建/输入文本
 */
void menu_input_text(void) {
    int choice;
    
    printf("\n===== 新建/输入文本 =====\n");
    
    if (g_buffer.line_count > 0) {
        printf("当前已有文本（%d行），请选择操作:\n", g_buffer.line_count);
        printf("1. 清空后重新输入\n");
        printf("2. 追加到现有文本\n");
        printf("3. 取消\n");
        
        if (!read_int_range("请选择: ", 1, 3, &choice)) {
            printf("输入无效\n");
            return;
        }
        
        switch (choice) {
            case 1:
                buffer_clear(&g_buffer);
                break;
            case 2:
                /* 继续追加 */
                break;
            case 3:
                return;
            default:
                printf("无效选择\n");
                return;
        }
    }
    
    input_text_ui(&g_buffer);
    
    /* 显示输入结果 */
    if (g_buffer.line_count > 0) {
        printf("\n输入的文本内容:\n");
        display_text(&g_buffer);
    }
}

/*
 * 菜单: 打开文件
 */
void menu_open_file(void) {
    char filename[MAX_FILENAME];
    
    printf("\n===== 打开文本文件 =====\n");
    
    if (g_buffer.modified && !read_yes_no("警告: 当前文本已修改但未保存，是否继续? (y/n): ")) {
        return;
    }
    
    if (!read_line_prompt("请输入文件名: ", filename, sizeof(filename))) {
        return;
    }
    trim_string(filename);
    
    if (filename[0] == '\0') {
        printf("错误: 文件名不能为空\n");
        return;
    }
    
    if (file_open(&g_buffer, filename) == 0) {
        printf("成功打开文件 '%s'，共读取 %d 行\n", filename, g_buffer.line_count);
        display_text(&g_buffer);
    } else {
        printf("错误: 无法打开文件 '%s'\n", filename);
    }
}

/*
 * 菜单: 保存文件
 */
void menu_save_file(void) {
    char filename[MAX_FILENAME];
    int choice;
    
    printf("\n===== 保存文本文件 =====\n");
    
    if (g_buffer.line_count == 0) {
        printf("警告: 当前没有文本内容可保存\n");
        return;
    }
    
    if (g_buffer.filename[0] != '\0') {
        printf("当前文件: %s\n", g_buffer.filename);
        printf("1. 保存到当前文件\n");
        printf("2. 另存为新文件\n");
        printf("3. 取消\n");
        
        if (!read_int_range("请选择: ", 1, 3, &choice)) {
            printf("输入无效\n");
            return;
        }
        
        switch (choice) {
            case 1:
                if (file_save_current(&g_buffer) == 0) {
                    printf("成功保存到文件 '%s'，共写入 %d 行\n", g_buffer.filename, g_buffer.line_count);
                } else {
                    printf("错误: 保存失败\n");
                }
                return;
            case 2:
                break;
            case 3:
                return;
            default:
                printf("无效选择\n");
                return;
        }
    }
    
    if (!read_line_prompt("请输入保存的文件名: ", filename, sizeof(filename))) {
        return;
    }
    trim_string(filename);
    
    if (filename[0] == '\0') {
        printf("错误: 文件名不能为空\n");
        return;
    }
    
    if (file_save(&g_buffer, filename) == 0) {
        printf("成功保存到文件 '%s'，共写入 %d 行\n", filename, g_buffer.line_count);
    } else {
        printf("错误: 无法创建文件 '%s'\n", filename);
    }
}

/*
 * 菜单: 字符统计
 */
void menu_statistics(void) {
    printf("\n===== 统计字符信息 =====\n");
    
    if (g_buffer.line_count == 0) {
        printf("当前没有文本内容\n");
        return;
    }
    
    /* 先显示原文本 */
    printf("\n--- 原文本内容 ---\n");
    display_text(&g_buffer);
    
    /* 统计并显示结果 */
    CharStatistics stats = count_characters(&g_buffer);
    display_statistics(&stats);
}

/*
 * 菜单: 查找子串
 */
void menu_find_substring(void) {
    char substr[MAX_LINE_LENGTH];
    
    printf("\n===== 查找子串出现次数 =====\n");
    
    if (get_line_count(&g_buffer) == 0) {
        printf("当前没有文本内容\n");
        return;
    }
    
    /* 显示当前文本 */
    display_text(&g_buffer);
    
    if (!read_line_prompt("\n请输入要查找的子串: ", substr, sizeof(substr))) {
        return;
    }
    
    if (substr[0] == '\0') {
        printf("错误: 子串不能为空\n");
        return;
    }
    
    int count = 0;
    SearchResult *results = find_all_occurrences(&g_buffer, substr, &count);
    
    printf("\n========== 查找结果 ==========\n");
    printf("查找子串: \"%s\"\n", substr);
    printf("------------------------------\n");
    
    if (results != NULL && count > 0) {
        for (int i = 0; i < count; i++) {
            int line_idx = results[i].line;
            int col_idx = results[i].column;
            const char *line_content = get_line(&g_buffer, line_idx);
            if (line_content) {
                printf("第%d行，第%d列: %s\n", line_idx + 1, col_idx + 1, line_content);
            }
        }
        free(results);
    }
    
    printf("------------------------------\n");
    printf("共找到 %d 处匹配\n", count);
    printf("==============================\n");
}

/*
 * 菜单: 插入子串
 */
void menu_insert_substring(void) {
    int line, col;
    char substr[MAX_LINE_LENGTH];
    
    printf("\n===== 在指定位置插入子串 =====\n");
    
    if (g_buffer.line_count == 0) {
        printf("当前没有文本内容，请先输入文本\n");
        return;
    }
    
    /* 显示当前文本 */
    printf("\n--- 操作前文本 ---\n");
    display_text(&g_buffer);
    
    printf("\n请输入插入位置:\n");
    char prompt[64];
    snprintf(prompt, sizeof(prompt), "行号 (1-%d): ", g_buffer.line_count);
    if (!read_int_range(prompt, 1, g_buffer.line_count, &line)) {
        printf("输入无效\n");
        return;
    }
    
    printf("第%d行内容: %s\n", line, g_buffer.lines[line - 1]);
    int line_chars = utf8_strlen_chars(g_buffer.lines[line - 1]);
    snprintf(prompt, sizeof(prompt), "列号 (1-%d): ", line_chars + 1);
    if (!read_int_range(prompt, 1, line_chars + 1, &col)) {
        printf("输入无效\n");
        return;
    }
    
    if (!read_line_prompt("请输入要插入的子串: ", substr, sizeof(substr))) {
        return;
    }
    
    if (substr[0] == '\0') {
        printf("错误: 子串不能为空\n");
        return;
    }
    
    if (insert_substring(&g_buffer, line - 1, col - 1, substr) == 0) {
        printf("\n插入成功!\n");
        printf("\n--- 操作后文本 ---\n");
        display_text(&g_buffer);
    } else {
        printf("插入失败 (可能是行长度超过限制)\n");
    }
}

/*
 * 菜单: 修改子串
 */
void menu_modify_substring(void) {
    int choice;
    
    printf("\n===== 修改指定位置字符/子串 =====\n");
    
    if (g_buffer.line_count == 0) {
        printf("当前没有文本内容\n");
        return;
    }
    
    /* 显示当前文本 */
    printf("\n--- 操作前文本 ---\n");
    display_text(&g_buffer);
    
    printf("\n请选择修改方式:\n");
    printf("1. 修改指定位置的字符\n");
    printf("2. 修改指定位置的子串\n");
    printf("3. 替换所有匹配的子串\n");
    printf("4. 取消\n");
    
    if (!read_int_range("请选择: ", 1, 4, &choice)) {
        printf("输入无效\n");
        return;
    }
    
    switch (choice) {
        case 1: {
            /* 修改单个字符 */
            int line, col;
            char newchar_buf[MAX_LINE_LENGTH];
            char prompt[64];
            
            snprintf(prompt, sizeof(prompt), "行号 (1-%d): ", g_buffer.line_count);
            if (!read_int_range(prompt, 1, g_buffer.line_count, &line)) {
                printf("无效的行号\n");
                return;
            }
            
            printf("第%d行内容: %s\n", line, g_buffer.lines[line - 1]);
            int line_chars = utf8_strlen_chars(g_buffer.lines[line - 1]);
            snprintf(prompt, sizeof(prompt), "列号 (1-%d): ", line_chars);
            if (!read_int_range(prompt, 1, line_chars, &col)) {
                printf("无效的列号\n");
                return;
            }
            
            if (!read_line_prompt("请输入新字符: ", newchar_buf, sizeof(newchar_buf))) {
                return;
            }
            if (newchar_buf[0] == '\0') {
                printf("输入为空，已取消\n");
                return;
            }

            if (replace_char(&g_buffer, line - 1, col - 1, newchar_buf) == 0) {
                printf("\n修改成功!\n");
                printf("\n--- 操作后文本 ---\n");
                display_text(&g_buffer);
            } else {
                printf("修改失败\n");
            }
            break;
        }
        
        case 2: {
            /* 修改指定位置子串 */
            int line, col, len;
            char newstr[MAX_LINE_LENGTH];
            char prompt[64];
            
            snprintf(prompt, sizeof(prompt), "行号 (1-%d): ", g_buffer.line_count);
            if (!read_int_range(prompt, 1, g_buffer.line_count, &line)) {
                printf("无效的行号\n");
                return;
            }
            
            printf("第%d行内容: %s\n", line, g_buffer.lines[line - 1]);
            int line_chars = utf8_strlen_chars(g_buffer.lines[line - 1]);
            snprintf(prompt, sizeof(prompt), "列号 (1-%d): ", line_chars);
            if (!read_int_range(prompt, 1, line_chars, &col)) {
                printf("无效的列号\n");
                return;
            }
            int max_len = line_chars - col + 1;
            snprintf(prompt, sizeof(prompt), "要替换的长度(字符数 1-%d): ", max_len);
            if (!read_int_range(prompt, 1, max_len, &len)) {
                printf("无效的长度\n");
                return;
            }
            
            if (!read_line_prompt("请输入新的字符串: ", newstr, sizeof(newstr))) {
                return;
            }
            
            if (replace_at_position(&g_buffer, line - 1, col - 1, len, newstr) == 0) {
                printf("\n修改成功!\n");
                printf("\n--- 操作后文本 ---\n");
                display_text(&g_buffer);
            } else {
                printf("修改失败 (可能是行长度超过限制)\n");
            }
            break;
        }
        
        case 3: {
            /* 替换所有匹配子串 */
            char oldstr[MAX_LINE_LENGTH], newstr[MAX_LINE_LENGTH];
            
            if (!read_line_prompt("请输入要查找的子串: ", oldstr, sizeof(oldstr))) {
                return;
            }
            if (!read_line_prompt("请输入替换为的新子串: ", newstr, sizeof(newstr))) {
                return;
            }
            
            int count = replace_all(&g_buffer, oldstr, newstr);
            if (count > 0) {
                printf("\n成功替换 %d 处!\n", count);
                printf("\n--- 操作后文本 ---\n");
                display_text(&g_buffer);
            } else if (count == 0) {
                printf("未找到匹配的子串\n");
            } else {
                printf("替换失败\n");
            }
            break;
        }
        
        case 4:
            return;
            
        default:
            printf("无效选择\n");
    }
}

/*
 * 菜单: 删除子串
 */
void menu_delete_substring(void) {
    char substr[MAX_LINE_LENGTH];
    
    printf("\n===== 删除指定子串 =====\n");
    
    if (g_buffer.line_count == 0) {
        printf("当前没有文本内容\n");
        return;
    }
    
    /* 显示当前文本 */
    printf("\n--- 操作前文本 ---\n");
    display_text(&g_buffer);
    
    if (!read_line_prompt("\n请输入要删除的子串: ", substr, sizeof(substr))) {
        return;
    }
    
    if (substr[0] == '\0') {
        printf("错误: 子串不能为空\n");
        return;
    }
    
    /* 先统计匹配数 */
    int found = find_substring_count(&g_buffer, substr);
    if (found == 0) {
        printf("未找到匹配的子串\n");
        return;
    }
    
    char prompt[64];
    snprintf(prompt, sizeof(prompt), "找到 %d 处匹配，确认删除? (y/n): ", found);
    if (!read_yes_no(prompt)) {
        printf("操作已取消\n");
        return;
    }
    
    int deleted = delete_substring(&g_buffer, substr);
    if (deleted >= 0) {
        printf("\n成功删除 %d 处子串!\n", deleted);
        printf("\n--- 操作后文本 ---\n");
        display_text(&g_buffer);
    } else {
        printf("删除失败\n");
    }
}

/*
 * 菜单: 显示文本
 */
void menu_display_text(void) {
    display_text(&g_buffer);
    
    /* 同时显示统计信息 */
    if (g_buffer.line_count > 0) {
        CharStatistics stats = count_characters(&g_buffer);
        display_statistics(&stats);
    }
}

/*
 * 菜单: 插件管理
 */
void menu_plugins(void) {
    int choice;
    char input[MAX_FILENAME];

    while (1) {
        printf("\n===== 插件管理 =====\n");
        printf("1. 扫描并加载 plugins 目录下的 DLL 插件\n");
        printf("2. 列出插件命令\n");
        printf("3. 执行插件命令\n");
        printf("4. 返回主菜单\n");

        if (!read_int_range("请选择: ", 1, 4, &choice)) {
            printf("输入无效\n");
            continue;
        }

        switch (choice) {
            case 1:
                load_plugins_default();
                break;
            case 2: {
                PluginCommand* p = get_plugin_commands();
                printf("\n===== 插件命令列表 =====\n");
                if (!p) {
                    printf("(暂无)\n");
                }
                while (p) {
                    printf("%-16s [native] : %s\n", p->name, p->description);
                    p = p->next;
                }
                printf("========================\n");
                break;
            }
            case 3: {
                PluginCommand* p = get_plugin_commands();
                printf("\n===== 插件命令列表 =====\n");
                if (!p) {
                    printf("(暂无)\n");
                }
                while (p) {
                    printf("%-16s [native] : %s\n", p->name, p->description);
                    p = p->next;
                }
                printf("========================\n");

                printf("请输入命令名称: ");
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    trim_string(input);
                    if (input[0] != '\0') {
                        if (execute_plugin_command(input) != 0) {
                            printf("命令执行失败或未找到\n");
                        }
                    }
                }
                break;
            }
            case 4:
                return;
            default:
                printf("无效选择\n");
        }
    }
}

/*
 * 确认退出
 */
int confirm_exit(void) {
    if (g_buffer.modified) {
        printf("\n警告: 当前文本已修改但未保存!\n");
        printf("1. 保存并退出\n");
        printf("2. 不保存退出\n");
        printf("3. 取消\n");
        
        int choice;
        if (!read_int_range("请选择: ", 1, 3, &choice)) {
            return 0;
        }
        
        switch (choice) {
            case 1:
                if (g_buffer.filename[0] != '\0') {
                    if (file_save_current(&g_buffer) != 0) {
                        printf("保存失败，已取消退出\n");
                        return 0;
                    }
                } else {
                    char filename[MAX_FILENAME];
                    if (!read_line_prompt("请输入保存的文件名: ", filename, sizeof(filename))) {
                        return 0;
                    }
                    trim_string(filename);
                    if (filename[0] != '\0') {
                        if (file_save(&g_buffer, filename) != 0) {
                            printf("保存失败，已取消退出\n");
                            return 0;
                        }
                    } else {
                        printf("文件名为空，已取消退出\n");
                        return 0;
                    }
                }
                return 1;
            case 2:
                return 1;
            case 3:
                return 0;
            default:
                return 0;
        }
    }
    return 1;
}

/*
 * 主函数
 */
int main(void) {
    int choice;
    bool running = true;

    /* 初始化缓冲区 */
    buffer_init(&g_buffer);
    /* 初始化插件管理器 */
    plugin_manager_init(&g_buffer, ui_log_func);

    printf("\n欢迎使用简易文本编辑器!\n");

    while (running) {
        display_menu();

        if (!read_int_range(NULL, 0, 10, &choice)) {
            printf("输入无效，请输入数字 0-10\n");
            continue;
        }

        switch (choice) {
            case 1:
                menu_input_text();
                break;
            case 2:
                menu_open_file();
                break;
            case 3:
                menu_save_file();
                break;
            case 4:
                menu_statistics();
                break;
            case 5:
                menu_find_substring();
                break;
            case 6:
                menu_insert_substring();
                break;
            case 7:
                menu_modify_substring();
                break;
            case 8:
                menu_delete_substring();
                break;
            case 9:
                menu_display_text();
                break;
            case 10:
                menu_plugins();
                break;
            case 0:
                if (confirm_exit()) {
                    running = false;
                    printf("\n感谢使用，再见!\n");
                }
                break;
            default:
                printf("无效选项，请输入 0-10\n");
                break;
        }
    }

    plugin_manager_cleanup();
    return 0;
}
