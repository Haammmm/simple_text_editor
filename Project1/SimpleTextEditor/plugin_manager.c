#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winhttp.h>
#include <stdarg.h>
#include <stdbool.h>
#include "plugin_manager.h"
#include "text_editor.h"


static TextBuffer* g_buf = NULL;
static PluginCommand* g_commands = NULL;
static HMODULE g_modules[32];
static int g_module_count = 0;
static char g_plugin_dir[MAX_PATH] = {0};
static const char* PLUGIN_DIR = ".\\plugins";
static PluginLogFunc g_log_func = NULL;

/* ================= 工具函数 ================= */
static void log_message(const char* fmt, ...) {
    if (!g_log_func) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    g_log_func(buffer);
}

static void trim(char* s) {
    if (!s) return;
    char* p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

static bool command_exists(const char* name) {
    for (PluginCommand* p = g_commands; p; p = p->next) {
        if (strcmp(p->name, name) == 0) return true;
    }
    return false;
}

/* 简易 URL 解析（仅 http/https） */
static bool parse_url(const char* url, wchar_t* host, size_t host_len, wchar_t* path, size_t path_len, INTERNET_PORT* port, BOOL* https) {
    if (!url || !host || !path || !port || !https) return false;
    const char* p = NULL;
    *https = FALSE;
    *port = 80;
    if (strncmp(url, "https://", 8) == 0) { *https = TRUE; *port = 443; p = url + 8; }
    else if (strncmp(url, "http://", 7) == 0) { p = url + 7; }
    else return false;
    const char* slash = strchr(p, '/');
    size_t hostlen = slash ? (size_t)(slash - p) : strlen(p);
    if (hostlen == 0 || hostlen + 1 > host_len) return false;
    size_t converted = 0;
    if (mbstowcs_s(&converted, host, host_len, p, hostlen) != 0) return false;
    if (slash) {
        if (mbstowcs_s(&converted, path, path_len, slash, _TRUNCATE) != 0) return false;
    } else {
        if (wcscpy_s(path, path_len, L"/") != 0) return false;
    }
    return true;
}

/* 简易 http_get（同步 GET），返回写入字节数或 -1 */
static int api_http_get(const char* url, char* out, size_t out_sz) {
    if (!url || !out || out_sz == 0) return -1;
    wchar_t host[256], path[512];
    INTERNET_PORT port; BOOL https;
    if (!parse_url(url, host, _countof(host), path, _countof(path), &port, &https)) {
        snprintf(out, out_sz, "[http_get] bad url: %s", url);
        return -1;
    }

    HINTERNET hSession = WinHttpOpen(L"SimpleTextEditor/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { snprintf(out, out_sz, "[http_get] open fail"); return -1; }

    HINTERNET hConnect = WinHttpConnect(hSession, host, port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); snprintf(out, out_sz, "[http_get] connect fail"); return -1; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, https ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); snprintf(out, out_sz, "[http_get] openreq fail"); return -1; }

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (ok) ok = WinHttpReceiveResponse(hRequest, NULL);
    if (!ok) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        snprintf(out, out_sz, "[http_get] send/recv fail");
        return -1;
    }

    DWORD dwSize = 0, dwDownloaded = 0; size_t written = 0;
    do {
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;
        char buffer[512]; DWORD toRead = dwSize < sizeof(buffer) ? dwSize : (DWORD)sizeof(buffer);
        if (!WinHttpReadData(hRequest, buffer, toRead, &dwDownloaded)) break;
        if (dwDownloaded == 0) break;
        size_t copy = (written + dwDownloaded >= out_sz) ? (out_sz - written - 1) : dwDownloaded;
        if (copy > 0) {
            memcpy(out + written, buffer, copy);
            written += copy;
        }
    } while (dwSize > 0);
    out[written] = '\0';

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return (int)written;
}

/* 简易本地文件读写 */
static int api_read_file(const char* path, char* out, size_t out_sz) {
    if (!path || !out || out_sz == 0) return -1;
    FILE* fp = NULL;
    if (fopen_s(&fp, path, "rb") != 0 || !fp) return -1;
    size_t n = fread(out, 1, out_sz - 1, fp);
    out[n] = '\0';
    fclose(fp);
    return (int)n;
}

static int api_write_file(const char* path, const char* data) {
    if (!path || !data) return -1;
    FILE* fp = NULL;
    if (fopen_s(&fp, path, "wb") != 0 || !fp) return -1;
    size_t len = strlen(data);
    size_t n = fwrite(data, 1, len, fp);
    fclose(fp);
    return (n == len) ? 0 : -1;
}

/* ================= API 实现 ================= */
static int api_get_line_count(void) {
    return g_buf ? g_buf->line_count : 0;
}

static const char* api_get_line(int line_num) {
    if (!g_buf) return NULL;
    if (line_num < 0 || line_num >= g_buf->line_count) return NULL;
    return g_buf->lines[line_num];
}

static int api_insert_line(int line_num, const char* text) {
    if (!g_buf) return -1;
    return insert_line(g_buf, line_num, text);
}

static int api_delete_line(int line_num) {
    if (!g_buf) return -1;
    return delete_line(g_buf, line_num);
}

static int api_replace_line(int line_num, const char* text) {
    if (!g_buf) return -1;
    if (line_num < 0 || line_num >= g_buf->line_count) return -1;
    if ((int)strlen(text) > MAX_LINE_LENGTH) return -1;
    strncpy_s(g_buf->lines[line_num], sizeof(g_buf->lines[line_num]), text, _TRUNCATE);
    g_buf->modified = 1;
    return 0;
}

