#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include "plugin.h"

/* 日志回调函数类型 */
typedef void (*PluginLogFunc)(const char* msg);

/* 初始化/销毁 */
void plugin_manager_init(TextBuffer* buf, PluginLogFunc log_func);
void plugin_manager_cleanup(void);

/* 加载 DLL 插件（固定 plugins 目录，扫描 *.dll） */
int load_plugins_default(void);

/* 命令相关 */
int execute_plugin_command(const char* cmd_name);
PluginCommand* get_plugin_commands(void);

#endif /* PLUGIN_MANAGER_H */
