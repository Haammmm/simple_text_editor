#ifndef PLUGIN_H
#define PLUGIN_H

#include <stddef.h>
#include "text_editor.h"

/* 插件命令类型：仅原生 DLL */
typedef enum {
    CMD_NATIVE = 0
} PluginCommandType;

/* 插件接口 */
typedef struct EditorAPI {
    int (*get_line_count)(void);
    const char* (*get_line)(int line_num);
    int (*insert_line)(int line_num, const char* text);
    int (*delete_line)(int line_num);
    int (*replace_line)(int line_num, const char* text);
    void (*print_msg)(const char* msg);
    void (*clear_screen)(void);
    void (*register_command)(const char* name, void (*func)(void), const char* desc);
    int (*read_file)(const char* path, char* out, size_t out_sz);
    int (*write_file)(const char* path, const char* data);
    int (*http_get)(const char* url, char* out, size_t out_sz);
} EditorAPI;

/* 插件初始化函数签名（DLL 出口） */
typedef int (*PluginInitFunc)(EditorAPI* api);

/* 插件命令节点 */
typedef struct PluginCommand {
    char name[32];
    void (*func)(void);
    char description[64];
    PluginCommandType type;
    int arg_int;
    char arg_text[MAX_LINE_LENGTH + 1];
    struct PluginCommand* next;
} PluginCommand;

#endif /* PLUGIN_H */
