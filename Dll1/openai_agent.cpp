#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "openai.hpp"
#include "plugin.h"


namespace {
constexpr std::string_view kBaseUrl = "https://api-inference.modelscope.cn/v1/";
constexpr std::string_view kModel = "deepseek-ai/DeepSeek-V3.2";

static EditorAPI* g_api = nullptr;

std::string get_api_key_from_env() {
    char* env = nullptr;
    size_t len = 0;
    if (_dupenv_s(&env, &len, "test_apikey") != 0 || !env) {
        return {};
    }
    std::string value(env);
    free(env);
    return value;
}

void trim_eol(std::string& s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
}

void apply_to_buffer(const std::string& text) {
    if (!g_api) return;
    for (int i = g_api->get_line_count() - 1; i >= 0; --i) {
        g_api->delete_line(i);
    }

    std::stringstream ss(text);
    std::string line;
    int current_line = 0;
    bool failed = false;
    while (std::getline(ss, line)) {
        trim_eol(line);
        if (g_api->insert_line(current_line, line.c_str()) != 0) {
            failed = true;
            g_api->print_msg("插入行失败，可能超过行数或行长度限制");
            break;
        }
        ++current_line;
    }
    if (failed) {
        g_api->print_msg("缓冲区已被部分更新，请检查内容");
    }
}

std::string get_buffer_content() {
    if (!g_api) return {};
    std::string buffer_text;
    const int count = g_api->get_line_count();
    for (int i = 0; i < count; ++i) {
        if (const char* line = g_api->get_line(i)) {
            buffer_text.append(line).push_back('\n');
        }
    }
    return buffer_text;
}

void cmd_llm_dialog() {
    if (!g_api) return;
    
    std::cout << "\n=== LLM 对话（C++ Agent）===\n";
    std::cout << "请输入你的指令（单行，回车结束）：\n> " << std::flush;
    std::string user_msg;
    if (!std::getline(std::cin, user_msg)) return;
    trim_eol(user_msg);

    const std::string buffer_text = get_buffer_content();
    const std::string api_key = get_api_key_from_env();
    if (api_key.empty()) {
        g_api->print_msg("缺少环境变量 test_apikey，无法调用 LLM 接口");
        return;
    }

    openai::start(api_key, "", true, std::string{kBaseUrl});

    openai::Json tools = openai::Json::array({
        {
            {"type", "function"},
            {"function", {
                {"name", "apply_editor"},
                {"description", "Overwrite editor buffer with provided text."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"content", {{"type", "string"}}}
                    }},
                    {"required", openai::Json::array({"content"})}
                }}
            }}
        },
        {
            {"type", "function"},
            {"function", {
                {"name", "get_buffer"},
                {"description", "Return the current editor buffer content as text."},
                {"parameters", {
                    {"type", "object"},
                    {"properties", openai::Json::object()},
                    {"required", openai::Json::array()}
                }}
            }}
        }
    });

    std::vector<openai::Json> history;
    history.reserve(3);
    history.emplace_back(openai::Json{{"role", "system"}, {"content", "You are an editor agent. If the user asks to change/overwrite the buffer, you must use the tool 'apply_editor' to return the final buffer content. If no buffer change is needed, you may reply directly."}});
    history.emplace_back(openai::Json{{"role", "user"}, {"content", user_msg}});
    history.emplace_back(openai::Json{{"role", "user"}, {"content", "Current buffer:\n" + buffer_text}});

    g_api->print_msg("正在请求 LLM (C++)...");

    try {
        bool done = false;
        for (int safety = 4; !done && safety-- > 0;) {
            openai::Json payload = {
                {"model", std::string{kModel}},
                {"messages", history},
                {"tools", tools},
                {"tool_choice", "auto"},
                {"temperature", 0}
            };

            auto chat = openai::chat().create(payload);
            if (!(chat.contains("choices") && !chat["choices"].empty())) {
                g_api->print_msg("未收到有效回复");
                return;
            }

            auto& message = chat["choices"][0]["message"];

            if (message.contains("content") && !message["content"].is_null()) {
                std::string content = message["content"];
                std::cout << "\n[LLM 回复]:\n" << content << "\n";
            }

            if (message.contains("tool_calls") && !message["tool_calls"].empty()) {
                history.push_back(message);

                for (auto& tool_call : message["tool_calls"]) {
                    std::string name = tool_call["function"]["name"];
                    std::string result;
                    if (name == "apply_editor") {
                        std::string args_str = tool_call["function"]["arguments"];
                        auto args_json = openai::Json::parse(args_str);
                        if (args_json.contains("content")) {
                            std::string content = args_json["content"];
                            apply_to_buffer(content);
                            g_api->print_msg("LLM 已通过工具更新缓冲区");
                            result = "applied";
                        } else {
                            result = "no content provided";
                        }
                    } else if (name == "get_buffer") {
                        result = get_buffer_content();
                    }

                    history.push_back(openai::Json{
                        {"role", "tool"},
                        {"tool_call_id", tool_call["id"]},
                        {"content", result}
                    });
                }
            } else {
                done = true;
            }
        }
    } catch (const std::exception& e) {
        std::string err = "OpenAI Error: ";
        err += e.what();
        g_api->print_msg(err.c_str());
    }
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    g_api->register_command("LLMDialog", cmd_llm_dialog, "Chat with LLM (C++ Modern); tool updates buffer");
    return 0;
}

#ifdef __cplusplus
}
#endif