static void api_print_msg(const char* msg) {
    if (g_log_func) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "[插件] %s\n", msg ? msg : "");
        g_log_func(buffer);
    }
}

static void api_clear_screen(void) {
    clear_screen();
}

static void api_register_command(const char* name, void (*func)(void), const char* desc) {
    if (!name || !func) return;
    if (command_exists(name)) return; /* 避免重名 */
    PluginCommand* cmd = (PluginCommand*)malloc(sizeof(PluginCommand));
    if (!cmd) return;
    strncpy_s(cmd->name, sizeof(cmd->name), name, _TRUNCATE);
    cmd->func = func;
    strncpy_s(cmd->description, sizeof(cmd->description), desc ? desc : "", _TRUNCATE);
    cmd->type = CMD_NATIVE;
    cmd->arg_int = 0;
    cmd->arg_text[0] = '\0';
    cmd->next = g_commands;
    g_commands = cmd;
}

static EditorAPI g_api = {
    api_get_line_count,
    api_get_line,
    api_insert_line,
    api_delete_line,
    api_replace_line,
    api_print_msg,
    api_clear_screen,
    api_register_command,
    api_read_file,
    api_write_file,
    api_http_get
};

/* ================= 管理器 ================= */
static void set_plugin_dir_from_exe(void) {
    DWORD len = GetModuleFileNameA(NULL, g_plugin_dir, (DWORD)sizeof(g_plugin_dir));
    if (len == 0 || len >= sizeof(g_plugin_dir)) {
        strncpy_s(g_plugin_dir, sizeof(g_plugin_dir), PLUGIN_DIR, _TRUNCATE);
        return;
    }
    /* 去掉文件名 */
    for (int i = (int)strlen(g_plugin_dir) - 1; i >= 0; --i) {
        if (g_plugin_dir[i] == '\\' || g_plugin_dir[i] == '/') {
            g_plugin_dir[i] = '\0';
            break;
        }
    }
    /* 追加 plugins 子目录 */
    size_t base_len = strlen(g_plugin_dir);
    if (base_len + 9 < sizeof(g_plugin_dir)) {
        strcat_s(g_plugin_dir, sizeof(g_plugin_dir), "\\plugins");
    }
}

void plugin_manager_init(TextBuffer* buf, PluginLogFunc log_func) {
    g_buf = buf;
    g_log_func = log_func;
    g_commands = NULL;
    g_module_count = 0;
    memset(g_modules, 0, sizeof(g_modules));
    set_plugin_dir_from_exe();
}

/* 内部加载单个 DLL */
static int load_plugin_internal(const char* dll_path) {
    if (!dll_path || dll_path[0] == '\0') return -1;

    /* 使用 LoadLibraryExA 并指定 LOAD_WITH_ALTERED_SEARCH_PATH，
       以便插件可以加载同目录下的依赖库（如 libcurl.dll） */
    HMODULE h = LoadLibraryExA(dll_path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!h) {
        log_message("错误: 无法加载插件 '%s' (错误码 %lu)\n", dll_path, GetLastError());
        return -1;
    }

    PluginInitFunc init_func = (PluginInitFunc)GetProcAddress(h, "PluginInit");
    if (!init_func) {
        log_message("错误: 插件 '%s' 缺少 PluginInit\n", dll_path);
        FreeLibrary(h);
        return -1;
    }

    if (init_func(&g_api) != 0) {
        log_message("错误: 插件 '%s' 初始化失败\n", dll_path);
        FreeLibrary(h);
        return -1;
    }

    if (g_module_count < (int)_countof(g_modules)) {
        g_modules[g_module_count++] = h;
    }

    log_message("成功加载插件: %s\n", dll_path);
    return 0;
}

int load_plugins_default(void) {
    char pattern[MAX_PATH];
    const char* dir = (g_plugin_dir[0] != '\0') ? g_plugin_dir : PLUGIN_DIR;
    snprintf(pattern, sizeof(pattern), "%s\\*.dll", dir);

    WIN32_FIND_DATAA fdata;
    HANDLE hFind = FindFirstFileA(pattern, &fdata);
    if (hFind == INVALID_HANDLE_VALUE) {
        log_message("提示: 目录 '%s' 下未找到 DLL 插件\n", dir);
        return -1;
    }

    int count = 0;
    do {
        if (!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            /* 过滤掉非 plugin_ 开头的 DLL */
            if (strncmp(fdata.cFileName, "plugin_", 7) != 0) {
                continue;
            }

            char path[MAX_PATH];
            snprintf(path, sizeof(path), "%s\\%s", dir, fdata.cFileName);
            if (load_plugin_internal(path) == 0) {
                count++;
            }
        }
    } while (FindNextFileA(hFind, &fdata));
    FindClose(hFind);

    log_message("已加载 DLL 插件 %d 个 (来自 %s)\n", count, dir);
    return count > 0 ? count : -1;
}

int execute_plugin_command(const char* cmd_name) {
    PluginCommand* p = g_commands;
    while (p) {
        if (strcmp(p->name, cmd_name) == 0) {
            if (p->func) {
                p->func();
                return 0;
            }
            return -1;
        }
        p = p->next;
    }
    return -1;
}

PluginCommand* get_plugin_commands(void) {
    return g_commands;
}

void plugin_manager_cleanup(void) {
    PluginCommand* p = g_commands;
    while (p) {
        PluginCommand* next = p->next;
        free(p);
        p = next;
    }
    g_commands = NULL;
    for (int i = 0; i < g_module_count; ++i) {
        if (g_modules[i]) FreeLibrary(g_modules[i]);
    }
    g_module_count = 0;
    g_buf = NULL;
}
